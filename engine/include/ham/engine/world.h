/*
 * Ham World Engine Runtime
 * Copyright (C) 2022 Hamsmith Ltd.
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

#ifndef HAM_ENGINE_WORLD_H
#define HAM_ENGINE_WORLD_H 1

/**
 * @defgroup HAM_ENGINE_WORLD World management
 * @ingroup HAM_ENGINE
 * @{
 */

#include "ham/engine/config.h"

#include "ham/net.h" // IWYU pragma: keep

#include "entity.h"

HAM_C_API_BEGIN

typedef struct ham_world ham_world;

//! A single world partition, part of a larger world tree
typedef struct ham_world_partition ham_world_partition;

ham_engine_api ham_world *ham_world_create(ham_str8 name);

ham_engine_api void ham_world_destroy(ham_world *world);

HAM_C_API_END

/**
 * @}
 */

#endif // !HAM_ENGINE_WORLD_H
