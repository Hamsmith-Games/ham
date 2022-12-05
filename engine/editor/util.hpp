/*
 * Ham World Engine Editor
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

#ifndef HAM_ENGINE_EDITOR_UTIL_HPP
#define HAM_ENGINE_EDITOR_UTIL_HPP 1

#include <QString>

#include "ham/typedefs.h"

namespace ham::engine::editor{
	ham_used
	static inline QString to_QString(str8 str){
		return QString::fromUtf8(str.data(), str.len());
	}
}

#endif // !HAM_ENGINE_EDITOR_UTIL_HPP
