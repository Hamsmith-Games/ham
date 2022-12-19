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

#ifndef HAM_PHYSICS_H
#define HAM_PHYSICS_H 1

/**
 * @defgroup HAM_PHYSICS Physics
 * @ingroup HAM
 * @{
 */

#include "memory.h"
#include "shape.h"
#include "async.h"

HAM_C_API_BEGIN

typedef struct ham_physics ham_physics;
typedef struct ham_physics_vtable ham_physics_vtable;
typedef const ham_physics_vtable *ham_physics_vptr;

ham_api ham_physics *ham_physics_create_alloc(const ham_allocator *allocator, ham_physics_vptr vptr);

ham_used
static inline ham_physics *ham_physics_create(ham_physics_vptr vptr){
	return ham_physics_create_alloc(ham_current_allocator(), vptr);
}

ham_api ham_nothrow void ham_physics_destroy(ham_physics *phys);

/**
 * @defgroup HAM_PHYSICS_WORLD Physics worlds
 * @{
 */

typedef struct ham_physics_world ham_physics_world;
typedef struct ham_physics_world_vtable ham_physics_world_vtable;
typedef const ham_physics_world_vtable *ham_physics_world_vptr;

ham_api bool ham_physics_world_create_async(ham_physics *phys, ham_async_result *result);

ham_api ham_physics_world *ham_physics_world_create(ham_physics *phys);
ham_api ham_nothrow void ham_physics_world_destroy(ham_physics_world *phys_world);

ham_api void ham_physics_world_tick(ham_physics_world *phys_world, ham_f64 dt);

/**
 * @}
 */

/**
 * @defgroup HAM_PHYSICS_SHAPE Collision shapes
 * @{
 */

typedef struct ham_physics_shape ham_physics_shape;
typedef struct ham_physics_shape_vtable ham_physics_shape_vtable;
typedef const ham_physics_shape_vtable *ham_physics_shape_vptr;

ham_api bool ham_physics_shape_create_async(
	ham_physics_world *phys_world,
	ham_usize num_shapes, const ham_shape *const *shapes, const ham_vec3 *offsets, const ham_quat *orientations,
	ham_async_result *result
);

ham_api ham_physics_shape *ham_physics_shape_create(ham_physics_world *phys_world, ham_usize num_shapes, const ham_shape *const *shapes, const ham_vec3 *offsets, const ham_quat *orientations);
ham_api ham_nothrow void ham_physics_shape_destroy(ham_physics_shape *phys_shape);

/**
 * @}
 */

/**
 * @defgroup HAM_PHYSICS_BODY Bodies
 * @{
 */

typedef struct ham_physics_body ham_physics_body;
typedef struct ham_physics_body_vtable ham_physics_body_vtable;
typedef const ham_physics_body_vtable *ham_physics_body_vptr;

ham_api ham_physics_body *ham_physics_body_create(ham_physics_world *phys_world);
ham_api ham_nothrow void ham_physics_body_destroy(ham_physics_body *phys_body);

/**
 * @}
 */

HAM_C_API_END

#ifdef __cplusplus

namespace ham{
	template<bool IsMutable>
	class basic_physics_body_view: public basic_pointer_view<std::conditional_t<IsMutable, ham_physics_body, const ham_physics_body>>{
		public:
			using Super = basic_pointer_view<std::conditional_t<IsMutable, ham_physics_body, const ham_physics_body>>;
			using Super::Super;
	};

	using physics_body_view = basic_physics_body_view<true>;
	using const_physics_body_view = basic_physics_body_view<false>;

	template<bool IsMutable>
	class basic_physics_world_view: public basic_pointer_view<std::conditional_t<IsMutable, ham_physics_world, const ham_physics_world>>{
		public:
			using Super = basic_pointer_view<std::conditional_t<IsMutable, ham_physics_world, const ham_physics_world>>;
			using Super::Super;
	};

	using physics_world_view = basic_physics_world_view<true>;
	using const_physics_world_view = basic_physics_world_view<false>;

	template<bool IsMutable>
	class basic_physics_view: public basic_pointer_view<std::conditional_t<IsMutable, ham_physics, const ham_physics>>{
		public:
			using Super = basic_pointer_view<std::conditional_t<IsMutable, ham_physics, const ham_physics>>;
			using Super::Super;
	};

	using physics_view = basic_physics_view<true>;
	using const_physics_view = basic_physics_view<false>;

	class physics{
		public:
			explicit physics(ham_physics_vptr vptr, const ham_allocator *allocator = ham_current_allocator())
				: m_handle(ham_physics_create_alloc(allocator, vptr)){}



		private:
			unique_handle<ham_physics*, ham_physics_destroy> m_handle;
	};
}

#endif

/**
 * @}
 */

#endif // !HAM_PHYSICS_H
