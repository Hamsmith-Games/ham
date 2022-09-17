#include "ham/lex.h"
#include "ham/str_buffer.h"
#include "ham/meta.hpp"

#include <iostream>

using namespace ham::typedefs;

constexpr auto test_lex_src = HAM_LIT_UTF8(
	"hello \"world\" '123' 456 7.89\n"
	"my name is ham"
);

static inline bool ham_test_meta(){
	std::cout << "Running meta tests... " << std::flush;

	static_assert(ham::type_name_v<void> == "void");
	static_assert(ham::type_name_v<bool> == "bool");

	static_assert(ham::type_name_v<char8>  == "char8");
	static_assert(ham::type_name_v<char16> == "char16");
	static_assert(ham::type_name_v<char32> == "char32");

	static_assert(ham::type_name_v<i8>  == "i8");
	static_assert(ham::type_name_v<u8>  == "u8");
	static_assert(ham::type_name_v<i16> == "i16");
	static_assert(ham::type_name_v<u16> == "u16");
	static_assert(ham::type_name_v<i32> == "i32");
	static_assert(ham::type_name_v<u32> == "u32");
	static_assert(ham::type_name_v<i64> == "i64");
	static_assert(ham::type_name_v<u64> == "u64");
#ifdef HAM_INT128
	static_assert(ham::type_name_v<i128> == "i128");
	static_assert(ham::type_name_v<u128> == "u128");
#endif

#ifdef HAM_FLOAT16
	static_assert(ham::type_name_v<f16> == "f16");
#endif

	static_assert(ham::type_name_v<f32> == "f32");
	static_assert(ham::type_name_v<f64> == "f64");

#ifdef HAM_FLOAT128
	static_assert(ham::type_name_v<f128> == "f128");
#endif

	static_assert(ham::type_name_v<ham_str8>  == "str8");
	static_assert(ham::type_name_v<ham_str16> == "str16");
	static_assert(ham::type_name_v<ham_str32> == "str32");

	static_assert(ham::type_name_v<ham_uuid> == "uuid");

	static_assert(ham::type_name_v<ham::basic_str<char8>>  == "ham::basic_str<char>");
	static_assert(ham::type_name_v<ham::basic_str<char16>> == "ham::basic_str<char16_t>");
	static_assert(ham::type_name_v<ham::basic_str<char32>> == "ham::basic_str<char32_t>");

	static_assert(ham::type_name_v<ham::uuid> == "ham::uuid");

	static_assert(ham::type_name_v<ham::basic_str_buffer<char8>> == "ham::basic_str_buffer<char>");
	static_assert(ham::type_name_v<ham::basic_str_buffer<char16>> == "ham::basic_str_buffer<char16_t>");
	static_assert(ham::type_name_v<ham::basic_str_buffer<char32>> == "ham::basic_str_buffer<char32_t>");

	std::cout << "DONE\n";
	return true;
}

static inline bool ham_test_utf(){
	std::cout << "Running unicode tests... " << std::flush;

	constexpr char8  u8str[]  =  "abcdefghijklmnopqrstuvwxyz";
	constexpr char32 u32str[] = U"abcdefghijklmnopqrstuvwxyz";

	utf_cp cp;

	usize off = 0;
	while(off < (sizeof(u8str)-1)){
		const auto u32idx = off;

		const auto nchars = ham_str_next_codepoint_utf8(&cp, u8str + off, sizeof(u8str) - (off+1));
		if(nchars == (usize)-1 || u32str[u32idx] != cp){
			std::cout << "FAILED\n" << std::flush;

			std::cerr << "Test failed on utf character '" << u8str[u32idx] << "'\n";
			return false;
		}

		off += nchars;
	}

	std::cout << "DONE\n";
	return true;
}

static inline bool ham_test_lex(){
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
			<< "Lexing error[" << tok_loc.line() + 1 << ":" << tok_loc.column()+1 << "]: " << tok_str << '\n';

		return false;
	}

	std::cout << "DONE\n";
	return true;
}

static inline bool ham_test_parse(){
	std::cout << "Running parser tests... ";
	std::cout << std::flush;

	std::cout << "DONE\n";
	return true;
}

int main(int argc, char *argv[]){
	(void)argc; (void)argv;

	usize passed_tests = 0;
	usize num_tests = 0;

	const auto run_test = [&](auto f){
		++num_tests;
		if(f()) ++passed_tests;
	};

	run_test(ham_test_meta);
	run_test(ham_test_utf);
	run_test(ham_test_lex);
	run_test(ham_test_parse);

	std::cout << passed_tests << "/" << num_tests << " tests passed\n";

	return (passed_tests == num_tests) ? 0 : 1;
}
