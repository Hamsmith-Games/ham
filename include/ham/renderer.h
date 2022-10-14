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

#ifndef HAM_RENDERER_H
#define HAM_RENDERER_H 1

/**
 * @defgroup HAM_RENDERER Rendering
 * @ingroup HAM
 * @{
 */

#include "shape.h"
#include "vk.h" // IWYU pragma: keep

#include <stdarg.h>

HAM_C_API_BEGIN

typedef struct ham_camera{
	ham_mat4 proj, view;
	ham_vec3 pos, pyr;

	//! @cond ignore
	bool _ham_dirty;
	//! @endcond
} ham_camera;

ham_nonnull_args(1)
ham_api void ham_camera_reset_perspective(ham_camera *cam, ham_f32 aspect, ham_f32 fovy, ham_f32 minz, ham_f32 maxz);

ham_nonnull_args(1)
ham_api void ham_camera_reset_ortho(ham_camera *cam, ham_f32 top, ham_f32 bottom, ham_f32 left, ham_f32 right, ham_f32 minz, ham_f32 maxz);

ham_nonnull_args(1)
ham_constexpr ham_nothrow static inline const ham_mat4 *ham_camera_proj_matrix(const ham_camera *cam){
	return &cam->proj;
}

ham_nonnull_args(1)
ham_api const ham_mat4 *ham_camera_view_matrix(const ham_camera *cam);

ham_nonnull_args(1)
ham_constexpr ham_nothrow static inline void ham_camera_rotate(ham_camera *cam, ham_f32 rads, ham_vec3 axis){
	cam->pyr.x += axis.x * rads;
	cam->pyr.y += axis.y * rads;
	cam->pyr.z += axis.z * rads;
	cam->_ham_dirty = true;
}

ham_nonnull_args(1)
ham_constexpr ham_nothrow static inline void ham_camera_translate(ham_camera *cam, ham_vec3 amnt){
	cam->pos.x += amnt.x;
	cam->pos.y += amnt.y;
	cam->pos.z += amnt.z;
	cam->_ham_dirty = true;
}

ham_nonnull_args(1)
ham_constexpr ham_nothrow static inline void ham_camera_set_rotation(ham_camera *cam, ham_vec3 pyr){
	cam->pyr = pyr;
	cam->_ham_dirty = true;
}

ham_nonnull_args(1)
ham_constexpr ham_nothrow static inline void ham_camera_set_position(ham_camera *cam, ham_vec3 pos){
	cam->pos = pos;
	cam->_ham_dirty = true;
}

typedef void*(*ham_gl_get_proc_addr)(const char *name);

typedef struct ham_renderer_create_args_vulkan{
	VkInstance instance;
	VkSurfaceKHR surface;
	PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr;
} ham_renderer_create_args_vulkan;

typedef struct ham_renderer_create_args_gl{
	ham_uptr context_handle;
	ham_gl_get_proc_addr glGetProcAddr;
} ham_renderer_create_args_gl;

typedef union ham_renderer_create_args{
	ham_renderer_create_args_vulkan vulkan;
	ham_renderer_create_args_gl gl;
} ham_renderer_create_args;

typedef struct ham_renderer_frame_data_common{
	ham_u64 current_frame;
	const ham_camera *cam;
} ham_renderer_frame_data_common;

typedef struct ham_renderer_frame_data_vulkan{
	ham_renderer_frame_data_common common;
	VkCommandBuffer command_buffer;
	VkFramebuffer framebuffer;
	VkRenderPass default_render_pass;
} ham_renderer_frame_data_vulkan;

typedef struct ham_renderer_frame_data_gl{
	ham_renderer_frame_data_common common;
} ham_renderer_frame_data_gl;

typedef union ham_renderer_frame_data{
	ham_renderer_frame_data_common common;
	ham_renderer_frame_data_vulkan vulkan;
	ham_renderer_frame_data_gl gl;
} ham_renderer_frame_data;

typedef struct ham_renderer ham_renderer;
typedef struct ham_renderer_vtable ham_renderer_vtable;

ham_api ham_renderer *ham_renderer_vptr_create(const ham_renderer_vtable *vptr, const ham_renderer_create_args *args);

ham_api ham_renderer *ham_renderer_create(const char *plugin_id, const char *obj_id, const ham_renderer_create_args *args);

ham_api void ham_renderer_destroy(ham_renderer *renderer);

ham_api bool ham_renderer_resize(ham_renderer *renderer, ham_u32 w, ham_u32 h);

ham_api void ham_renderer_frame(ham_renderer *renderer, ham_f64 dt, const ham_renderer_frame_data *data);

/**
 * @defgroup HAM_RENDERER_TEXTURE Textures
 * @{
 */

typedef struct ham_texture ham_texture;

/**
 * @}
 */

/**
 * @defgroup HAM_RENDERER_DRAW_GROUP Draw groups
 * @{
 */

typedef struct ham_draw_group ham_draw_group;

// typedef struct ham_draw

ham_api ham_draw_group *ham_draw_group_create(
	ham_renderer *r,
	ham_usize num_shapes, const ham_shape *const *shapes
);

ham_api void ham_draw_group_destroy(ham_draw_group *group);

typedef bool(*ham_draw_group_instance_iterate_fn);

ham_api ham_usize ham_draw_group_instance_iterate(
	ham_draw_group *group,
	ham_draw_group_instance_iterate_fn fn,
	void *user
);

static inline ham_usize ham_draw_group_num_instances(ham_draw_group *group){
	return ham_draw_group_instance_iterate(group, ham_null, ham_null);
}

/**
 * @}
 */

HAM_C_API_END

namespace ham{
	class renderer{
		public:
			renderer(){}

		private:
			unique_handle<ham_renderer*, ham_renderer_destroy> m_handle;
	};
}

/**
 * @}
 */

#endif // !HAM_RENDERER_H
