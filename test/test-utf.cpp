#include "ham/typedefs.h"

#include "tests.hpp"

#include <iostream>

using namespace ham::typedefs;

#define HAM_TEST_UTF_STR "🍖abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789?\\\n\r"

bool ham_test_utf(){
	std::cout << "Running unicode tests... " << std::flush;

	constexpr char8  u8str[]  = HAM_TEST_UTF_STR;
	constexpr char32 u32str[] = HAM_CONCAT(U, HAM_TEST_UTF_STR);

	utf_cp cp;

	usize off = 0;
	usize u32idx = 0;
	while(off < (sizeof(u8str)-1)){
		const auto nchars = ham_str_next_codepoint_utf8(&cp, u8str + off, sizeof(u8str) - (off+1));
		if(nchars == (usize)-1 || u32str[u32idx] != cp){
			std::cout << "FAILED\n" << std::flush;

			std::cerr << "    Test failed at character " << off << " '" << u8str[u32idx] << "'\n" << std::flush;
			return false;
		}

		off += nchars;
		++u32idx;
	}

	std::cout << "DONE\n";
	return true;
}
