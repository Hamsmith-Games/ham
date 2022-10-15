#ifndef HAM_AUDIO_OALSOFT_AUDIO_OALSOFT_H
#define HAM_AUDIO_OALSOFT_AUDIO_OALSOFT_H 1

#include "ham/audio-object.h"

#include "AL/alc.h"
#include "AL/al.h"

HAM_C_API_BEGIN

typedef struct ham_audio_context_oalsoft{
	ham_derive(ham_audio_context)

	ALCdevice *alc_dev;
	ALCcontext *alc_ctx;
} ham_audio_context_oalsoft;

typedef struct ham_audio_listener_oalsoft{
	ham_derive(ham_audio_listener)
} ham_audio_listener_oalsoft;

typedef struct ham_audio_source_oalsoft{
	ham_derive(ham_audio_source)
} ham_audio_source_oalsoft;

HAM_C_API_END

#endif // !HAM_AUDIO_OALSOFT_AUDIO_OALSOFT_H
