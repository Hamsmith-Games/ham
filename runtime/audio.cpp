/*
 * Ham Runtime
 * Copyright (C) 2022 Keith Hammond
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "ham/memory.h"
#include "ham/check.h"
#include "ham/audio-object.h"

#include "sndfile.h"

HAM_C_API_BEGIN

//
// Audio data
//

struct ham_audio_data{
	const ham_allocator *allocator;
	ham_audio_sample_format format;
	ham_u32 num_channels, num_frames, freq;
	void *data;
};

ham_audio_data *ham_audio_data_load(ham_usize len, const void *data){
	if(!ham_check(len && data)){
		return nullptr;
	}

	struct sf_user_data{
		ham_usize len;
		const unsigned char *beg, *end;
		const unsigned char *ptr;
	} user_data{len, (const unsigned char*)data, (const unsigned char*)data + len, (const unsigned char*)data};

	SF_VIRTUAL_IO io;

	io.get_filelen = [](void *user_data) -> sf_count_t{
		const auto data = (sf_user_data*)user_data;
		return (sf_count_t)data->len;
	};

	io.seek = [](sf_count_t offset, int whence, void *user_data) -> sf_count_t{
		const auto data = (sf_user_data*)user_data;

		switch(whence){
			case SEEK_CUR:{
				data->ptr += offset;
				return (ham_iptr)data->end - (ham_iptr)data->ptr;
			}

			case SEEK_SET:{
				data->ptr = data->beg + offset;
				return offset;
			}

			case SEEK_END:{
				data->ptr = data->end - offset;
				return (ham_iptr)data->end - (ham_iptr)data->ptr;
			}

			default:{
				return (ham_iptr)data->end - (ham_iptr)data->ptr;
			}
		}
	};

	io.read = [](void *ptr, sf_count_t count, void *user_data) -> sf_count_t{
		const auto data = (sf_user_data*)user_data;
		const ham_usize rem = (ham_uptr)data->end - (ham_uptr)data->ptr;
		const ham_usize len = ham_min((sf_count_t)rem, count);

		memcpy(ptr, data->ptr, count);
		data->ptr += count;

		return (sf_count_t)len;
	};

	io.write = [](const void *ptr, sf_count_t count, void *user_data) -> sf_count_t{
		(void)ptr; (void)count; (void)user_data;
		return 0;
	};

	io.tell = [](void *user_data) -> sf_count_t{
		const auto data = (sf_user_data*)user_data;
		return (ham_iptr)data->end - (ham_iptr)data->ptr;
	};

	SF_INFO sf_info;
	const auto sf = sf_open_virtual(&io, SFM_READ, &sf_info, &user_data);
	if(!sf){
		const auto sf_res = sf_error(sf);
		ham_logapierrorf("Error in sf_open_virtual: %s", sf_error_number(sf_res));
		return nullptr;
	}

	const auto buf_size = sf_info.channels * sf_info.frames * sizeof(ham_f32);

	const auto allocator = ham_current_allocator();

	const auto buf = ham_allocator_alloc(allocator, alignof(ham_f32), buf_size);
	if(!buf){
		ham_logapierrorf("Error allocating audio buffer");

		const int sf_res = sf_close(sf);
		if(sf_res != 0){
			ham_logapiwarnf("Error in sf_close: %s", sf_error_number(sf_res));
		}

		return nullptr;
	}

	const auto to_read = sf_info.channels * sf_info.frames;
	if(sf_read_float(sf, (ham_f32*)buf, sf_info.channels * sf_info.frames) != to_read){
		const auto sf_res = sf_error(sf);
		ham_logapierrorf("Error in sf_read_float: %s", sf_error_number(sf_res));
		ham_allocator_free(allocator, buf);
		return nullptr;
	}

	const int sf_res = sf_close(sf);
	if(sf_res != 0){
		ham_logapiwarnf("Error in sf_close: %s", sf_error_number(sf_res));
	}

	const auto ptr = ham_allocator_new(allocator, ham_audio_data);
	if(!ptr){
		ham_logapierrorf("Error allocating ham_audio_data");
		ham_allocator_free(allocator, buf);
		return nullptr;
	}

	ptr->allocator    = allocator;
	ptr->format       = HAM_AUDIO_SAMPLE_F32;
	ptr->num_channels = (ham_u32)sf_info.channels;
	ptr->num_frames   = (ham_u32)sf_info.frames;
	ptr->freq         = (ham_u32)sf_info.samplerate;
	ptr->data         = buf;

	return ptr;
}

void ham_audio_data_destroy(ham_audio_data *data){
	if(!data) return;

	const auto allocator = data->allocator;

	ham_allocator_free(allocator, data->data);
	ham_allocator_delete(allocator, data);
}

ham_usize ham_audio_data_size(const ham_audio_data *data){
	if(!data) return 0;
	return data->num_channels * data->num_frames * sizeof(ham_f32);
}

const void *ham_audio_data_ptr(const ham_audio_data *data){
	if(!data) return nullptr;
	return data->data;
}

ham_u32 ham_audio_data_num_channels(const ham_audio_data *data){
	if(!data) return (ham_u32)-1;
	return data->num_channels;
}

ham_u32 ham_audio_data_num_samples(const ham_audio_data *data){
	if(!data) return (ham_u32)-1;
	return data->num_frames;
}

ham_u32 ham_audio_data_frequency(const ham_audio_data *data){
	if(!data) return (ham_u32)-1;
	return data->freq;
}

ham_audio_sample_format ham_audio_data_format(const ham_audio_data *data){
	if(!data) return HAM_AUDIO_SAMPLE_FORMAT_COUNT;
	return data->format;
}

//
// Audio context object
//

ham_audio *ham_audio_create(const ham_audio_vtable *vptr){
	const auto allocator = ham_current_allocator();

	const auto info = ham_super(vptr)->info;

	const auto obj = (ham_object*)ham_allocator_alloc(allocator, info->alignment, info->size);
	if(!obj) return nullptr;

	const auto audio = (ham_audio*)obj;

	obj->vptr = ham_super(vptr);
	audio->allocator = allocator;
	audio->pos = ham_make_vec3_scalar(0.f);
	audio->vel = ham_make_vec3_scalar(0.f);

	if(!ham_super(vptr)->ctor(obj, 0, nullptr)){
		ham_allocator_free(allocator, obj);
		return nullptr;
	}

	return audio;
}

ham_nothrow void ham_audio_destroy(ham_audio *audio){
	if(!audio) return;

	const auto allocator = audio->allocator;
	const auto vptr = ham_super(audio)->vptr;

	vptr->dtor(ham_super(audio));
	ham_allocator_free(allocator, audio);
}

void ham_audio_update(ham_audio *ctx, ham_f64 dt){
	if(!ham_check(ctx != NULL)) return;
	const auto vptr = (const ham_audio_vtable*)ham_super(ctx)->vptr;
	vptr->update(ctx, dt);
}

ham_vec3 ham_audio_position(const ham_audio *ctx){
	if(!ham_check(ctx != NULL)) return ham_make_vec3_scalar(0.f);
	const auto vptr = (const ham_audio_vtable*)ham_super(ctx)->vptr;
	return vptr->position(ctx);
}

ham_vec3 ham_audio_rotation(const ham_audio *ctx){
	if(!ham_check(ctx != NULL)) return ham_make_vec3_scalar(0.f);
	const auto vptr = (const ham_audio_vtable*)ham_super(ctx)->vptr;
	return vptr->rotation(ctx);
}

ham_vec3 ham_audio_velocity(const ham_audio *ctx){
	if(!ham_check(ctx != NULL)) return ham_make_vec3_scalar(0.f);
	const auto vptr = (const ham_audio_vtable*)ham_super(ctx)->vptr;
	return vptr->velocity(ctx);
}

void ham_audio_set_position(ham_audio *ctx, ham_vec3 pos){
	if(!ham_check(ctx != NULL)) return;
	const auto vptr = (const ham_audio_vtable*)ham_super(ctx)->vptr;
	return vptr->set_position(ctx, pos);
}

void ham_audio_set_rotation(ham_audio *ctx, ham_vec3 pyr){
	if(!ham_check(ctx != NULL)) return;
	const auto vptr = (const ham_audio_vtable*)ham_super(ctx)->vptr;
	return vptr->set_rotation(ctx, pyr);
}

void ham_audio_set_velocity(ham_audio *ctx, ham_vec3 vel){
	if(!ham_check(ctx != NULL)) return;
	const auto vptr = (const ham_audio_vtable*)ham_super(ctx)->vptr;
	return vptr->set_velocity(ctx, vel);
}

//
// Audio streams
//

ham_audio_stream *ham_audio_stream_create(ham_audio *ctx, const ham_audio_data *data){
	if(!ham_check(ctx != NULL)) return nullptr;

	const auto allocator = ctx->allocator;

	const auto vptr = ((const ham_audio_vtable*)ham_super(ctx)->vptr)->stream_vptr();
	const auto obj_vptr = ham_super(vptr);
	const auto obj_info = obj_vptr->info;

	const auto stream = (ham_audio_stream*)ham_allocator_alloc(allocator, obj_info->alignment, obj_info->size);
	const auto obj = ham_super(stream);

	obj->vptr = obj_vptr;
	stream->ctx  = ctx;
	stream->data = data;

	if(!obj_vptr->ctor(obj, 0, nullptr)){
		ham_logapierrorf("Error constructing object of type '%s'", obj_info->type_id);
		ham_allocator_free(allocator, stream);
		return nullptr;
	}

	if(!vptr->set_data(stream, data)){
		ham_logapiwarnf("Error in %s::set_data", obj_info->type_id);
	}

	return stream;
}

void ham_audio_stream_destroy(ham_audio_stream *stream){
	if(!stream) return;

	const auto allocator = stream->ctx->allocator;
	const auto vptr = ham_super(stream)->vptr;

	vptr->dtor(ham_super(stream));

	ham_allocator_free(allocator, stream);
}

bool ham_audio_stream_set_data(ham_audio_stream *stream, const ham_audio_data *data){
	if(!ham_check(stream != NULL)) return false;
	stream->data = data;
	const auto vptr = (const ham_audio_stream_vtable*)ham_super(stream)->vptr;
	return vptr->set_data(stream, data);
}

ham_vec3 ham_audio_stream_position(const ham_audio_stream *stream){
	if(!ham_check(stream != NULL)) return ham_make_vec3_scalar(0.f);
	const auto vptr = (const ham_audio_stream_vtable*)ham_super(stream)->vptr;
	return vptr->position(stream);
}

ham_vec3 ham_audio_stream_rotation(const ham_audio_stream *stream){
	if(!ham_check(stream != NULL)) return ham_make_vec3_scalar(0.f);
	const auto vptr = (const ham_audio_stream_vtable*)ham_super(stream)->vptr;
	return vptr->rotation(stream);
}

ham_vec3 ham_audio_stream_velocity(const ham_audio_stream *stream){
	if(!ham_check(stream != NULL)) return ham_make_vec3_scalar(0.f);
	const auto vptr = (const ham_audio_stream_vtable*)ham_super(stream)->vptr;
	return vptr->velocity(stream);
}

void ham_audio_stream_set_position(ham_audio_stream *stream, ham_vec3 pos){
	if(!ham_check(stream != NULL)) return;
	const auto vptr = (const ham_audio_stream_vtable*)ham_super(stream)->vptr;
	return vptr->set_position(stream, pos);
}

void ham_audio_stream_set_rotation(ham_audio_stream *stream, ham_vec3 pyr){
	if(!ham_check(stream != NULL)) return;
	const auto vptr = (const ham_audio_stream_vtable*)ham_super(stream)->vptr;
	return vptr->set_rotation(stream, pyr);
}

void ham_audio_stream_set_velocity(ham_audio_stream *stream, ham_vec3 vel){
	if(!ham_check(stream != NULL)) return;
	const auto vptr = (const ham_audio_stream_vtable*)ham_super(stream)->vptr;
	return vptr->set_velocity(stream, vel);
}

bool ham_audio_stream_play(ham_audio_stream *stream){
	if(!ham_check(stream != NULL)) return false;
	const auto vptr = (const ham_audio_stream_vtable*)ham_super(stream)->vptr;
	return vptr->play(stream);
}

bool ham_audio_stream_pause(ham_audio_stream *stream){
	if(!ham_check(stream != NULL)) return false;
	const auto vptr = (const ham_audio_stream_vtable*)ham_super(stream)->vptr;
	return vptr->pause(stream);
}

bool ham_audio_stream_stop(ham_audio_stream *stream){
	if(!ham_check(stream != NULL)) return false;
	const auto vptr = (const ham_audio_stream_vtable*)ham_super(stream)->vptr;
	return vptr->stop(stream);
}

HAM_C_API_END
