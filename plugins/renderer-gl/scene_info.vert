#version 450 core
#extension GL_ARB_separate_shader_objects : require
#extension GL_ARB_shader_draw_parameters : require

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

#define gl_DrawID gl_DrawIDARB

out gl_PerVertex {
	vec4 gl_Position;
	float gl_PointSize;
};

layout(std140, binding = 0) uniform RenderData{
	mat4 view_proj, inv_view_proj;
	float near_z, far_z;
	float time;
} globals;

layout(location = 0) in vec3 vert;
layout(location = 1) in vec3 norm;
layout(location = 2) in vec2 uv;

layout(location = 3) in vec4 color;
layout(location = 4) in mat4 trans;

layout(location = 0) out vec3 vert_f;
layout(location = 1) out vec3 norm_f;
layout(location = 2) out vec2 uv_f;
layout(location = 3) out vec4 color_f;
layout(location = 4) out int draw_id_f;

void main(){
	vert_f    = vert;
	norm_f    = norm;
	uv_f      = uv;
	color_f   = color;
	draw_id_f = gl_DrawID;

	const mat4 mvp = globals.view_proj * trans;

	gl_Position = mvp * vec4(vert, 1.0);
}
