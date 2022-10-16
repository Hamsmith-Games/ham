/*
 * Ham Runtime
 * Copyright (C) 2022 Hamsmith Ltd.
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

#ifndef HAM_AUDIO_H
#define HAM_AUDIO_H 1

/**
 * @defgroup HAM_AUDIO Spatial Audio
 * @ingroup HAM
 * @{
 */

#include "typedefs.h"

HAM_C_API_BEGIN

typedef enum ham_audio_sample_format{
	HAM_AUDIO_SAMPLE_U8,
	HAM_AUDIO_SAMPLE_U16,
	HAM_AUDIO_SAMPLE_U32,
	HAM_AUDIO_SAMPLE_F32,

	HAM_AUDIO_SAMPLE_FORMAT_COUNT
} ham_audio_sample_format;

typedef struct ham_audio ham_audio;
typedef struct ham_audio_stream ham_audio_stream;

typedef struct ham_audio_vtable ham_audio_vtable;

ham_api ham_audio *ham_audio_create(const ham_audio_vtable *vptr);
ham_api void ham_audio_destroy();

ham_api ham_vec3 ham_audio_position(const ham_audio *ctx);
ham_api ham_vec3 ham_audio_rotation(const ham_audio *ctx);
ham_api ham_vec3 ham_audio_velocity(const ham_audio *ctx);

ham_api void ham_audio_set_position(ham_audio *ctx, ham_vec3 pos);
ham_api void ham_audio_set_rotation(ham_audio *ctx, ham_vec3 pyr);
ham_api void ham_audio_set_velocity(ham_audio *ctx, ham_vec3 vel);

ham_api ham_audio_stream *ham_audio_stream_create(ham_audio *ctx, ham_audio_sample_format sample_format, ham_u32 num_channels, ham_u32 sample_rate);
ham_api void ham_audio_stream_destroy(ham_audio_stream *stream);

ham_api ham_vec3 ham_audio_stream_position(const ham_audio_stream *stream);
ham_api ham_vec3 ham_audio_stream_rotation(const ham_audio_stream *stream);
ham_api ham_vec3 ham_audio_stream_velocity(const ham_audio_stream *stream);

ham_api void ham_audio_stream_set_position(ham_audio_stream *stream, ham_vec3 pos);
ham_api void ham_audio_stream_set_rotation(ham_audio_stream *stream, ham_vec3 pyr);
ham_api void ham_audio_stream_set_velocity(ham_audio_stream *stream, ham_vec3 vel);

ham_api bool ham_audio_stream_play (ham_audio_stream *source);
ham_api bool ham_audio_stream_pause(ham_audio_stream *source);
ham_api bool ham_audio_stream_stop (ham_audio_stream *source);

HAM_C_API_END

/**
 * @}
 */

#endif // !HAM_AUDIO_H
