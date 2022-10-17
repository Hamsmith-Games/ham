/*
 * Ham Runtime Plugins
 * Copyright (C) 2022 Hamsmith Ltd.
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

#ifndef HAM_RENDERER_GL_RENDERER_GL_HPP
#define HAM_RENDERER_GL_RENDERER_GL_HPP 1

#include "ham/renderer-object.h" // IWYU pragma: keep

#ifdef HAM_RENDERER_GL_IMPLEMENTATION
#	define ham_renderer_gl_api ham_private ham_export
#else
#	define ham_renderer_gl_api ham_private ham_import
#endif

HAM_C_API_BEGIN

typedef enum ham_renderer_gl_fbo_attachment{
	HAM_RENDERER_GL_FBO_DEPTH_STENCIL,
	HAM_RENDERER_GL_FBO_DIFFUSE,
	HAM_RENDERER_GL_FBO_NORMAL,
	HAM_RENDERER_GL_FBO_SCENE,

	HAM_RENDERER_GL_FBO_ATTACHMENT_COUNT
} ham_renderer_gl_fbo_attachment;

typedef enum ham_draw_buffer_gl_data{
	HAM_DRAW_BUFFER_GL_POINTS,
	HAM_DRAW_BUFFER_GL_INDICES,
	HAM_DRAW_BUFFER_GL_COMMANDS,

	HAM_DRAW_BUFFER_GL_DATA_COUNT
} ham_draw_buffer_gl_data;

typedef struct ham_draw_group_gl ham_draw_group_gl;

typedef struct ham_renderer_gl_global_ubo_data{
	ham_f32 time; ham_f32 _pad0[3];
	ham_mat4 view_proj;
} ham_renderer_gl_global_ubo_data;

typedef struct ham_renderer_gl_api ham_renderer_gl{
	ham_derive(ham_renderer)

	ham_u32
		fbo,
		fbo_attachments[HAM_RENDERER_GL_FBO_ATTACHMENT_COUNT]
	;

	ham_u32 render_w, render_h;

	ham_u32 scene_info_vert, scene_info_frag, scene_info_pipeline;
	ham_u32 screen_post_vert, screen_post_frag, screen_post_pipeline;

	ham_draw_group_gl *screen_group;

	ham_u32 global_ubo;
	void *global_ubo_writep;
	ham_uptr global_time_offset, global_view_proj_offset;

	ham_f64 total_time;
} ham_renderer_gl;

ham_renderer_gl_api ham_u32 ham_renderer_gl_load_shader(ham_renderer_gl *r, ham_u32 shader_type, ham_str8 name);
ham_renderer_gl_api ham_u32 ham_renderer_gl_create_pipeline(ham_renderer_gl *r, ham_u32 vert_prog, ham_u32 frag_prog);

struct ham_renderer_gl_api ham_draw_group_gl{
	ham_derive(ham_draw_group)

	ham_u32 num_instances;
	ham_u32 vao;
	ham_u32 bufs[HAM_DRAW_BUFFER_GL_DATA_COUNT];
};

HAM_C_API_END

#endif // !HAM_RENDERER_GL_RENDERER_GL_HPP
