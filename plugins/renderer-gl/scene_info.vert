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

layout(location = 0) in vec3 vert;
layout(location = 1) in vec3 norm;
layout(location = 2) in vec2 uv;

layout(location = 0) out vec3 vert_f;
layout(location = 1) out vec3 norm_f;
layout(location = 2) out vec2 uv_f;

void main(){
	vert_f = vert;
	norm_f = norm;
	uv_f   = uv;

	// TODO: use globals.view_proj
	gl_Position = vec4(vert, 1.0);
}
