#version 450 core
#extension GL_ARB_separate_shader_objects : require

#ifdef GL_SPIRV
#extension GL_GOOGLE_include_directive : enable
#else
#extension GL_ARB_shading_language_include : enable
#endif

/*
 * Ham Renderer OpenGL Shaders
 * Copyright (C) 2022 Keith Hammond
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
	vec3 view_pos;
	float near_z, far_z;
	vec2 uv_scale;
	float time;
} globals;

layout(binding = 0)
uniform sampler2D depth_tex;

layout(binding = 1)
uniform sampler2D diffuse_tex;

layout(binding = 2)
uniform sampler2D normal_tex;

//layout(binding = 3) uniform sampler2D material_tex;

layout(binding = 4)
uniform sampler2D scene_tex;

layout(location = 0) in vec3 vert_f;
layout(location = 1) in vec3 norm_f;
layout(location = 2) in vec2 uv_f;

layout(location = 0) out vec4 out_color;

vec3 hsv_to_rgb(in vec3 c){
	const vec4 k = vec4(1.0, 2.0/3.0, 1.0/3.0, 3.0);
	const vec3 p = abs(fract(c.xxx + k.xyz) * 6.0 - k.www);
	return c.z * mix(k.xxx, clamp(p - k.xxx, 0.0, 1.0), c.y);
}

float tonemap_reinhard(in float x){
	return x / (x + 1.0);
}

float tonemap_reinhard2(in float x){
	const float white = 4.0;
	return (x * (1.0 + x / (white * white))) / (1.0 + x);
}

float tonemap_aces(in float x){
	const float a = 2.51;
	const float b = 0.03;
	const float c = 2.43;
	const float d = 0.59;
	const float e = 0.14;
	return (x * (a * x + b)) / (x * (c * x + d) + e);
}

vec3 tonemap_aces(in vec3 x){
  const float a = 2.51;
  const float b = 0.03;
  const float c = 2.43;
  const float d = 0.59;
  const float e = 0.14;
  return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

vec4 screen_quadrants(in vec3 ldr){
	const vec3 albedo = texture(diffuse_tex, uv_f).rgb;
	const vec3 norm   = texture(normal_tex, uv_f).rgb;
	const float d     = texture(depth_tex, uv_f).r;

	const float x_coef = step(0.5, uv_f.x);
	const float y_coef = step(0.5, 1.0 - uv_f.y);

	const vec3 x_mix0 = mix(albedo, norm, x_coef);
	const vec3 x_mix1 = mix(vec3(d), ldr, x_coef);

	const vec3 y_mix = mix(x_mix0, x_mix1, y_coef);

	return vec4(y_mix, 1.0);
}

void main(){
	const vec3 hdr = texture(scene_tex, uv_f).rgb;
	const vec3 ldr = tonemap_aces(hdr);

	//out_color = screen_quadrants(ldr);
	out_color = vec4(ldr, 1.0);
}
