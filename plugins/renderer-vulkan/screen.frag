#version 450

/*
 * Ham Renderer Vulkan Shaders
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

layout(binding = 0) uniform FragmentGlobalData{
	float time;
} globals;

layout(location = 0) in vec3 vert_f;
layout(location = 1) in vec3 norm_f;
layout(location = 2) in vec2 uv_f;

layout(location = 0) out vec4 out_color;

void main(){
	out_color = vec4(uv_f, 1.0, 1.0);
}
