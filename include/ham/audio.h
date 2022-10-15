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

typedef struct ham_audio_context ham_audio_context;
typedef struct ham_audio_context_vtable ham_audio_context_vtable;

typedef struct ham_audio_listener ham_audio_listener;
typedef struct ham_audio_source ham_audio_source;

ham_api ham_audio_context *ham_audio_context_create(const ham_audio_context_vtable *vptr);
ham_api void ham_audio_context_destroy();

ham_api ham_audio_listener *ham_audio_get_listener(ham_audio_context *ctx, ham_u32 idx);

ham_api ham_audio_source *ham_audio_source_create(ham_audio_context *ctx);
ham_api void ham_audio_source_destroy(ham_audio_source *source, ham_u32 num_channels, ham_u32 sample_rate);

ham_api bool ham_audio_source_play(ham_audio_source *source);
ham_api bool ham_audio_source_pause(ham_audio_source *source);
ham_api bool ham_audio_source_stop(ham_audio_source *source);

HAM_C_API_END

/**
 * @}
 */

#endif // !HAM_AUDIO_H
