/*
 * Ham Runtime
 * Copyright (C) 2022  Hamsmith Ltd.
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

#ifndef HAM_RENDERER_OBJECT_H
#define HAM_RENDERER_OBJECT_H 1

/**
 * @defgroup HAM_RENDERER_OBJECT Renderer objects
 * @ingroup HAM_RENDERER
 * @{
 */

#include "renderer.h"
#include "object.h"
#include "memory.h"
#include "plugin.h"

HAM_C_API_BEGIN

// vtable forward declarations

typedef struct ham_renderer_vtable ham_renderer_vtable;
typedef struct ham_draw_group_vtable ham_draw_group_vtable;

// Renderer itself

struct ham_renderer{
	ham_derive(ham_object)

	const ham_allocator *allocator;
	ham_dso_handle dso;
	ham_plugin *plugin;

	ham_object_manager *draw_groups;
};

struct ham_renderer_vtable{
	ham_derive(ham_object_vtable)

	const ham_draw_group_vtable*(*draw_group_vtable)();

	bool(*init)(ham_renderer *r);
	void(*fini)(ham_renderer *r);
	void(*loop)(ham_renderer *r, ham_f64 dt);
};

// Draw groups

struct ham_draw_group{
	ham_derive(ham_object)

	ham_renderer *r;
	ham_usize num_shapes;
	ham_usize *num_shape_points;
	ham_usize *num_shape_indices;
};

struct ham_draw_group_vtable{
	ham_derive(ham_object_vtable)

	bool(*init)(
		ham_draw_group *group,
		ham_renderer *r,
		ham_usize num_shapes, const ham_shape *const *shapes
	);

	void(*fini)(ham_draw_group *group);
};

//! @cond ignore
#define ham_impl_define_renderer( \
	derived_renderer, \
	derived_draw_group, \
	ctor_fn, dtor_fn, \
	init_name, init_fn, \
	fini_name, fini_fn, \
	loop_name, loop_fn \
) \
static inline bool ham_impl_init_##derived_draw_group( \
	ham_draw_group *group, \
	ham_renderer *r, \
	ham_usize num_shapes, const ham_shape *const *shapes \
){ \
	return (derived_draw_group##_init)((derived_draw_group*)group, (derived_renderer*)r, num_shapes, shapes); \
}\
static inline void ham_impl_fini_##derived_draw_group(ham_draw_group *group){ \
	(derived_draw_group##_fini)((derived_draw_group*)group); \
} \
ham_define_object_x( \
	2, derived_draw_group, 1, ham_draw_group_vtable, \
	derived_draw_group##_ctor, derived_draw_group##_dtor, \
	( .init = ham_impl_init_##derived_draw_group, .fini = ham_impl_fini_##derived_draw_group ) \
) \
static inline const ham_draw_group_vtable *ham_impl_draw_group_vtable_##derived_renderer(){ \
	return (const ham_draw_group_vtable*)(ham_impl_object_vtable_name(derived_draw_group)());\
} \
static inline bool ham_impl_init_##derived_renderer(ham_renderer *r){ return (init_fn)((derived_renderer*)r); } \
static inline void ham_impl_fini_##derived_renderer(ham_renderer *r){ (fini_fn)((derived_renderer*)r); } \
static inline void ham_impl_loop_##derived_renderer(ham_renderer *r, ham_f64 dt){ (loop_fn)((derived_renderer*)r, dt); } \
ham_define_object_x( \
	2, derived_renderer, 1, ham_renderer_vtable, \
	ctor_fn, dtor_fn, \
	(	.draw_group_vtable = ham_impl_draw_group_vtable_##derived_renderer, \
		.init = ham_impl_init_##derived_renderer, \
		.fini = ham_impl_fini_##derived_renderer, \
		.loop = ham_impl_loop_##derived_renderer ) \
)
//! @endcond

#define ham_define_renderer(derived_renderer, derived_draw_group)\
	ham_impl_define_renderer(\
		derived_renderer, \
		derived_draw_group, \
		derived_renderer##_ctor, \
		derived_renderer##_dtor, \
		ham_impl_init_##derived_renderer, derived_renderer##_init, \
		ham_impl_fini_##derived_renderer, derived_renderer##_fini, \
		ham_impl_loop_##derived_renderer, derived_renderer##_loop \
	)

HAM_C_API_END

/**
 * @}
 */

#endif // !HAM_RENDERER_OBJECT_H
