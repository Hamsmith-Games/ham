/*
 * Ham Runtime Plugins
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

#ifndef HAM_RENDERER_GL_RENDERER_GL_HPP
#define HAM_RENDERER_GL_RENDERER_GL_HPP 1

#include "ham/renderer-object.h"

#include "KHR/khrplatform.h"

#ifdef HAM_RENDERER_GL_IMPLEMENTATION
#	define ham_renderer_gl_api ham_private ham_export
#else
#	define ham_renderer_gl_api ham_private ham_import
#endif

HAM_C_API_BEGIN

typedef enum ham_renderer_gl_fbo_attachment{
	HAM_RENDERER_GL_FBO_DEPTH_STENCIL,
	HAM_RENDERER_GL_FBO_DIFFUSE,
	HAM_RENDERER_GL_FBO_NORMAL,
	HAM_RENDERER_GL_FBO_SCENE,

	HAM_RENDERER_GL_FBO_ATTACHMENT_COUNT
} ham_renderer_gl_fbo_attachment;

typedef struct ham_renderer_gl{
	ham_derive(ham_renderer)

	khronos_uint32_t
		fbo,
		fbo_attachments[HAM_RENDERER_GL_FBO_ATTACHMENT_COUNT]
	;
} ham_renderer_gl;

typedef struct ham_draw_group_gl{
	ham_derive(ham_draw_group)

	khronos_uint32_t vao, vbo, ibo, cbo;
} ham_draw_group_gl;

HAM_C_API_END

#endif // !HAM_RENDERER_GL_RENDERER_GL_HPP
