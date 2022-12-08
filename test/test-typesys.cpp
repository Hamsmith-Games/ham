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

#include "ham/typesys.h"

#include "tests.hpp"

using namespace ham::typedefs;

struct ham_typesys_test_type{
	bool b;
	f32 f;
	ham_vec3 v3;
};

bool ham_test_typesys(){
	ham::typeset ts;

	{
		ham::type_builder builder;
		ham_test_assert(builder.set_kind(HAM_TYPE_OBJECT));
		ham_test_assert(builder.set_name(ham::meta::type_name_v<ham_typesys_test_type>));

		const auto bool_ty = ham::get_type<bool>(ts);
		const auto f32_ty  = ham::get_type<f32>(ts);
		const auto v3_ty   = ham::get_type<ham_vec3>(ts);

		ham_test_assert(builder.add_member("b",  bool_ty));
		ham_test_assert(builder.add_member("f",  f32_ty));
		ham_test_assert(builder.add_member("v3", v3_ty));

		const auto test_ty = builder.instantiate(ts);

		ham_test_assert(test_ty);

		ham_test_assert(ham_type_size(test_ty) >= ham_type_alignment(test_ty));

		ham_test_assert(ham_type_alignment(test_ty) == alignof(ham_typesys_test_type));
		ham_test_assert(ham_type_size(test_ty) == sizeof(ham_typesys_test_type));
	}

	return true;
}
