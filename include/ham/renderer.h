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
#include "camera.h"
#include "vk.h" // IWYU pragma: keep

#include <stdarg.h>

HAM_C_API_BEGIN

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
