#include "audio-oalsoft.h"

#include "ham/log.h"

#include "AL/alc.h"
#include "AL/alext.h"

HAM_C_API_BEGIN

static inline ham_audio_oalsoft *ham_audio_oalsoft_ctor(ham_audio_oalsoft *ptr, ham_u32 nargs, va_list va){
	// Open the default device
	const auto dev = alcOpenDevice(nullptr);
	if(!dev){
		ham::logapierror("Error in alcOpenDevice");
		return nullptr;
	}

#define ALC_CHECK_EXT_SUPPORTED(name) \
	if(!alcIsExtensionPresent(dev, #name)){ \
		ham::logapierror("ALC extension not supported: " #name); \
		return nullptr; \
	}

#define AL_CHECK_EXT_SUPPORTED(name) \
	if(!alIsExtensionPresent(#name)){ \
		ham::logapierror("AL extension not supported: " #name); \
		return nullptr; \
	}

	ALC_CHECK_EXT_SUPPORTED(ALC_SOFT_reopen_device)
	ALC_CHECK_EXT_SUPPORTED(ALC_SOFT_pause_device)
	ALC_CHECK_EXT_SUPPORTED(ALC_SOFT_HRTF)
	ALC_CHECK_EXT_SUPPORTED(ALC_SOFT_output_mode)
	ALC_CHECK_EXT_SUPPORTED(ALC_SOFT_output_limiter)
	ALC_CHECK_EXT_SUPPORTED(ALC_EXT_EFX)

	AL_CHECK_EXT_SUPPORTED(AL_SOFT_buffer_samples)
	AL_CHECK_EXT_SUPPORTED(AL_SOFT_source_resampler)
	AL_CHECK_EXT_SUPPORTED(AL_SOFT_source_spatialize)
	AL_CHECK_EXT_SUPPORTED(AL_SOFT_source_latency)
	AL_CHECK_EXT_SUPPORTED(AL_SOFT_effect_target)
	AL_CHECK_EXT_SUPPORTED(AL_EXT_float32)


#undef ALC_CHECK_EXT_SUPPORTED

	const ALCint attrs[] = {
		ALC_HRTF_SOFT, ALC_TRUE,
		ALC_OUTPUT_MODE_SOFT, ALC_STEREO_HRTF_SOFT,
		ALC_OUTPUT_LIMITER_SOFT, ALC_TRUE,
		0, 0,
	};

	const auto ctx = alcCreateContext(dev, attrs);
	if(!ctx){
		ham::logapierror("Error in alcCreateContext");
		alcCloseDevice(dev);
	}

	const auto ret = new(ptr) ham_audio_oalsoft;

	ret->alc_dev = dev;
	ret->alc_ctx = ctx;

	return ret;
}

static inline void ham_audio_oalsoft_dtor(ham_audio_oalsoft *ctx){
	alcDestroyContext(ctx->alc_ctx);
	alcCloseDevice(ctx->alc_dev);
	std::destroy_at(ctx);
}

static inline ham_vec3 ham_audio_oalsoft_position(const ham_audio_oalsoft *self){
	ham_vec3 ret;
	alGetListenerfv(AL_POSITION, ret.data);
	return ret;
}

static inline ham_vec3 ham_audio_oalsoft_rotation(const ham_audio_oalsoft *self){
//	float orientation[6];
//	alGetListenerfv(AL_ORIENTATION, orientation);

//	const ham::vec3 forward{ orientation[0], orientation[1], orientation[2] };
//	const ham::vec3 up{ orientation[3], orientation[4], orientation[5] };

	ham::logapiwarn("Getting listener rotation is unimplemented");
	return ham::vec3(0.f, 0.f, 1.f);
}

static inline ham_vec3 ham_audio_oalsoft_velocity(const ham_audio_oalsoft *self){
	ham_vec3 ret;
	alGetListenerfv(AL_VELOCITY, ret.data);
	return ret;
}

static inline void ham_audio_oalsoft_set_position(ham_audio_oalsoft *self, ham_vec3 pos){
	alListenerfv(AL_POSITION, pos.data);
}

static inline void ham_audio_oalsoft_set_rotation(ham_audio_oalsoft *self, ham_vec3 pyr){
	constexpr ham::vec3 basis_z(0.f, 0.f, 1.f);
	constexpr ham::vec3 basis_y(0.f, 1.f, 0.f);

	const ham::quat q_p = ham::quat::from_pitch(pyr.x);
	const ham::quat q_y = ham::quat::from_pitch(pyr.y);
	const ham::quat q_r = ham::quat::from_pitch(pyr.z);

	const ham::quat q_pyr = q_p * q_y * q_r;

	const ham::vec3 forward = q_pyr * basis_z;
	const ham::vec3 up      = q_pyr * basis_y;

	const ham::f32 orientation[] = {
		forward.x(), forward.y(), forward.z(),
		up.x(), up.y(), up.z()
	};

	alListenerfv(AL_ORIENTATION, orientation);
}

static inline void ham_audio_oalsoft_set_velocity(ham_audio_oalsoft *self, ham_vec3 vel){
	alListenerfv(AL_VELOCITY, vel.data);
}

ham_define_audio_object(ham_audio_oalsoft, ham_audio_stream_oalsoft)

HAM_C_API_END
