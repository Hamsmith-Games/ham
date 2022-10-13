/*
 * Ham Runtime
 * Copyright (C) 2022 Hamsmith Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "ham/typesys.h"
#include "ham/log.h"
#include "ham/memory.h"
#include "ham/colony.h"

using namespace ham::typedefs;

HAM_C_API_BEGIN

struct ham_type{
	const ham_typeset *ts;
	const char *name;
	u32 flags;
	usize alignment, size;
	const ham_type *nodes;
};

struct ham_typeset{
	const ham_allocator *allocator;
	mutable ham::colony<ham_type> types;

	const ham_type *void_type;
	const ham_type *unit_type;
	const ham_type *top_type;
	const ham_type *bottom_type;
	const ham_type *bool_type;

	// u8, u16, u32, u64, u128
	const ham_type *nat_types[5];

	// i8, i16, i32, i64, i128
	const ham_type *int_types[5];

	// r8, r16, r32, r64, r128, r256
	const ham_type *rat_types[6];

	// f16, f32, f64 and f128
	const ham_type *float_types[4];

	// str8, str16, str32
	const ham_type *str_types[3];

	// vec2, vec3, vec4
	const ham_type *vec_types[3];
};

ham_nonnull_args(1)
static inline const ham_type *ham_impl_typeset_new_type(
	ham_typeset *ts,
	const char *name,
	ham_type_kind_flag kind,
	ham_type_info_flag info,
	usize alignment,
	usize size
){
	const auto new_type = ts->types.emplace();
	new_type->ts = ts;
	new_type->name = name;
	new_type->flags = ham_make_type_flags(kind, info);
	new_type->alignment = alignment;
	new_type->size = size;
	return new_type;
}

ham_typeset *ham_typeset_create(){
	const auto allocator = ham_current_allocator();

	const auto ptr = ham_allocator_new(allocator, ham_typeset);
	if(!ptr) return nullptr;

	ptr->allocator = allocator;

	ptr->void_type   = ham_impl_typeset_new_type(ptr, "void",       HAM_TYPE_THEORETIC, HAM_TYPE_INFO_THEORETIC_VOID,    0, 0);
	ptr->unit_type   = ham_impl_typeset_new_type(ptr, "ham_unit",   HAM_TYPE_THEORETIC, HAM_TYPE_INFO_THEORETIC_UNIT,    1, 1);
	ptr->top_type    = ham_impl_typeset_new_type(ptr, "HAM_TOP",    HAM_TYPE_THEORETIC, HAM_TYPE_INFO_THEORETIC_TOP,     0, 0);
	ptr->bottom_type = ham_impl_typeset_new_type(ptr, "HAM_BOTTOM", HAM_TYPE_THEORETIC, HAM_TYPE_INFO_THEORETIC_BOTTOM,  0, 0);
	ptr->bool_type   = ham_impl_typeset_new_type(ptr, "bool",       HAM_TYPE_NUMERIC,   HAM_TYPE_INFO_NUMERIC_BOOLEAN,   alignof(bool), sizeof(bool));

	constexpr const char *nat_names[] = { "u8", "u16", "u32", "u64", "u128" };

	// nat types
	for(usize i = 0; i < std::size(ptr->nat_types); i++){
		const auto size = 1UL << i;
		ptr->nat_types[i] = ham_impl_typeset_new_type(ptr, nat_names[i], HAM_TYPE_NUMERIC, HAM_TYPE_INFO_NUMERIC_NATURAL, size, size);
	}

	constexpr const char *int_names[] = { "i8", "i16", "i32", "i64", "i128" };

	// int types
	for(usize i = 0; i < std::size(ptr->int_types); i++){
		const auto size = 1UL << i;
		ptr->nat_types[i] = ham_impl_typeset_new_type(ptr, int_names[i], HAM_TYPE_NUMERIC, HAM_TYPE_INFO_NUMERIC_INTEGER, size, size);
	}

	constexpr const char *rat_names[] = { "ham_rat8", "ham_rat16", "ham_rat32", "ham_rat64", "ham_rat128", "ham_rat256" };

	// rat types
	for(usize i = 0; i < std::size(ptr->rat_types); i++){
		const auto size = 1UL << i;
		ptr->nat_types[i] = ham_impl_typeset_new_type(ptr, rat_names[i], HAM_TYPE_NUMERIC, HAM_TYPE_INFO_NUMERIC_RATIONAL, size, size);
	}

	constexpr const char *float_names[] = { "f16", "f32", "f64", "f128" };

	// float types
	for(usize i = 0; i < std::size(ptr->float_types); i++){
		const auto size = 2UL << i;
		ptr->float_types[i] = ham_impl_typeset_new_type(ptr, float_names[i], HAM_TYPE_NUMERIC, HAM_TYPE_INFO_NUMERIC_FLOATING_POINT, size, size);
	}

	constexpr const char *str_names[] = {
		"ham_str8", "ham_str16", "ham_str32"
	};

	// string types
	for(int i = 0; i < 3; i++){
		constexpr auto size = sizeof(uptr) * 2;
		constexpr auto alignment = alignof(uptr);
		ptr->str_types[i] = ham_impl_typeset_new_type(ptr, str_names[i], HAM_TYPE_STRING, (ham_type_info_flag)(HAM_TYPE_INFO_STRING_UTF8 + i), alignment, size);
	}

	constexpr usize vec_sizes[] = {
		sizeof(ham_vec2),
		sizeof(ham_vec3),
		sizeof(ham_vec4),
	};

	constexpr usize vec_aligns[] = {
		alignof(ham_vec2),
		alignof(ham_vec3),
		alignof(ham_vec4),
	};

	constexpr const char *vec_names[] = {
		"ham_vec2", "ham_vec3", "ham_vec4"
	};

	for(usize i = 0; i < std::size(vec_sizes); i++){
		ptr->vec_types[i] = ham_impl_typeset_new_type(ptr, vec_names[i], HAM_TYPE_NUMERIC, HAM_TYPE_INFO_NUMERIC_VECTOR, vec_aligns[i], vec_sizes[i]);
	}

	return ptr;
}

void ham_typeset_destroy(ham_typeset *ts){
	if(ham_unlikely(ts == NULL)) return;

	ham_allocator_delete(ts->allocator, ts);
}

const ham_type *ham_typeset_void(const ham_typeset *ts){ return ts->void_type; }
const ham_type *ham_typeset_unit(const ham_typeset *ts){ return ts->unit_type; }
const ham_type *ham_typeset_top(const ham_typeset *ts){ return ts->top_type; }
const ham_type *ham_typeset_bottom(const ham_typeset *ts){ return ts->bottom_type; }

const ham_type *ham_typeset_bool(const ham_typeset *ts){ return ts->bool_type; }

const ham_type *ham_typeset_nat(const ham_typeset *ts, ham_usize num_bits){
	switch(num_bits){
		case CHAR_BIT:    return ts->nat_types[0];
		case CHAR_BIT*2:  return ts->nat_types[1];
		case CHAR_BIT*4:  return ts->nat_types[2];
		case CHAR_BIT*8:  return ts->nat_types[3];
		case CHAR_BIT*16: return ts->nat_types[4];

		default:{
			ham_logapierrorf("Arbitrary bit-width natural types unimplemented");
			return nullptr;
		}
	}
}

const ham_type *ham_typeset_int(const ham_typeset *ts, ham_usize num_bits){
	switch(num_bits){
		case CHAR_BIT:    return ts->int_types[0];
		case CHAR_BIT*2:  return ts->int_types[1];
		case CHAR_BIT*4:  return ts->int_types[2];
		case CHAR_BIT*8:  return ts->int_types[3];
		case CHAR_BIT*16: return ts->int_types[4];

		default:{
			ham_logapierrorf("Arbitrary bit-width integer types unimplemented");
			return nullptr;
		}
	}
}

const ham_type *ham_typeset_rat(const ham_typeset *ts, ham_usize num_bits){
	switch(num_bits){
		case CHAR_BIT:    return ts->rat_types[0];
		case CHAR_BIT*2:  return ts->rat_types[1];
		case CHAR_BIT*4:  return ts->rat_types[2];
		case CHAR_BIT*8:  return ts->rat_types[3];
		case CHAR_BIT*16: return ts->rat_types[4];
		case CHAR_BIT*32: return ts->rat_types[5];

		default:{
			ham_logapierrorf("Arbitrary bit-width rational types unimplemented");
			return nullptr;
		}
	}
}

const ham_type *ham_typeset_float(const ham_typeset *ts, ham_usize num_bits){
	switch(num_bits){
		case 16:  return ts->float_types[0];
		case 32:  return ts->float_types[1];
		case 64:  return ts->float_types[2];
		case 128: return ts->float_types[3];

		default:{
			ham_logapierrorf("Arbitrary bit-width floating-point types unimplemented");
			return nullptr;
		}
	}
}

const ham_type *ham_typeset_str(const ham_typeset *ts, ham_str_encoding encoding){
	switch(encoding){
		case HAM_STR_UTF8:  return ts->str_types[0];
		case HAM_STR_UTF16: return ts->str_types[1];
		case HAM_STR_UTF32: return ts->str_types[2];

		default:{
			ham_logapierrorf("Unrecognized ham_str_encoding: 0x%x", encoding);
			return nullptr;
		}
	}
}

const ham_type *ham_typeset_vec(const ham_typeset *ts, const ham_type *elem, ham_usize n){
	if(elem != ts->float_types[1]){
		ham_logapierrorf("Only vec2<f32> vec3<f32> and vec4<f32> currently supported");
		return nullptr;
	}

	switch(n){
		case 8:  return ts->vec_types[0]; // vec2
		case 12: return ts->vec_types[1]; // vec3
		case 16: return ts->vec_types[2]; // vec4

		default:{
			ham_logapierrorf("Only vec2<f32> vec3<f32> and vec4<f32> currently supported");
			return nullptr;
		}
	}
}

HAM_C_API_END
