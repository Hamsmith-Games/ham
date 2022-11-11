/*
 * Ham Runtime Tests
 * Copyright (C) 2022 Keith Hammond
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

	constexpr auto check_true = +[](bool result){ return result; };

	ham::test_bool<> tests[] = {
		{"meta",       ham_test_meta,      check_true},
		{"object",     ham_test_object,    check_true},
		{"utf",        ham_test_utf,       check_true},
		{"lex",        ham_test_lex,       check_true},
		{"buffer",     ham_test_buffer,    check_true},
		{"octree",     ham_test_octree,    check_true},
		{"mat",        ham_test_mat,       check_true},
		{"quaternion", ham_test_quat,      check_true},
		{"transform",  ham_test_transform, check_true},
		{"camera",     ham_test_camera,    check_true},
	};

	constexpr usize num_tests = std::size(tests);

	usize passed_tests = 0;

	for(usize i = 0; i < std::size(tests); i++){
		const auto &test = tests[i];
		if(test.run()) ++passed_tests;
	}

	std::cout << passed_tests << "/" << num_tests << " tests passed\n";

	return (passed_tests == num_tests) ? 0 : 1;
}
