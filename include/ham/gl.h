/*
 * Ham Runtime
 * Copyright (C) 2022  Hamsmith Ltd.
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

#ifndef HAM_GL_H
#define HAM_GL_H 1

/**
 * @defgroup HAM_GL OpenGL
 * @ingroup HAM
 * @{
 */

#include "typedefs.h"

HAM_C_API_BEGIN

typedef struct ham_gl_draw_elements_indirect_command{
	ham_u32 index_count;
	ham_u32 instance_count;
	ham_u32 first_index;
	ham_i32 base_vertex;
	ham_u32 base_instance;
} ham_gl_draw_elements_indirect_command;

HAM_C_API_END

/**
 * @}
 */

#endif // !HAM_GL_H
