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

const vec3 verts[] = vec3[](
    vec3(-1.0, -1.0, 0.0),
    vec3( 1.0, -1.0, 0.0),
    vec3( 1.0,  1.0, 0.0),
    vec3(-1.0,  1.0, 0.0)
);

const vec3 norms[] = vec3[](
    vec3(0.0, 0.0, 1.0),
    vec3(0.0, 0.0, 1.0),
    vec3(0.0, 0.0, 1.0),
    vec3(0.0, 0.0, 1.0)
);

const vec2 uvs[] = vec2[](
    vec2(0.0, 0.0),
    vec2(1.0, 0.0),
    vec2(1.0, 1.0),
    vec2(0.0, 1.0)
);

layout(location = 0) out vec3 vert_f;
layout(location = 1) out vec3 norm_f;
layout(location = 2) out vec2 uv_f;

void main(){
	vert_f = verts[gl_VertexID];
	norm_f = norms[gl_VertexID];
	uv_f   = uvs[gl_VertexID];

	gl_Position = vec4(verts[gl_VertexID], 1.0);
}
