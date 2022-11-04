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

out gl_PerVertex {
	vec4 gl_Position;
	float gl_PointSize;
};

#define MAX_BONES 256

layout(std140, binding = 0) uniform RenderData{
	mat4 view_proj, inv_view_proj;
	float near_z, far_z;
	float time;
} globals;

layout(std140, binding = 1) uniform BoneData{
	mat4 transforms[MAX_BONES];
} bones;

layout(location = 0) in vec3 vert;
layout(location = 1) in vec3 norm;
layout(location = 2) in vec2 uv;
layout(location = 3) in ivec4 bone_indices;
layout(location = 4) in vec4 bone_weights;

void main(){
	
}
