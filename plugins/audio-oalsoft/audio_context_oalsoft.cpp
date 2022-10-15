#include "audio-oalsoft.h"

#include "ham/log.h"

#include "AL/alc.h"
#include "AL/alext.h"

HAM_C_API_BEGIN

static inline ham_audio_context_oalsoft *ham_audio_context_oalsoft_ctor(ham_audio_context_oalsoft *ptr, ham_u32 nargs, va_list va){
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

	const auto ret = new(ptr) ham_audio_context_oalsoft;

	ret->alc_dev = dev;
	ret->alc_ctx = ctx;

	return ret;
}

static inline void ham_audio_context_oalsoft_dtor(ham_audio_context_oalsoft *ctx){
	alcDestroyContext(ctx->alc_ctx);
	alcCloseDevice(ctx->alc_dev);
	std::destroy_at(ctx);
}

ham_define_audio_context_object(ham_audio_context_oalsoft, ham_audio_listener_oalsoft, ham_audio_source_oalsoft)

HAM_C_API_END
