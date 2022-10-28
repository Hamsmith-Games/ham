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

#include "ham/check.h"
#include "ham/math.h"

#define ham_impl_eat(...) __VA_ARGS__

#define ham_declare_test(name) \
	bool ham_test_##name();

#define ham_define_test(name, body) \
	bool ham_test_##name(){ ham_impl_eat body }

#define ham_test_assert_msg(cond, fmt_str, ...) \
	if(!ham_check_msg(cond, fmt_str __VA_OPT__(,) __VA_ARGS__)) return false

#define ham_test_assert(cond) ham_test_assert_msg((cond), "Test condition failed: %s", #cond)

ham_declare_test(meta)
ham_declare_test(utf)
ham_declare_test(lex)
ham_declare_test(parse)
ham_declare_test(object)
ham_declare_test(buffer)
ham_declare_test(octree)
ham_declare_test(mat)
ham_declare_test(quat)
ham_declare_test(transform)
ham_declare_test(camera)

#define GLM_FORCE_RADIANS
#define GLM_FORCE_LEFT_HANDED
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_QUAT_DATA_XYZW
#include "glm/gtc/type_ptr.hpp"

static inline bool operator==(const glm::mat4 &a, const ham::mat4 &b) noexcept{
	return std::memcmp(glm::value_ptr(a), b.data(), sizeof(ham_f32) * 16) == 0;
}

static inline bool operator==(const ham::mat4 &a, const glm::mat4 &b) noexcept{
	return std::memcmp(a.data(), glm::value_ptr(b), sizeof(ham_f32) * 16) == 0;
}

static inline bool operator!=(const glm::mat4 &a, const ham::mat4 &b) noexcept{
	return std::memcmp(glm::value_ptr(a), b.data(), sizeof(ham_f32) * 16) != 0;
}

static inline bool operator!=(const ham::mat4 &a, const glm::mat4 &b) noexcept{
	return std::memcmp(a.data(), glm::value_ptr(b), sizeof(ham_f32) * 16) != 0;
}

#define DEFAULT_EPSILON 0.000125f

static inline bool roughly_equal(float a, float b, float epsilon = DEFAULT_EPSILON) noexcept{
	return std::abs(a - b) <= epsilon;
}

static inline int roughly_equal(const ham::quat &a, const glm::quat &b, float epsilon = DEFAULT_EPSILON) noexcept{
	for(int i = 0; i < 4; i++){
		if(!roughly_equal(a.data()[i], glm::value_ptr(b)[i], epsilon)){
			return i + 1;
		}
	}

	return 0;
}

static inline int roughly_equal(const ham::mat4 &a, const ham::mat4 &b, float epsilon = DEFAULT_EPSILON) noexcept{
	for(int i = 0; i < 4; i++){
		for(int j = 0; j < 4; j++){
			const auto idx = (i * 4) + j;
			if(!roughly_equal(a.data()[idx], b.data()[idx], epsilon)){
				return idx + 1;
			}
		}
	}

	return 0;
}

static inline int roughly_equal(const ham::mat4 &a, const glm::mat4 &b, float epsilon = DEFAULT_EPSILON) noexcept{
	for(int i = 0; i < 4; i++){
		for(int j = 0; j < 4; j++){
			const auto idx = (i * 4) + j;
			if(!roughly_equal(a.data()[idx], glm::value_ptr(b)[idx], epsilon)){
				return idx + 1;
			}
		}
	}

	return 0;
}

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
				fmt::print("Test: {:<12} ", m_name);
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
