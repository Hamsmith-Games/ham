/*
 * Ham Runtime
 * Copyright (C) 2022 Keith Hammond
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
#include "ham/math.h"
#include "ham/check.h"
#include "ham/buffer.h"
#include "ham/colony.h"

#include "ham/std_vector.hpp"

#include "robin_hood.h"

using namespace ham::typedefs;

HAM_C_API_BEGIN

struct ham_type{
	const ham_typeset *ts;
	const char *name;
	u32 flags;
	usize alignment, size;
	const void *data;
	uptr n0, n1;
};

struct ham_type_member{
	ham_str8 name;
	const ham_type *type;
};

struct ham_type_method{
	ham_str8 name;
	ham::basic_buffer<ham_str8> param_names;
	ham::basic_buffer<const ham_type*> param_types;
};

struct ham_type_object{
	ham_derive(ham_type)
	ham::std_vector<ham_type_member> members;
	ham::std_vector<ham_type_method> methods;
};

struct ham_typeset{
	const ham_allocator *allocator;
	mutable ham::colony<ham_type> types;
	mutable robin_hood::unordered_flat_map<ham::str8, const ham_type*> type_map;

	robin_hood::unordered_node_map<ham::str8, ham_type_object> obj_types;

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

//
// Type introspection
//

ham_nothrow ham_u32 ham_type_get_flags(const ham_type *type){
	if(!ham_check(type != NULL)) return (ham_u32)-1;
	return type->flags;
}

ham_nothrow const ham_object_vtable *ham_type_vptr(const ham_type *type){
	if(!ham_check(type != NULL) || !ham_check(ham_type_is_object(type))) return nullptr;
	return reinterpret_cast<const ham_object_vtable*>(type->data);
}

ham_nothrow ham_usize ham_type_num_members(const ham_type *type){
	if(!ham_check(type != NULL) || !ham_check(ham_type_is_object(type))) return (ham_usize)-1;
	return type->n0;
}

ham_nothrow ham_usize ham_type_num_methods(const ham_type *type){
	if(!ham_check(type != NULL) || !ham_check(ham_type_is_object(type))) return (ham_usize)-1;
	return type->n1;
}

ham_usize ham_type_members_iterate(const ham_type *type, ham_type_members_iterate_fn fn, void *user){
	if(!ham_check(type != NULL) || !ham_check(ham_type_is_object(type))) return (ham_usize)-1;

	if(fn && type->n0 != (ham_uptr)-1){
		const auto &obj_type_map = type->ts->obj_types;

		const auto obj_res = obj_type_map.find(ham::str8(type->name));
		if(obj_res == obj_type_map.end()){
			ham::logapierror("Could not find object type within it's typeset");
			return (ham_usize)-1;
		}

		const auto &members = obj_res->second.members;

		for(ham_usize i = 0; i < type->n0; i++){
			if(!fn(i, members[i].name, members[i].type, user)){
				return i;
			}
		}
	}

	return type->n0;
}

ham_usize ham_type_methods_iterate(const ham_type *type, ham_type_methods_iterate_fn fn, void *user){
	if(!ham_check(type != NULL) || !ham_check(ham_type_is_object(type))) return (ham_usize)-1;

	if(fn && type->n1 != (ham_uptr)-1){
		const auto &obj_type_map = type->ts->obj_types;

		const auto obj_res = obj_type_map.find(ham::str8(type->name));
		if(obj_res == obj_type_map.end()){
			ham::logapierror("Could not find object type within it's typeset");
			return (ham_usize)-1;
		}

		const auto &methods = obj_res->second.methods;

		for(ham_usize i = 0; i < type->n1; i++){
			const auto &param_names = methods[i].param_names;
			const auto &param_types = methods[i].param_types;
			if(!fn(i, methods[i].name, param_types.size(), param_names.data(), param_types.data(), user)){
				return i;
			}
		}
	}

	return type->n1;
}

//
// Typesets
//

ham_nonnull_args(1)
static inline const ham_type *ham_impl_typeset_new_type(
	ham_typeset *ts,
	const char *name,
	ham_type_kind_flag kind,
	ham_type_info_flag info,
	usize alignment,
	usize size,
	const void *data = nullptr,
	uptr n0 = (uptr)-1,
	uptr n1 = (uptr)-1
){
	const auto name_str = ham::str8(name);

	const auto name_res = ts->type_map.find(name_str);
	if(name_res != ts->type_map.end()){
		ham::logapierror("Type by name '{}' already exists in typeset", name);
		return nullptr;
	}

	const auto new_type = ts->types.emplace();
	new_type->ts = ts;
	new_type->name = name;
	new_type->flags = ham_make_type_flags(kind, info);
	new_type->alignment = alignment;
	new_type->size = size;
	new_type->data = data;
	new_type->n0 = n0;
	new_type->n1 = n1;

	if(!ts->type_map.try_emplace(name_str, new_type).second){
		ham::logapiwarn("Type by name '{}' could not be emplaced in typeset map", name);
	}

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

ham_nothrow const ham_type *ham_typeset_get(const ham_typeset *ts, ham_str8 name){
	if(!ham_check(ts != NULL)) return nullptr;

	const auto res = ts->type_map.find(name);
	if(res != ts->type_map.end()){
		return res->second;
	}

	return nullptr;
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

const ham_type *ham_typeset_object(const ham_typeset *ts, ham_str8 name){
	if(!ham_check(ts != NULL)) return nullptr;

	const auto res = ts->obj_types.find(name);
	if(res != ts->obj_types.end()){
		return ham_super(&res->second);
	}

	return nullptr;
}

//
// Type builders
//

struct ham_type_builder{
	const ham_allocator *allocator;
	ham_name_buffer_utf8 name_buf;
	ham_type_kind_flag kind;
	ham_type_info_flag info;
	const ham_type *instance;
	const ham_type *parent;
	const ham_object_vtable *vptr;
	ham::std_vector<ham_type_member> members;
	ham::std_vector<ham_type_method> methods;
};

ham_type_builder *ham_type_builder_create(){
	const auto allocator = ham_current_allocator();

	const auto ptr = ham_allocator_new(allocator, ham_type_builder);
	if(!ptr) return nullptr;

	ptr->allocator = allocator;
	memset(ptr->name_buf, 0, sizeof(ptr->name_buf));
	ptr->kind = (ham_type_kind_flag)-1;
	ptr->info = (ham_type_info_flag)-1;
	ptr->instance = nullptr;

	return ptr;
}

ham_nothrow void ham_type_builder_destroy(ham_type_builder *builder){
	if(ham_unlikely(!builder)) return;

	const auto allocator = builder->allocator;

	ham_allocator_delete(allocator, builder);
}

ham_nothrow bool ham_type_builder_reset(ham_type_builder *builder){
	if(!ham_check(builder != NULL)) return false;

	memset(builder->name_buf, 0, sizeof(builder->name_buf));
	builder->kind = (ham_type_kind_flag)-1;
	builder->info = (ham_type_info_flag)-1;
	builder->instance = nullptr;

	return true;
}

const ham_type *ham_type_builder_instantiate(ham_type_builder *builder, ham_typeset *ts){
	if(!ham_check(builder != NULL)) return nullptr;
	else if(builder->instance) return builder->instance;

	switch(builder->kind){
		case HAM_TYPE_OBJECT:{
			ham::logapierror("Object type builders currently unimplemented");
			return nullptr;
		}

		default:{
			ham::logapierror("Type builders currently unimplemented");
			return nullptr;
		}
	}
}

ham_nothrow bool ham_type_builder_set_parent(ham_type_builder *builder, const ham_type *parent_type){
	if(!ham_check(builder != NULL) || !ham_check(ham_type_is_object(parent_type))) return false;

	if(builder->kind == (ham_type_kind_flag)-1){
		builder->kind   = HAM_TYPE_OBJECT;
		builder->info   = HAM_TYPE_INFO_OBJECT_POD;
	}
	else if(builder->kind != HAM_TYPE_OBJECT){
		ham::logapierror("Type builder already contains non-object");
		return false;
	}
	else if(builder->parent){
		if(parent_type == builder->parent){
			return true;
		}

		ham::logapiwarn("Type builder already has parent set (overwriting)");
	}

	builder->parent   = parent_type;
	builder->instance = nullptr;

	return true;
}

ham_nothrow bool ham_type_builder_set_vptr(ham_type_builder *builder, const ham_object_vtable *vptr){
	if(!ham_check(builder != NULL) || !ham_check(vptr != NULL)) return false;

	if(builder->kind == (ham_type_kind_flag)-1){
		builder->kind   = HAM_TYPE_OBJECT;
	}
	else if(builder->kind != HAM_TYPE_OBJECT){
		ham::logapierror("Type builder already contains non-object");
		return false;
	}
	else if(builder->vptr){
		if(vptr == builder->vptr){
			return true;
		}

		ham::logapiwarn("Type builder already has vptr set (overwriting)");
	}

	builder->info     = HAM_TYPE_INFO_OBJECT_VIRTUAL;
	builder->vptr     = vptr;
	builder->instance = nullptr;

	return true;
}

bool ham_type_builder_add_member(ham_type_builder *builder, ham_str8 name, const ham_type *type){
	if(!ham_check(builder != NULL) || !ham_check(name.ptr && name.len) || !ham_check(type != NULL)) return false;

	if(builder->kind != (ham_type_kind_flag)-1){
		if(builder->kind != HAM_TYPE_OBJECT){
			ham::logapierror("Can not add members to non-object types");
			return false;
		}
	}
	else{
		builder->kind = HAM_TYPE_OBJECT;
		builder->info = HAM_TYPE_INFO_OBJECT_POD;
	}

	auto &&member = builder->members.emplace_back();
	member.name = name;
	member.type = type;

	builder->instance = nullptr;
	return true;
}

bool ham_type_builder_add_method(ham_type_builder *builder, ham_str8 name, ham_usize num_params, const ham_str8 *param_names, const ham_type *const *param_types){
	if(
	   !ham_check(builder != NULL) ||
	   !ham_check(builder->info == HAM_TYPE_INFO_OBJECT_VIRTUAL) ||
	   !ham_check(name.ptr && name.len) ||
	   !ham_check(num_params == 0 || (param_names && param_types))
	){
		return false;
	}

	for(usize i = 0; i < num_params; i++){
		const auto param_type  = param_types[i];

		if(!param_type){
			ham::logapierror("NULL passed for parameter type {}", i);
			return false;
		}
	}

	auto &&method = builder->methods.emplace_back();
	method.name = name;

	method.param_names.resize(num_params);
	method.param_types.resize(num_params);

	memcpy(method.param_names.data(), param_names, num_params * sizeof(ham_str8));
	memcpy(method.param_types.data(), param_types, num_params * sizeof(const ham_type*));

	builder->instance = nullptr;
	return true;
}

HAM_C_API_END
