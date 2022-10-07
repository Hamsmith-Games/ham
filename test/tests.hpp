/*
 * Ham Runtime Tests
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

bool ham_test_meta();
bool ham_test_utf();
bool ham_test_lex();
bool ham_test_parse();
bool ham_test_object();
bool ham_test_buffer();
bool ham_test_octree();

#include "ham/str_buffer.h"
#include "ham/functional.h"

#include "fmt/color.h"

#include <sstream>
#include <iostream>

namespace ham{
	template<typename Sig>
	class basic_test;

	template<typename Result, typename ... Args>
	class basic_test<Result(Args...)>{
		public:
			basic_test(
				str_buffer_utf8 name_,
				indirect_function<Result(Args...)> fn_,
				indirect_function<bool(Result)> check_fn_
			)
				: m_name(std::move(name_))
				, m_fn(std::move(fn_))
				, m_check_fn(std::move(check_fn_))
			{}

			template<typename ... UArgs>
			bool run(UArgs &&... args) const{
				fmt::print("Test: {:<8} ", m_name);
				std::fflush(stdout);

				std::stringstream cerr_buf;
				std::streambuf *cerr_old_buf = std::cerr.rdbuf(cerr_buf.rdbuf());

				Result res = m_fn(std::forward<UArgs>(args)...);
				const auto result = m_check_fn(res);
				if(result){
					fmt::print(fmt::emphasis::bold | fg(fmt::color::green), "PASSED\n");
				}
				else{
					fmt::print(fmt::emphasis::bold | fg(fmt::color::red), "FAILED\n");
				}

				std::cerr.rdbuf(cerr_old_buf);

				if(cerr_buf.tellp() != std::streampos(0)){
					std::cerr << cerr_buf.str() << '\n';
				}

				return result;
			}

		private:
			str_buffer_utf8 m_name;
			indirect_function<Result(Args...)> m_fn;
			indirect_function<bool(Result)> m_check_fn;
	};

	template<typename ... Args>
	using test_bool = basic_test<bool(Args...)>;
}

#endif // !HAM_TEST_TESTS_HPP
