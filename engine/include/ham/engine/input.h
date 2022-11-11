/*
 * Ham World Engine Runtime
 * Copyright (C) 2022 Keith Hammond
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

#ifndef HAM_ENGINE_INPUT_H
#define HAM_ENGINE_INPUT_H 1

/**
 * @defgroup HAM_ENGINE_INPUT Input management
 * @ingroup HAM_ENGINE
 * @{
 */

#include "ham/engine/config.h"

#include "ham/typedefs.h"

HAM_C_API_BEGIN

typedef struct ham_input_device ham_input_device;

typedef void(*ham_input_device_pointer_motion_fn)(const ham_input_device *dev, ham_i32 xrel, ham_i32 yrel);

typedef struct ham_input_manager ham_input_manager;

ham_engine_api ham_input_manager *ham_input_manager_create();

ham_engine_api void ham_input_manager_destroy(ham_input_manager *input);

HAM_C_API_END

/**
 * @}
 */

#endif // !HAM_ENGINE_INPUT_H
