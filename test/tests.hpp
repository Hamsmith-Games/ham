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

#ifndef HAM_TEST_TESTS_HPP
#define HAM_TEST_TESTS_HPP 1

#include <stdio.h>

#define HAM_TEST_PASSED_STR "PASSED"
#define HAM_TEST_FAILED_STR "FAILED"

#define ham_test_cond(cond_, err_fmt_, ...) \
	((cond_) ? true : ( \
		(fprintf(stdout, HAM_TEST_FAILED_STR "\n"), 0), \
		(fprintf(stderr, "    Condition not met: %s\n    " err_fmt_, #cond_ __VA_OPT__(,) __VA_ARGS__), 0), \
		false))

bool ham_test_meta();
bool ham_test_utf();
bool ham_test_lex();
bool ham_test_parse();
bool ham_test_object();

#endif // !HAM_TEST_TESTS_HPP
