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
	uint voxel_resolution;
} globals;

layout(std140, binding = 1) uniform BoneData{
	mat4 transforms[MAX_BONES];
} bones;

// Vertex attributes
layout(location = 0) in vec3 vert;
layout(location = 1) in vec3 norm;
layout(location = 2) in vec2 uv;
layout(location = 3) in ivec4 bone_indices;
layout(location = 4) in vec4 bone_weights;

// Instance attributes
layout(location = 5) in uint material_id;
layout(location = 6) in mat4 trans;
layout(location = 10) in mat4 normal_mat;

// Outputs
layout(location = 0) out vec3 vert_v;
layout(location = 1) out vec3 norm_v;
layout(location = 2) out vec2 uv_v;
layout(location = 3) flat out int draw_id_v;
layout(location = 4) flat out uint material_id_v;
layout(location = 5) out vec3 voxel_pos_v;

void main(){
	vec4 final_vert = vec4(0.0, 0.0, 0.0, 1.0);
	vec4 final_norm = vec4(0.0, 0.0, 0.0, 0.0);

	for(int index_i = 0; index_i < 4; index_i++){
		const int bone_i = bone_indices[index_i];

		const float bone_w = bone_weights[index_i];

		const vec4 bone_v = bones.transforms[bone_i] * vec4(vert, 1.0);
		const vec4 bone_n = bones.transforms[bone_i] * vec4(norm, 0.0);

		final_vert += bone_v * bone_w;
		final_norm += bone_n * bone_w;
	}

	vert_v        = final_vert.xyz;
	norm_v        = normalize((normal_mat * vec4(final_norm.xyz, 1.0)).xyz);
	uv_v          = uv;
	draw_id_v     = gl_DrawID;
	material_id_v = material_id;
	voxel_pos_v   = final_vert.xyz * float(globals.voxel_resolution);

	const mat4 mvp = globals.view_proj * trans;

	gl_Position = mvp * vec4(final_vert.xyz, 1.0);
}
