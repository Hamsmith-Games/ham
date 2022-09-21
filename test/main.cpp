/*
 * Ham Programming Language Tests
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
