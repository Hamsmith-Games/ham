#include "ham/parse.h"

#include "tests.hpp"

#include <iostream>

using namespace ham::typedefs;

constexpr auto test_parse_src = HAM_LIT_UTF8(
	"x = 1\n"
	"y = 2\n"
	//"f(a) = a + 1\n"
	//"g(b) = (f b) * 2\n"
);

constexpr ham::expr_kind test_parse_expected_kinds[] = {
	ham::expr_kind::binary_op,
	ham::expr_kind::binary_op
};

bool ham_test_parse(){
	std::cout << "Running parser tests... ";
	std::cout << std::flush;

	ham::source_location_utf8 source_loc("parser-test.ham", 0, 0);

	const auto lexed = ham::lex_all(source_loc, ham::str8(test_parse_src));
	if(lexed.empty()){
		std::cout << "FAILED\n" << std::flush;
		std::cerr << "    Failed to lex test source.\n";
		return false;
	}

	for(auto &&tok : lexed){
		if(tok.is_error()){
			const auto tok_loc = tok.source_location();
			std::cout << "FAILED\n" << std::flush;
			std::cerr << "    Lexing error[" << tok_loc.line()+1 << ":" << tok_loc.column()+1 << "]: " << tok.str() << '\n';
			return false;
		}
	}


	ham::token_range_utf8 tokens(&*lexed.begin(), &*lexed.end());

	ham::parse_context_utf8 ctx;
	ham::expr_base_utf8 expr;

	const usize expected_num_exprs = std::size(test_parse_expected_kinds);

	usize i = 0;

	while((expr = ham::parse(ctx, tokens))){
		const auto expr_tokens = expr.tokens();

		if(expr.is_error()){
			const auto err_expr = (const ham_expr_error_utf8*)expr.handle();

			ham::source_location_utf8 src_loc;
			if(expr_tokens.begin() != expr_tokens.end()){
				src_loc = expr_tokens.begin()->source_location();
			}

			std::cout << "FAILED\n" << std::flush;
			std::cerr << "    Parsing error[" << src_loc.line()+1 << ":" << src_loc.column()+1 << "]: " << err_expr->message << '\n';
			return false;
		}
		else if(expr.kind() != test_parse_expected_kinds[i]){
			std::cout << "FAILED\n" << std::flush;
			std::cerr << "    Parsing error: expected kind " << ham::expr_kind_str8(test_parse_expected_kinds[i]) << " got " << ham::expr_kind_str8(expr.kind()) << '\n';
			return false;
		}

		++i;

		tokens = ham::token_range_utf8{ expr_tokens.end(), tokens.end() };
		if(tokens.begin() == tokens.end()){
			break;
		}
	}

	if(i != expected_num_exprs){
		std::cout << "FAILED\n" << std::flush;
		std::cerr << "    Parsing error: wrong number of expressions, expected " << expected_num_exprs << " got " << i << '\n';
		return false;
	}

	std::cout << "DONE\n";
	return true;
}
