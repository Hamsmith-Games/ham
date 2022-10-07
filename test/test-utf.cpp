/*
 * Ham Runtime Tests
 * Copyright (C) 2022  Hamsmith Ltd.
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

#include "ham/typedefs.h"

#include "tests.hpp"

#include <iostream>

using namespace ham::typedefs;

#define HAM_TEST_UTF_STR "🍖abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789?\\\n\r"

bool ham_test_utf(){
	constexpr char8  u8str[]  = HAM_TEST_UTF_STR;
	constexpr char32 u32str[] = HAM_CONCAT(U, HAM_TEST_UTF_STR);

	utf_cp cp;

	usize off = 0;
	usize u32idx = 0;
	while(off < (sizeof(u8str)-1)){
		const auto nchars = ham_str_next_codepoint_utf8(&cp, u8str + off, sizeof(u8str) - (off+1));
		if(nchars == (usize)-1 || u32str[u32idx] != cp){
			std::cerr << "Test failed at character " << off << " '" << u8str[u32idx] << "'\n" << std::flush;
			return false;
		}

		off += nchars;
		++u32idx;
	}

	return true;
}
