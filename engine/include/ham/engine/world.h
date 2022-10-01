/*
 * Ham World Engine Runtime
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

#ifndef HAM_ENGINE_WORLD_H
#define HAM_ENGINE_WORLD_H 1

/**
 * @defgroup HAM_ENGINE_WORLD World management
 * @ingroup HAM_ENGINE
 * @{
 */

#include "ham/engine/config.h"

#include "ham/object.h"

HAM_C_API_BEGIN

typedef struct ham_world ham_world;

/**
 * @defgroup HAM_ENGINE_WORLD_ENTITY Entities
 * @{
 */

typedef struct ham_entity ham_entity;
typedef struct ham_entity_vtable ham_entity_vtable;

struct ham_entity{
	ham_derive(ham_object)
	ham_world *world;
};

struct ham_entity_vtable{
	ham_derive(ham_object_vtable)

	void(*loop)(ham_entity *self, ham_f64 dt);
};

ham_engine_api ham_entity *ham_entity_vcreate(
	ham_world *world, const ham_entity_vtable *entity_vt,
	ham_usize nargs, va_list va
);

ham_engine_api void ham_entity_destroy(ham_entity *ent);

//! @cond ignore
static inline ham_entity *ham_impl_entity_create(
	ham_world *world, const ham_entity_vtable *entity_vt,
	ham_usize nargs, ...
){
	va_list va;
	va_start(va, nargs);
	ham_entity *const ret = ham_entity_vcreate(world, entity_vt, nargs, va);
	va_end(va);
	return ret;
}
//! @endcond

#define ham_entity_create(world, entity_vt, ...) \
	(ham_impl_entity_create((world), (entity_vt), HAM_NARGS(__VA_ARGS__) __VA_OPT__(,) __VA_ARGS__))

/**
 * @}
 */

HAM_C_API_END

/**
 * @}
 */

#endif // !HAM_ENGINE_WORLD_H
