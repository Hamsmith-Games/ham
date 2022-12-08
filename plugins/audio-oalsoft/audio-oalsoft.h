#ifndef HAM_AUDIO_OALSOFT_AUDIO_OALSOFT_H
#define HAM_AUDIO_OALSOFT_AUDIO_OALSOFT_H 1

#include "ham/audio-object.h"

#include "AL/alc.h"
#include "AL/al.h"

HAM_C_API_BEGIN

typedef struct ham_audio_oalsoft{
	ham_derive(ham_audio)

	ALCdevice *alc_dev;
	ALCcontext *alc_ctx;
} ham_audio_oalsoft;

#define HAM_AUDIO_NUM_BUFFERS 4
#define HAM_AUDIO_BUFFER_SAMPLES 8192

typedef struct ham_audio_stream_oalsoft{
	ham_derive(ham_audio_stream)

	ALuint al_source, al_bufs[HAM_AUDIO_NUM_BUFFERS];

	ham_uptr data_off, play_off;

	ham_vec3 pos, pyr, vel;
} ham_audio_stream_oalsoft;

struct ham_audio_orientation_oalsoft{
	ham_vec3 forward, up;
};

ham_audio_orientation_oalsoft ham_audio_orientation_from_pyr(ham_vec3 pyr);

HAM_C_API_END

#endif // !HAM_AUDIO_OALSOFT_AUDIO_OALSOFT_H
