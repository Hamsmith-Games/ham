#version 450 core
#extension GL_ARB_separate_shader_objects : require

#ifdef GL_SPIRV
#extension GL_GOOGLE_include_directive : enable
#else
#extension GL_ARB_shading_language_include : enable
#endif

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
	float time;
	mat4 view_proj;
} globals;

uniform sampler2D depth_tex;
uniform sampler2D diffuse_tex;
uniform sampler2D normal_tex;

layout(location = 0) in vec3 vert_f;
layout(location = 1) in vec3 norm_f;
layout(location = 2) in vec2 uv_f;

layout(location = 0) out vec4 out_color;

vec3 hsv_to_rgb(in vec3 c){
	const vec4 k = vec4(1.0, 2.0/3.0, 1.0/3.0, 3.0);
	const vec3 p = abs(fract(c.xxx + k.xyz) * 6.0 - k.www);
	return c.z * mix(k.xxx, clamp(p - k.xxx, 0.0, 1.0), c.y);
}

void main(){
	const vec2 hxy = uv_f * (vec2(sin(globals.time), cos(globals.time)) * 0.5 + 1.0);
	const float hue = ((hxy.x + hxy.y) * 0.5);
	out_color = vec4(hsv_to_rgb(vec3(hue, 1.0, 1.0)), 1.0);
}
