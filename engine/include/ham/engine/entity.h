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

#ifndef HAME_ENGINE_ENTITY_H
#define HAME_ENGINE_ENTITY_H 1

typedef struct ham_world_partition ham_world_partition;

/**
 * @defgroup HAM_ENGINE_ENTITY Entities
 * @ingroup HAM_ENGINE
 * @{
 */

#include "ham/engine/config.h"

#include "ham/object.h"
#include "ham/buffer.h"
#include "ham/transform.h"

HAM_C_API_BEGIN

ham_declare_object(ham_entity, ham_object)

/**
 * @defgroup HAM_ENGINE_ENTITY_COMPONENTS Components
 * @{
 */

ham_declare_object(ham_entity_component, ham_object)

struct ham_entity_component{
	ham_derive(ham_object)

	ham_entity *ent;
};

struct ham_entity_component_vtable{
	ham_derive(ham_object_vtable)

	void(*update)(ham_entity_component *self, ham_f64 dt);
};

ham_engine_api ham_entity_component *ham_entity_component_vcreate(ham_entity *ent, const ham_entity_component_vtable *comp_vptr, ham_u32 nargs, va_list va);

//! @cond ignore
ham_used static inline ham_entity_component *ham_impl_entity_component_create(ham_entity *ent, const ham_entity_component_vtable *comp_vptr, ham_u32 nargs, ...){
	va_list va;
	va_start(va, nargs);
	ham_entity_component *const ret = ham_entity_component_vcreate(ent, comp_vptr, nargs, va);
	va_end(va);
	return ret;
}
//! @endcond

#define ham_entity_component_create(ent, comp_vptr, ...) \
	ham_impl_entity_component_create((ent), (comp_vptr), HAM_NARGS(__VA_ARGS__) __VA_OPT__(,) __VA_ARGS__)

/**
 * @}
 */

struct ham_entity{
	ham_derive(ham_object)

	//! @brief Transform within \ref world_partition .
	ham_transform transform;

	//! @brief World partition this entity belongs to.
	ham_world_partition *world_partition;

	//! @brief Parent of this entity.
	ham_entity *parent;

	//! @brief Buffer of `ham_entity*`.
	ham_buffer children;

	//! @brief Buffer of `ham_entity_component*`.
	ham_buffer components;
};

struct ham_entity_vtable{
	ham_derive(ham_object_vtable)

	void(*tick)(ham_entity *self, ham_f64 dt);
};

HAM_C_API_END

#ifdef __cplusplus

namespace ham::engine{
}

#endif // __cplusplus

/**
 * @}
 */

#endif // !HAME_ENGINE_ENTITY_H
