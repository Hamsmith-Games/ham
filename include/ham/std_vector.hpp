/*
 * Ham Runtime
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

#ifndef HAM_STD_VECTOR_HPP
#define HAM_STD_VECTOR_HPP 1

/**
 * @defgroup HAM_STD_VECTOR std::vector wrapper
 * @ingroup HAM
 * @{
 */

#include "memory.h"

#include <vector>

namespace ham{
	template<typename T, typename Allocator = allocator<T>>
	using std_vector = std::vector<T, Allocator>;
}

/**
 * @}
 */

#endif // !HAM_STD_VECTOR_HPP
