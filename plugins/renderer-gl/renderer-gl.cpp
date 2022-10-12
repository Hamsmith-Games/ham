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

#include "renderer-gl.hpp"

#include "ham/log.h"

#include "glbinding/glbinding.h"
#include "glbinding/gl46core/gl.h"

HAM_C_API_BEGIN

static inline ham_renderer_gl *ham_renderer_gl_ctor(ham_renderer_gl *r, ham_u32 nargs, va_list va){
	if(nargs != 1){
		ham::logapierror("Wrong number of arguments passed: {}, expected 1 (const ham_renderer_create_args*)", nargs);
		return nullptr;
	}

	const ham_renderer_create_args *create_args = va_arg(va, const ham_renderer_create_args*);

	const auto getProcAddr = create_args->gl.glGetProcAddr;

	glbinding::initialize((glbinding::ContextHandle)create_args->gl.context_handle, [getProcAddr](const char *name){ return (void(*)())getProcAddr(name); });

	const auto ret = new(r) ham_renderer_gl;

	using namespace gl;

	glCreateFramebuffers(1, &ret->fbo);
	glCreateTextures(GL_TEXTURE_2D, (GLsizei)std::size(ret->fbo_attachments), ret->fbo_attachments);

	return ret;
}

static inline void ham_renderer_gl_dtor(ham_renderer_gl *r){
	using namespace gl;

	glDeleteFramebuffers(1, &r->fbo);
	glDeleteTextures((GLsizei)std::size(r->fbo_attachments), r->fbo_attachments);

	std::destroy_at(r);
}

static inline void ham_renderer_gl_frame(ham_renderer_gl *r, ham_f64 dt, const ham_renderer_frame_data *data){

}

ham_define_renderer(ham_renderer_gl, ham_draw_group_gl)

HAM_C_API_END
