#include "ham.h"

#include <iostream>

using namespace ham::typedefs;

constexpr auto test_lex_src = HAM_LIT_UTF8(
	"hello \"world\" '123' 456 7.89\n"
	"my name is ham"
);

static inline bool ham_test_utf(){
	std::cout << "Running unicode tests...\n";

	constexpr char8  u8str[]  =  "abcdefghijklmnopqrstuvwxyz";
	constexpr char32 u32str[] = U"abcdefghijklmnopqrstuvwxyz";

	utf_cp cp;

	usize off = 0;
	while(off < (sizeof(u8str)-1)){
		const auto u32idx = off;

		const auto nchars = ham_str_next_codepoint_utf8(&cp, u8str + off, sizeof(u8str) - (off+1));
		if(nchars == (usize)-1 || u32str[u32idx] != cp){
			std::cerr << "Test failed on utf character '" << u8str[u32idx] << "'\n";
			return false;
		}

		off += nchars;
	}

	return true;
}

int main(int argc, char *argv[]){
	assert(ham_test_utf());

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
