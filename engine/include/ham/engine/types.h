/*
 * Ham World Engine Runtime
 * Copyright (C) 2022 Keith Hammond
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef HAM_ENGINE_TYPES_H
#define HAM_ENGINE_TYPES_H 1

#include "ham/engine/config.h"
#include "ham/typesys.h"

HAM_C_API_BEGIN

ham_engine_api bool ham_engine_ensure_types(ham_typeset *ts);

HAM_C_API_END

#ifdef __cplusplus

namespace ham::engine{
	ham_used
	static inline bool ensure_types(typeset_view ts){
		return ham_engine_ensure_types(ts);
	}
}

#endif // __cplusplus

#endif // !HAM_ENGINE_TYPES_H
