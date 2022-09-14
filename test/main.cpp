#include "ham.h"

#include <stdio.h>

using namespace ham::typedefs;

const ham_str test_src = HAM_LIT(
	"hello [world] {my} name == ham\n"
	"my name is ham"
);

int main(int argc, char *argv[]){
	ham_source_location loc = {
		HAM_LIT("test-src"),
		0, 0
	};

	ham_str src_rem = test_src;
	ham_token tok;

	while(ham_lex(&loc, src_rem, &tok)){
		if(tok.kind == HAM_TOKEN_EOF){
			break;
		}

		src_rem = (ham_str){ src_rem.ptr + tok.str.len, src_rem.len - tok.str.len };

		printf("Lexed token: (%s) %.*s\n", ham_token_kind_str(tok.kind), (int)tok.str.len, tok.str.ptr);
	}

	if(tok.kind == HAM_TOKEN_ERROR){
		fprintf(stderr, "Lexing error[%zu:%zu]: %.*s\n", tok.loc.line+1, tok.loc.col+1, (int)tok.str.len, tok.str.ptr);
		return 1;
	}

	return 0;
}
