#version 450 core
#extension GL_ARB_separate_shader_objects : require

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

#ifdef VULKAN
#	define gl_VertexID gl_VertexIndex
#endif

out gl_PerVertex {
	vec4 gl_Position;
	float gl_PointSize;
};

layout(std140, binding = 0) uniform RenderData{
	mat4 view_proj, inv_view_proj;
	vec3 view_pos;
	float near_z, far_z;
	vec2 uv_scale;
	float time;
} globals;

layout(location = 0) in vec3  light_pos;
layout(location = 1) in float light_radius;
layout(location = 2) in vec3  light_color;
layout(location = 3) in float light_intensity;

layout(location = 0) out vec3  light_pos_f;
layout(location = 1) out float light_radius_f;
layout(location = 2) out vec3  light_color_f;
layout(location = 3) out float light_intensity_f;

layout(location = 4) out vec2 uv_f;

const vec3 verts[] = vec3[](
    vec3(-1.0, -1.0, 0.0),
    vec3( 1.0, -1.0, 0.0),
    vec3( 1.0,  1.0, 0.0),
    vec3(-1.0,  1.0, 0.0)
);

const vec2 uvs[] = vec2[](
    vec2(0.0, 0.0),
    vec2(1.0, 0.0),
    vec2(1.0, 1.0),
    vec2(0.0, 1.0)
);

void main(){
	light_pos_f = light_pos;
	light_radius_f = light_radius;
	light_color_f = light_color;
	light_intensity_f = light_intensity;

	uv_f = uvs[gl_VertexID] * globals.uv_scale;

	gl_Position = vec4(verts[gl_VertexID], 1.0);
}
