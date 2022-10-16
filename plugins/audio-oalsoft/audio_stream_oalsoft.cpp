#include "audio-oalsoft.h"

HAM_C_API_BEGIN

static inline ham_audio_stream_oalsoft *ham_audio_stream_oalsoft_ctor(ham_audio_stream_oalsoft *ptr, ham_u32 nargs, va_list va){
	// TODO: check args

	const auto ret = new(ptr) ham_audio_stream_oalsoft;

	// TODO: fill in stream info

	ret->pos = ham_vec3{ .data = { 0, 0, 0 } };
	ret->pyr = ham_vec3{ .data = { 0, 0, 0 } };
	ret->vel = ham_vec3{ .data = { 0, 0, 0 } };

	return ret;
}

static inline void ham_audio_stream_oalsoft_dtor(ham_audio_stream_oalsoft *ptr){
	std::destroy_at(ptr);
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
	self->pos = pos;
}

static inline void ham_audio_stream_oalsoft_set_rotation(ham_audio_stream_oalsoft *self, ham_vec3 pyr){
	self->pyr = pyr;
}

static inline void ham_audio_stream_oalsoft_set_velocity(ham_audio_stream_oalsoft *self, ham_vec3 vel){
	self->vel = vel;
}

static inline bool ham_audio_stream_oalsoft_play(ham_audio_stream_oalsoft *self){ return false; }
static inline bool ham_audio_stream_oalsoft_pause(ham_audio_stream_oalsoft *self){ return false; }
static inline bool ham_audio_stream_oalsoft_stop(ham_audio_stream_oalsoft *self){ return true; }

ham_define_audio_stream_object(ham_audio_stream_oalsoft)

HAM_C_API_END
