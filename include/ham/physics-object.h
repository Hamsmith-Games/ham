/*
 * Ham Runtime
 * Copyright (C) 2022 Keith Hammond
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

#ifndef HAM_PHYSICS_OBJECT_H
#define HAM_PHYSICS_OBJECT_H 1

/**
 * @defgroup HAM_PHYSICS_OBJECT Physics object definitions
 * @ingroup HAM_PHYSICS
 * @{
 */

#include "ham/physics.h"
#include "ham/object.h"
#include "ham/functional.h"

HAM_C_API_BEGIN

//
// Context management
//

struct ham_physics{
	ham_derive(ham_object)

	const ham_allocator *allocator;
	ham_object_manager *world_man;

	ham_workgroup workgroup;
};

struct ham_physics_vtable{
	ham_derive(ham_object_vtable)

	ham_physics_world_vptr(*physics_world_vptr)();
	ham_physics_shape_vptr(*physics_shape_vptr)();
	ham_physics_body_vptr(*physics_body_vptr)();

	void(*tick)(ham_physics *self, ham_f64 dt);
};

//
// Physics worlds
//

struct ham_physics_world{
	ham_derive(ham_object)

	ham_physics *phys;
	ham_object_manager *shape_man, *body_man;
};

struct ham_physics_world_vtable{
	ham_derive(ham_object_vtable)

	void(*tick)(ham_physics_world *self, ham_f64 dt);
};

//
// Physics shapes
//

struct ham_physics_shape{
	ham_derive(ham_object)
	ham_physics_world *phys_world;
};

struct ham_physics_shape_vtable{
	ham_derive(ham_object_vtable)
};

//
// Physics bodies
//

struct ham_physics_body{
	ham_derive(ham_object)
	ham_physics_world *phys_world;
};

struct ham_physics_body_vtable{
	ham_derive(ham_object_vtable)
};

//
// Helper macros
//

//! @cond ignore

#define ham_impl_def_physics_world_object(derived) \
	ham_define_object_x(2, derived, 1, ham_physics_world_vtable, derived##_ctor, derived##_dtor, ())

#define ham_impl_def_physics_shape_object(derived) \
	ham_define_object_x(2, derived, 1, ham_physics_shape_vtable, derived##_ctor, derived##_dtor, ())

#define ham_impl_def_physics_body_object(derived) \
	ham_define_object_x(2, derived, 1, ham_physics_body_vtable, derived##_ctor, derived##_dtor, ())

#define ham_impl_def_physics_object(derived, derived_world, derived_shape, derived_body) \
	ham_extern_c ham_public ham_nothrow const ham_object_vtable *ham_object_vptr_name(derived_world)(); \
	ham_extern_c ham_public ham_nothrow const ham_object_vtable *ham_object_vptr_name(derived_shape)(); \
	ham_extern_c ham_public ham_nothrow const ham_object_vtable *ham_object_vptr_name(derived_body)(); \
	ham_nothrow static inline ham_physics_world_vptr ham_impl_world_vptr_##derived(){ return (ham_physics_world_vptr)(ham_object_vptr_name(derived_world)()); } \
	ham_nothrow static inline ham_physics_shape_vptr ham_impl_shape_vptr_##derived(){ return (ham_physics_shape_vptr)(ham_object_vptr_name(derived_shape)()); } \
	ham_nothrow static inline ham_physics_body_vptr ham_impl_body_vptr_##derived(){ return (ham_physics_body_vptr)(ham_object_vptr_name(derived_body)()); } \
	 \
	static inline void ham_impl_tick_##derived(ham_physics *phys, ham_f64 dt){ derived##_tick((derived*)phys, dt); } \
	 \
	ham_define_object_x(2, derived, 1, ham_physics_vtable, derived##_ctor, derived##_dtor, ( \
		.physics_world_vptr = ham_impl_world_vptr_##derived, \
		.physics_shape_vptr = ham_impl_shape_vptr_##derived, \
		.physics_body_vptr = ham_impl_body_vptr_##derived, \
		.tick = ham_impl_tick_##derived, \
	))

//! @endcond

#define ham_def_physics_world_object(derived) \
	ham_impl_def_physics_world_object(derived)

#define ham_def_physics_shape_object(derived) \
	ham_impl_def_physics_shape_object(derived)

#define ham_def_physics_body_object(derived) \
	ham_impl_def_physics_body_object(derived)

#define ham_def_physics_object(derived, derived_world, derived_shape, derived_body) \
	ham_impl_def_physics_object(derived, derived_world, derived_shape, derived_body)

HAM_C_API_END

/**
 * @}
 */

#endif // !HAM_PHYSICS_OBJECT_H
