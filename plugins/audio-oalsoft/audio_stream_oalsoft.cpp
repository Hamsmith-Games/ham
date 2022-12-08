#include "audio-oalsoft.h"

#include "alext.h"

HAM_C_API_BEGIN

static inline ALenum ham_audio_data_al_format(const ham_audio_data *data){
	if(!data){
		return AL_FORMAT_MONO_FLOAT32;
	}

	const auto num_channels = ham_audio_data_num_channels(data);

	switch(ham_audio_data_format(data)){
		case HAM_AUDIO_SAMPLE_U8:{
			switch(num_channels){
				case 2: return AL_FORMAT_STEREO8;
				case 4: return AL_FORMAT_QUAD8;
				case 6: return AL_FORMAT_51CHN8;
				case 8: return AL_FORMAT_71CHN8;
				case 1:
				default: return AL_FORMAT_MONO8;
			}
		}

		case HAM_AUDIO_SAMPLE_U16:{
			switch(num_channels){
				case 2: return AL_FORMAT_STEREO16;
				case 4: return AL_FORMAT_QUAD16;
				case 6: return AL_FORMAT_51CHN16;
				case 8: return AL_FORMAT_71CHN16;
				case 1:
				default: return AL_FORMAT_MONO8;
			}
		}

		case HAM_AUDIO_SAMPLE_U32:{
			switch(num_channels){
				case 2: return AL_FORMAT_STEREO_FLOAT32;
				case 4: return AL_FORMAT_QUAD32;
				case 6: return AL_FORMAT_51CHN32;
				case 8: return AL_FORMAT_71CHN32;
				case 1:
				default: return AL_FORMAT_MONO8;
			}
		}

		case HAM_AUDIO_SAMPLE_F32:{
			switch(num_channels){
				case 2: return AL_FORMAT_STEREO_FLOAT32;
				case 4: return AL_FORMAT_QUAD32;
				case 6: return AL_FORMAT_51CHN32;
				case 8: return AL_FORMAT_71CHN32;
				case 1:
				default: return AL_FORMAT_MONO_FLOAT32;
			}
		}

		default:{
			return AL_FORMAT_MONO_FLOAT32;
		}
	}
}

static inline ham_audio_stream_oalsoft *ham_audio_stream_oalsoft_ctor(ham_audio_stream_oalsoft *ptr, ham_u32 nargs, va_list va){
	// TODO: check args

	const auto ret = new(ptr) ham_audio_stream_oalsoft;

	alGenSources(1, &ret->al_source);
	alGenBuffers(HAM_AUDIO_NUM_BUFFERS, ret->al_bufs);

	for(int i = 0; i < HAM_AUDIO_NUM_BUFFERS; i++){
		alBufferData(ret->al_bufs[i], AL_FORMAT_MONO_FLOAT32, nullptr, HAM_AUDIO_BUFFER_SAMPLES * sizeof(ham_f32), 48000);
	}

	// TODO: fill in stream info

	ret->data_off = 0;
	ret->play_off = 0;

	ret->pos = ham_vec3{ .data = { 0, 0, 0 } };
	ret->pyr = ham_vec3{ .data = { 0, 0, 0 } };
	ret->vel = ham_vec3{ .data = { 0, 0, 0 } };

	return ret;
}

static inline void ham_audio_stream_oalsoft_dtor(ham_audio_stream_oalsoft *ptr){
	alDeleteSources(1, &ptr->al_source);
	alDeleteBuffers(HAM_AUDIO_NUM_BUFFERS, ptr->al_bufs);
	std::destroy_at(ptr);
}

static inline bool ham_audio_stream_oalsoft_set_data(ham_audio_stream_oalsoft *self, const ham_audio_data *data){
	self->play_off = 0;
	self->data_off = 0;

	int nb = 0;

	if(!data){
		for(int i = 0; i < HAM_AUDIO_NUM_BUFFERS; i++){
			alBufferData(self->al_bufs[i], AL_FORMAT_MONO_FLOAT32, nullptr, HAM_AUDIO_BUFFER_SAMPLES * sizeof(ham_f32), 48000);
		}

		nb = HAM_AUDIO_NUM_BUFFERS;
	}
	else{
		const auto data_ptr    = ham_audio_data_ptr(data);
		const auto data_size   = ham_audio_data_size(data);
		const auto data_format = ham_audio_data_al_format(data);
		const auto data_freq   = ham_audio_data_frequency(data);

		while(self->data_off < data_size && nb < HAM_AUDIO_NUM_BUFFERS){
			const auto to_write = ham_min(data_size - self->data_off, HAM_AUDIO_BUFFER_SAMPLES);

			alBufferData(self->al_bufs[nb], data_format, (const char*)data_ptr + self->data_off, to_write, data_freq);
			++nb;

			self->data_off += to_write;
		}
	}

	alSourceQueueBuffers(self->al_source, nb, self->al_bufs);
	return true;
}

static inline ham_vec3 ham_audio_stream_oalsoft_position(const ham_audio_stream_oalsoft *self){
	return self->pos;
}

static inline ham_vec3 ham_audio_stream_oalsoft_rotation(const ham_audio_stream_oalsoft *self){
	return self->pyr;
}

static inline ham_vec3 ham_audio_stream_oalsoft_velocity(const ham_audio_stream_oalsoft *self){
	return self->vel;
}

static inline void ham_audio_stream_oalsoft_set_position(ham_audio_stream_oalsoft *self, ham_vec3 pos){
	alSourcefv(self->al_source, AL_POSITION, pos.data);
	self->pos = pos;
}

static inline void ham_audio_stream_oalsoft_set_rotation(ham_audio_stream_oalsoft *self, ham_vec3 pyr){
	const auto orientation = ham_audio_orientation_from_pyr(pyr);

	const ham::f32 data[] = {
		orientation.forward.x, orientation.forward.y, orientation.forward.z,
		orientation.up.x, orientation.up.y, orientation.up.z
	};

	alSourcefv(self->al_source, AL_ORIENTATION, data);
	self->pyr = pyr;
}

static inline void ham_audio_stream_oalsoft_set_velocity(ham_audio_stream_oalsoft *self, ham_vec3 vel){
	alSourcefv(self->al_source, AL_VELOCITY, vel.data);
	self->vel = vel;
}

static inline bool ham_audio_stream_oalsoft_play(ham_audio_stream_oalsoft *self){
	alSourcePlay(self->al_source);
	return true;
}

static inline bool ham_audio_stream_oalsoft_pause(ham_audio_stream_oalsoft *self){
	alSourcePause(self->al_source);
	return true;
}

static inline bool ham_audio_stream_oalsoft_stop(ham_audio_stream_oalsoft *self){
	alSourceStop(self->al_source);
	return true;
}

ham_define_audio_stream_object(ham_audio_stream_oalsoft)

HAM_C_API_END
