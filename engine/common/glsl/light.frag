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

#define MAX_MATERIALS 256

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

layout(binding = 3)
uniform sampler2D material_tex;

layout(binding = 5)
uniform sampler2D noise_tex;

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

layout(location = 3) out vec3 out_scene;

const float PI = 3.14159265358979323846;

float geometry_schlick_ggx(in float N_dot_V, float roughness){
	const float r = roughness + 1.0;
	const float k = pow(r, 2.0) / 8.0;

	const float den = N_dot_V * (1.0 - k) + k;
	return N_dot_V / den;
}

float geometry_smith(in vec3 N, in vec3 V, in vec3 L, float roughness){
	const float N_dot_V = max(dot(N, V), 0.0);
	const float N_dot_L = max(dot(N, L), 0.0);
	const float ggx0 = geometry_schlick_ggx(N_dot_V, roughness);
	const float ggx1 = geometry_schlick_ggx(N_dot_L, roughness);
	return ggx0 * ggx1;
}

float distribution_ggx(in vec3 N, in vec3 H, in float roughness){
	const float a        = pow(roughness, 2.0);
	const float a2       = pow(a, 2.0);
	const float N_dot_H  = max(dot(N, H), 0.0);
	const float N_dot_H2 = pow(N_dot_H, 2.0);

	const float den = (N_dot_H2 * (a2 - 1.0) + 1.0);

	return a2 / (PI * pow(den, 2.0));
}

vec3 fresnel_schlick(in float cos_theta, in vec3 F0){
	return F0 + (1.0 - F0) * pow(clamp(1.0 - cos_theta, 0.0, 1.0), 5.0);
}

float calculate_attenuation(in float d){
	return pow(1.0 - min(d/light_radius_f, 1.0), 2.0);
}

vec3 calculate_lighting(
    in vec3 V, in vec3 N, in vec3 L,
    in float d,
    in float metalness,
    in float roughness,
    in vec3 albedo
){
	const vec3 H = normalize(V + L);

	const float atten = calculate_attenuation(d);

	const vec3 radiance = light_color_f * atten;

	const vec3 F0 = mix(vec3(0.04), albedo, metalness);
	const vec3 F = fresnel_schlick(max(dot(H, V), 0.0), F0);

	const float NDF = distribution_ggx(N, H, roughness);
	const float G   = geometry_smith(N, V, L, roughness);

	const vec3 num = NDF * G * F;
	const float den = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
	const vec3 specular = num / den;

	const vec3 kS = F;
	const vec3 kD = (vec3(1.0) - kS) * (1.0 - metalness);
	const vec3 kA = vec3(0.03) * albedo * 1.0;

	const float N_dot_L = max(dot(N, L), 0.0);

	const vec3 lit = (kD * albedo / PI + specular) * radiance * N_dot_L;

	return kA + lit;
}

void main(){
	const float depth = texture(depth_tex, uv_f).r;
	const vec3 pos = reconstruct_world_pos(uv_f, depth);

	const vec3 light_to_world = light_pos_f - pos;

	const float d = length(light_to_world);
	if(d > light_radius_f) discard;

	const vec3 V = normalize(globals.view_pos - pos);
	const vec3 N = normalize(texture(normal_tex, uv_f).rgb);
	const vec3 L = normalize(light_to_world);

	const vec3 albedo = texture(diffuse_tex, uv_f).rgb;
	const vec3 material = texture(material_tex, uv_f).rgb;

	const vec3 lit_color = calculate_lighting(V, N, L, d, material.r, material.g, albedo);

	const vec2 noise_uv = uv_f + (vec2(sin(globals.time), cos(globals.time)) * 1000.0);
	const vec3 rgb_noise = texture(noise_tex, noise_uv).rgb * 0.03125;

	out_scene = pow(lit_color * (1.0 - rgb_noise), vec3(max(light_intensity_f, 1.0)));
}
