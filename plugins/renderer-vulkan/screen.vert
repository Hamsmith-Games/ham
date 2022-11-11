#version 450

/*
 * Ham Renderer Vulkan Shaders
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

#define gl_VertexID gl_VertexIndex
#define gl_InstanceID gl_InstanceIndex

layout(binding = 0) uniform VertexGlobalData{
	mat4 view_proj;
} globals;

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 norm;
layout(location = 2) in vec2 uv;

const vec2 positions[4] = vec2[](
    vec2(-1.0, -1.0),
    vec2( 1.0, -1.0),
    vec2( 1.0,  1.0),
    vec2(-1.0,  1.0)
);

const vec2 uvs[4] = vec2[](
	vec2(0.0, 0.0),
	vec2(1.0, 0.0),
	vec2(1.0, 1.0),
    vec2(0.0, 1.0)
);

layout(location = 0) out vec3 pos_f;
layout(location = 1) out vec3 norm_f;
layout(location = 2) out vec2 uv_f;

void main(){
	pos_f = pos;
	norm_f = norm;
	uv_f = uvs[gl_VertexID];

	gl_Position = vec4(positions[gl_VertexID], 0.0, 1.0);
}
