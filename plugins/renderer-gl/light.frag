#version 450 core

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

layout(std140, binding = 0) uniform RenderData{
	mat4 view_proj, inv_view_proj;
	float near_z, far_z;
	float time;
} globals;

uniform sampler2D depth_tex;
uniform sampler2D diffuse_tex;
uniform sampler2D normal_tex;

vec3 reconstruct_world_pos(in vec2 uv, in float z){
	const vec4 xyzw = vec4(uv * 2.0 - 1.0, z, 1.0);
	const vec4 projected = globals.inv_view_proj * xyzw;
	return projected.xyz / projected.w;
}

layout(location = 0) in vec3  light_pos_f;
layout(location = 1) in float light_radius_f;
layout(location = 2) in vec3  light_color_f;
layout(location = 3) in float light_intensity_f;

layout(location = 4) in vec2 uv_f;

layout(location = 0) out vec4 out_diffuse;
layout(location = 1) out vec3 out_normal;
layout(location = 2) out vec3 out_scene;

void main(){
	const float depth = texture(depth_tex, uv_f).r;
	const vec3 pos = reconstruct_world_pos(uv_f, depth);

	vec3 light_to_world = pos - light_pos_f;
	float d = distance(pos, light_pos_f);

	if(d > light_radius_f) discard;

	const float atten = pow(1.0 - min(d/light_radius_f, 1.0), 2.0);

	const vec3 albedo = texture(diffuse_tex, uv_f).rgb;

	out_scene = albedo * atten * light_color_f;
}
