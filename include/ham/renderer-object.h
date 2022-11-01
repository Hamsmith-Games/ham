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
#include "async.h"
#include "buffer.h"

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

	ham_image *default_img;

	ham_object_manager *draw_groups;

	ham_mutex draw_mut;
	ham_buffer draw_list, tmp_list;
};

struct ham_renderer_vtable{
	ham_derive(ham_object_vtable)

	const ham_draw_group_vtable*(*draw_group_vtable)();

	bool(*resize)(ham_renderer *r, ham_u32 w, ham_u32 h);
	void(*frame)(ham_renderer *r, ham_f64 dt, const ham_renderer_frame_data *data);
};

// Draw groups

struct ham_draw_group{
	ham_derive(ham_object)

	ham_renderer *r;
	ham_mutex mut;
	ham_usize num_shapes;
	ham_usize *num_shape_points;
	ham_usize *num_shape_indices;
	const ham_image **images;

	ham_u32 num_instances;
};

struct ham_draw_group_vtable{
	ham_derive(ham_object_vtable)

	bool(*set_num_instances)(ham_draw_group *self, ham_u32 n);
	ham_draw_group_instance_data*(*instance_data)(ham_draw_group *self);
};

//! @cond ignore

#define ham_impl_define_renderer( \
	derived_renderer, \
	derived_draw_group, \
	ctor_fn, dtor_fn, \
	resize_name, resize_fn, \
	frame_name, frame_fn \
) \
ham_extern_c ham_public ham_nothrow const ham_object_vtable *ham_object_vptr_name(derived_draw_group)(); \
ham_nothrow static inline const ham_draw_group_vtable *ham_impl_draw_group_vtable_##derived_renderer(){ return (const ham_draw_group_vtable*)(ham_object_vptr_name(derived_draw_group)()); } \
static inline bool resize_name(ham_renderer *r, ham_u32 w, ham_u32 h){ return (resize_fn)((derived_renderer*)r, w, h); } \
static inline void frame_name(ham_renderer *r, ham_f64 dt, const ham_renderer_frame_data *data){ (frame_fn)((derived_renderer*)r, dt, data); } \
ham_define_object_x( \
	2, derived_renderer, 1, ham_renderer_vtable, \
	ctor_fn, dtor_fn, \
	( \
		.draw_group_vtable = ham_impl_draw_group_vtable_##derived_renderer, \
		.resize = resize_name, \
		.frame = frame_name, \
	) \
)

#define ham_impl_define_draw_group( \
	derived, \
	ctor_fn, dtor_fn \
) \
static inline bool ham_impl_set_num_instances_##derived(ham_draw_group *self, ham_u32 n){ \
	return (derived##_set_num_instances)((derived*)self, n); \
} \
static inline ham_draw_group_instance_data *ham_impl_instance_data_##derived(ham_draw_group *self){ \
	return (derived##_instance_data)((derived*)self); \
} \
ham_define_object_x( \
	2, derived, 1, ham_draw_group_vtable, \
	ctor_fn, dtor_fn, \
	( \
		.set_num_instances = ham_impl_set_num_instances_##derived,\
		.instance_data = ham_impl_instance_data_##derived,\
	) \
)

//! @endcond

#define ham_define_renderer(derived_renderer, derived_draw_group) \
	ham_impl_define_renderer(\
		derived_renderer, \
		derived_draw_group, \
		derived_renderer##_ctor, \
		derived_renderer##_dtor, \
		ham_impl_resize_##derived_renderer, derived_renderer##_resize, \
		ham_impl_frame_##derived_renderer, derived_renderer##_frame \
	)

#define ham_define_draw_group(derived) \
	ham_impl_define_draw_group(derived, derived##_ctor, derived##_dtor)

HAM_C_API_END

#ifdef __cplusplus

namespace ham{
	namespace detail{
		template<> struct ham_object_vtable<ham_renderer>: id<ham_renderer_vtable>{};
		template<> struct ham_object_vtable<ham_draw_group>: id<ham_draw_group_vtable>{};
	}
}

#endif // __cplusplus

/**
 * @}
 */

#endif // !HAM_RENDERER_OBJECT_H
