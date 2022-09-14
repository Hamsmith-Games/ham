#include "ham.h"

#include <iostream>

using namespace ham::typedefs;

const ham_str test_lex_src = HAM_LIT(
	"hello \"world\" '123' 456 7.89\n"
	"my name is ham"
);

int main(int argc, char *argv[]){
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

		std::cout
			<< "Lexed token: (kind: " << ham::token_kind_str(tok.kind()) << ") " << tok_str << '\n';
	}

	if(tok.is_error()){
		const auto tok_str = tok.str();
		const auto tok_loc = tok.source_location();

		std::cerr
			<< "Lexing error[" << tok_loc.line() + 1 << ":" << tok_loc.column()+1 << "]: " << tok_str << '\n';

		return 1;
	}

	return 0;
}
