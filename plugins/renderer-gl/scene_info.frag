#version 450 core
#extension GL_ARB_separate_shader_objects : require
//#extension GL_ARB_fragment_coord_conventions : require

/*
 * Ham Renderer OpenGL Shaders
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

//layout(origin_upper_left) in vec4 gl_FragCoord;

layout(std140, binding = 0) uniform RenderData{
	mat4 view_proj;
	float time;
} globals;

uniform sampler2DArray diffuse_tex;

layout(location = 0) in vec3 vert_f;
layout(location = 1) in vec3 norm_f;
layout(location = 2) in vec2 uv_f;
layout(location = 3) in vec4 color_f;
layout(location = 4) flat in int draw_id_f;

layout(location = 0) out vec4 out_diffuse;
layout(location = 1) out vec3 out_normal;

void main(){
	// TODO: texture mapping
	const vec4 color = texture(diffuse_tex, vec3(uv_f, draw_id_f));

	out_diffuse = color_f * color;
	out_normal  = norm_f;
}
