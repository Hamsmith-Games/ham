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

#include "ham/meta.hpp"
#include "ham/str_buffer.h"

#include "tests.hpp"

#include <iostream>

using namespace ham::typedefs;

bool ham_test_meta(){
	static_assert(ham::meta::type_name_v<void> == "void");
	static_assert(ham::meta::type_name_v<bool> == "bool");

	static_assert(ham::meta::type_name_v<char8>  == "char8");
	static_assert(ham::meta::type_name_v<char16> == "char16");
	static_assert(ham::meta::type_name_v<char32> == "char32");

	static_assert(ham::meta::type_name_v<i8>  == "i8");
	static_assert(ham::meta::type_name_v<u8>  == "u8");
	static_assert(ham::meta::type_name_v<i16> == "i16");
	static_assert(ham::meta::type_name_v<u16> == "u16");
	static_assert(ham::meta::type_name_v<i32> == "i32");
	static_assert(ham::meta::type_name_v<u32> == "u32");
	static_assert(ham::meta::type_name_v<i64> == "i64");
	static_assert(ham::meta::type_name_v<u64> == "u64");
#ifdef HAM_INT128
	static_assert(ham::meta::type_name_v<i128> == "i128");
	static_assert(ham::meta::type_name_v<u128> == "u128");
#endif

#ifdef HAM_FLOAT16
	static_assert(ham::type_name_v<f16> == "f16");
#endif

	static_assert(ham::meta::type_name_v<f32> == "f32");
	static_assert(ham::meta::type_name_v<f64> == "f64");

#ifdef HAM_FLOAT128
	static_assert(ham::meta::type_name_v<f128> == "f128");
#endif

	static_assert(ham::meta::type_name_v<ham_str8>  == "ham_str8");
	static_assert(ham::meta::type_name_v<ham_str16> == "ham_str16");
	static_assert(ham::meta::type_name_v<ham_str32> == "ham_str32");

	static_assert(ham::meta::type_name_v<ham_uuid> == "ham_uuid");

	static_assert(ham::meta::type_name_v<ham::basic_str<char8>>  == "ham::basic_str<char>");
	static_assert(ham::meta::type_name_v<ham::basic_str<char16>> == "ham::basic_str<char16_t>");
	static_assert(ham::meta::type_name_v<ham::basic_str<char32>> == "ham::basic_str<char32_t>");

	static_assert(ham::meta::type_name_v<ham::uuid> == "ham::uuid");

	static_assert(ham::meta::type_name_v<ham::basic_str_buffer<char8>> == "ham::basic_str_buffer<char>");
	static_assert(ham::meta::type_name_v<ham::basic_str_buffer<char16>> == "ham::basic_str_buffer<char16_t>");
	static_assert(ham::meta::type_name_v<ham::basic_str_buffer<char32>> == "ham::basic_str_buffer<char32_t>");

	return true;
}
