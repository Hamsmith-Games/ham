/*
 * Ham Runtime Plugins
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

#include "physics_bullet3.h"

HAM_C_API_BEGIN

ham_def_ctor(ham_physics_body_bullet3, nargs, va){
	(void)nargs; (void)va;
	return self;
}

ham_def_dtor(ham_physics_body_bullet3){
	(void)self;
}

ham_def_physics_body_object(ham_physics_body_bullet3)

HAM_C_API_END
