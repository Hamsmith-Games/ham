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

enum {
	HAM_GBO_DEPTH_TEXTURE_UNIT,
	HAM_GBO_DIFFUSE_TEXTURE_UNIT,
	HAM_GBO_NORMAL_TEXTURE_UNIT,
	HAM_GBO_MATERIAL_TEXTURE_UNIT,
	HAM_GBO_SCENE_TEXTURE_UNIT,

	HAM_NOISE_TEXTURE_UNIT,
	HAM_DIFFUSE_TEXTURE_UNIT,
	HAM_NORMAL_TEXTURE_UNIT,
	HAM_SPECULAR_TEXTURE_UNIT,
};

typedef enum ham_renderer_gl_fbo_attachment{
	HAM_RENDERER_GL_FBO_DEPTH_STENCIL,
	HAM_RENDERER_GL_FBO_DIFFUSE,
	HAM_RENDERER_GL_FBO_NORMAL,
	HAM_RENDERER_GL_FBO_MATERIAL,
	HAM_RENDERER_GL_FBO_SCENE,

	HAM_RENDERER_GL_FBO_ATTACHMENT_COUNT
} ham_renderer_gl_fbo_attachment;

typedef enum ham_draw_buffer_gl_data{
	HAM_DRAW_BUFFER_GL_POINTS,
	HAM_DRAW_BUFFER_GL_INDICES,
	HAM_DRAW_BUFFER_GL_COMMANDS,
	HAM_DRAW_BUFFER_GL_INSTANCE_DATA,
	HAM_DRAW_BUFFER_GL_BONE_DATA,
	HAM_DRAW_BUFFER_GL_MATERIAL_DATA,

	HAM_DRAW_BUFFER_GL_DATA_COUNT
} ham_draw_buffer_gl_data;

typedef struct ham_draw_group_gl ham_draw_group_gl;

typedef struct ham_renderer_gl_global_ubo_data{
	ham_mat4 view_proj, inv_view_proj;
	ham_vec3 view_pos; float pad0;
	ham_f32 near_z, far_z;
	ham_f32 time;
} ham_renderer_gl_global_ubo_data;

typedef struct ham_renderer_gl_api ham_renderer_gl{
	ham_derive(ham_renderer)

	ham_u32
		fbo,
		fbo_attachments[HAM_RENDERER_GL_FBO_ATTACHMENT_COUNT]
	;

	ham_u32 render_w, render_h;

	ham_u32 scene_info_vert, scene_info_frag, scene_info_pipeline;
	ham_u32 light_vert, light_frag, light_pipeline;
	ham_u32 screen_post_vert, screen_post_frag, screen_post_pipeline;

	// gbo, scene diffuse
	ham_u32 samplers[2];
	ham_u32 noise_tex;

	ham_i32 light_vert_uv_scale_loc;
	ham_i32 light_frag_depth_tex_loc;
	ham_i32 light_frag_diffuse_tex_loc;
	ham_i32 light_frag_normal_tex_loc;
	ham_i32 light_frag_material_tex_loc;
	ham_i32 light_frag_noise_tex_loc;

	ham_i32 scene_info_frag_diffuse_tex_loc;
	ham_i32 screen_post_frag_depth_loc;
	ham_i32 screen_post_frag_diffuse_loc;
	ham_i32 screen_post_frag_normal_loc;
	ham_i32 screen_post_frag_scene_loc;

	ham_draw_group_gl *screen_group;
	ham_i32 screen_post_vert_uv_scale_loc;

	ham_u32 global_ubo;
	void *global_ubo_writep;

	ham_f64 total_time;
} ham_renderer_gl;

ham_renderer_gl_api ham_u32 ham_renderer_gl_load_shader(ham_renderer_gl *r, ham_u32 shader_type, ham_str8 name);
ham_renderer_gl_api ham_u32 ham_renderer_gl_create_pipeline(ham_renderer_gl *r, ham_u32 vert_prog, ham_u32 frag_prog);

struct ham_renderer_gl_api ham_draw_group_gl{
	ham_derive(ham_draw_group)

	ham_u32 mode;
	ham_u32 vao;
	ham_u32 bufs[HAM_DRAW_BUFFER_GL_DATA_COUNT];
	ham_u32 diffuse_tex_arr;

	ham_u32 inst_cap;
	void *inst_map;
};

struct ham_renderer_gl_api ham_light_group_gl{
	ham_derive(ham_light_group)

	ham_u32 vao, vbo, ibo, cbo, buf;

	ham_u32 inst_cap;
	void *inst_map;
};

HAM_C_API_END

#endif // !HAM_RENDERER_GL_RENDERER_GL_HPP
