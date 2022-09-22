/*
 * The Ham World Engine
 * Copyright (C) 2022  Hamsmith Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef HAM_ENGINE_H
#define HAM_ENGINE_H 1

/**
 * @defgroup HAM_ENGINE Ham World Engine
 * @{
 */

#include "ham/engine/config.h"
#include "ham/typedefs.h"

#define HAM_ENGINE_VERSION ((ham_version){HAM_ENGINE_VERSION_MAJOR,HAM_ENGINE_VERSION_MINOR,HAM_ENGINE_VERSION_PATCH})

HAM_C_API_BEGIN

ham_engine_api ham_nothrow ham_version ham_engine_version();

ham_engine_api ham_nothrow const char *ham_engine_version_line();

typedef struct ham_screen{
	ham_u32 w, h, refresh;
} ham_screen;

/**
 * @defgroup HAM_ENGINE_CTX Context management
 * @{
 */

typedef struct ham_engine_context ham_engine_context;

ham_engine_api ham_engine_context *ham_engine_create(const char *vtable_id, int argc, char **argv);

ham_engine_api void ham_engine_destroy(ham_engine_context *ctx);

ham_engine_api bool ham_engine_request_exit(ham_engine_context *ctx);

ham_engine_api int ham_engine_exec(ham_engine_context *ctx);

/**
 * @}
 */

HAM_C_API_END

/**
 * @}
 */

#endif // !HAM_ENGINE_H
