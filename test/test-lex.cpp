#include "ham/lex.h"

#include "tests.hpp"

#include <iostream>

constexpr auto test_lex_src = HAM_LIT_UTF8(
	"hello \"world\" '123' 456 7.89\n"
	"my name is ham"
);

bool ham_test_lex(){
	std::cout << "Running lexer tests... ";
	std::cout << std::flush;

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

		std::cout << "FAILED\n" << std::flush;

		std::cerr
			<< "    Lexing error[" << tok_loc.line() + 1 << ":" << tok_loc.column()+1 << "]: " << tok_str << '\n';

		return false;
	}

	std::cout << "DONE\n";
	return true;
}
