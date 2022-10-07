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

#include "ham/lex.h"

#include "tests.hpp"

#include <iostream>

constexpr auto test_lex_src = HAM_LIT_UTF8(
	"hello \"world\" '123' 456 7.89\n"
	"my name is ham"
);

bool ham_test_lex(){
	ham::source_location loc(
		HAM_LIT("test-src"),
		0, 0
	);

	ham::str src_rem = test_lex_src;
	ham::token tok;

	while(ham::lex(loc, src_rem, tok)){
		if(tok.is_eof()){
			break;
		}

		const auto tok_str = tok.str();
		src_rem = ham::str(src_rem.ptr() + tok_str.len(), src_rem.len() - tok_str.len());
	}

	if(tok.is_error()){
		const auto tok_str = tok.str();
		const auto tok_loc = tok.source_location();

		std::cerr << "Lexing error[" << tok_loc.line() + 1 << ":" << tok_loc.column()+1 << "]: " << tok_str << '\n';

		return false;
	}

	return true;
}
