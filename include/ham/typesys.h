/*
 * Ham Runtime
 * Copyright (C) 2022  Hamsmith Ltd.
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

#ifndef HAM_TYPESYS_H
#define HAM_TYPESYS_H 1

/**
 * @defgroup HAM_TYPESYS Runtime types
 * @ingroup HAM
 * @{
 */

#include "object.h"

HAM_C_API_BEGIN

typedef struct ham_typeset ham_typeset;

typedef enum ham_type_kind_flag{
	HAM_TYPE_THEORETIC,
	HAM_TYPE_OBJECT,
	HAM_TYPE_STRING,
	HAM_TYPE_NUMERIC,

	HAM_TYPE_KIND_MAX_VALUE = 0x7,
} ham_type_kind_flag;

typedef enum ham_type_info_flag{
	HAM_TYPE_INFO_THEORETIC_VOID = 0x0,
	HAM_TYPE_INFO_THEORETIC_UNIT,
	HAM_TYPE_INFO_THEORETIC_TOP,
	HAM_TYPE_INFO_THEORETIC_BOTTOM,
	HAM_TYPE_INFO_THEORETIC_REF,
	HAM_TYPE_INFO_THEORETIC_OBJECT,

	HAM_TYPE_INFO_OBJECT_POD = 0x0,
	HAM_TYPE_INFO_OBJECT_VIRTUAL,

	HAM_TYPE_INFO_STRING_UTF8  = 0x0,
	HAM_TYPE_INFO_STRING_UTF16,
	HAM_TYPE_INFO_STRING_UTF32,

	HAM_TYPE_INFO_NUMERIC_BOOLEAN = 0x0,
	HAM_TYPE_INFO_NUMERIC_NATURAL,
	HAM_TYPE_INFO_NUMERIC_INTEGER,
	HAM_TYPE_INFO_NUMERIC_RATIONAL,
	HAM_TYPE_INFO_NUMERIC_FLOATING_POINT,
	HAM_TYPE_INFO_NUMERIC_VECTOR,

	HAM_TYPE_INFO_MAX_VALUE = 0x3F
} ham_type_info_flag;

#define HAM_TYPE_FLAGS_KIND_SHIFT 0u
#define HAM_TYPE_FLAGS_KIND_MASK HAM_TYPE_KIND_MAX_VALUE

#define HAM_TYPE_FLAGS_INFO_SHIFT (ham_popcnt(HAM_TYPE_FLAGS_KIND_MASK))
#define HAM_TYPE_FLAGS_INFO_MASK (HAM_TYPE_INFO_MAX_VALUE << HAM_TYPE_FLAGS_INFO_SHIFT)

typedef struct ham_type ham_type;

ham_api ham_nothrow ham_u32 ham_type_get_flags(const ham_type *type);

ham_used
ham_constexpr ham_nothrow static inline ham_u32 ham_make_type_flags(ham_type_kind_flag kind, ham_type_info_flag info){
	return
		((kind & HAM_TYPE_FLAGS_KIND_MASK) << HAM_TYPE_FLAGS_KIND_SHIFT) |
		((info & HAM_TYPE_FLAGS_INFO_MASK) << HAM_TYPE_FLAGS_INFO_SHIFT)
	;
}

ham_used
ham_constexpr ham_nothrow static inline ham_type_kind_flag ham_type_flags_kind(ham_u32 flags){
	return (ham_type_kind_flag)((flags & HAM_TYPE_FLAGS_KIND_MASK) >> HAM_TYPE_FLAGS_KIND_SHIFT);
}

ham_used
ham_constexpr ham_nothrow static inline ham_type_info_flag ham_type_flags_info(ham_u32 flags){
	return (ham_type_info_flag)((flags & HAM_TYPE_FLAGS_INFO_MASK) >> HAM_TYPE_FLAGS_INFO_SHIFT);
}

ham_api const char *ham_type_name(const ham_type *type);

/**
 * @defgroup HAM_TYPESYS_OBJECT Object type introspection
 * @{
 */

ham_nonnull_args(1) ham_used
ham_nothrow static inline bool ham_type_is_object(const ham_type *type){
	const ham_u32 flags = ham_type_get_flags(type);
	return ham_type_flags_kind(flags) == HAM_TYPE_OBJECT;
}

ham_api ham_nothrow const ham_object_vtable *ham_type_vptr(const ham_type *type);

ham_api ham_nothrow ham_usize ham_type_num_members(const ham_type *type);
ham_api ham_nothrow ham_usize ham_type_num_methods(const ham_type *type);

typedef bool(*ham_type_members_iterate_fn)(ham_usize i, ham_str8 name, const ham_type *type, void *user);

ham_api ham_usize ham_type_members_iterate(const ham_type *type, ham_type_members_iterate_fn fn, void *user);

typedef bool(*ham_type_methods_iterate_fn)(ham_usize i, ham_str8 name, ham_usize num_params, const ham_str8 *param_names, const ham_type *const *param_types, void *user);

ham_api ham_usize ham_type_methods_iterate(const ham_type *type, ham_type_methods_iterate_fn fn, void *user);

/**
 * @}
 */

/**
 * @defgroup HAM_TYPESYS_TYPESET Typesets
 * @{
 */

ham_api ham_typeset *ham_typeset_create();

ham_api void ham_typeset_destroy(ham_typeset *ts);

ham_api ham_nothrow const ham_type *ham_typeset_get(const ham_typeset *ts, ham_str8 name);

ham_api const ham_type *ham_typeset_void(const ham_typeset *ts);
ham_api const ham_type *ham_typeset_unit(const ham_typeset *ts);
ham_api const ham_type *ham_typeset_top(const ham_typeset *ts);
ham_api const ham_type *ham_typeset_bottom(const ham_typeset *ts);
ham_api const ham_type *ham_typeset_ref(const ham_typeset *ts, const ham_type *refed);
ham_api const ham_type *ham_typeset_object(const ham_typeset *ts, ham_str8 name);

ham_api const ham_type *ham_typeset_bool(const ham_typeset *ts);
ham_api const ham_type *ham_typeset_nat(const ham_typeset *ts, ham_usize num_bits);
ham_api const ham_type *ham_typeset_int(const ham_typeset *ts, ham_usize num_bits);
ham_api const ham_type *ham_typeset_rat(const ham_typeset *ts, ham_usize num_bits);
ham_api const ham_type *ham_typeset_float(const ham_typeset *ts, ham_usize num_bits);

ham_api const ham_type *ham_typeset_str(const ham_typeset *ts, ham_str_encoding encoding);

ham_api const ham_type *ham_typeset_vec(const ham_typeset *ts, const ham_type *elem, ham_usize n);

/**
 * @}
 */

/**
 * @defgroup HAM_TYPESYS_BUILDER Type builders
 * @{
 */

typedef struct ham_type_builder ham_type_builder;

ham_api ham_type_builder *ham_type_builder_create();

ham_api ham_nothrow void ham_type_builder_destroy(ham_type_builder *builder);

ham_api ham_nothrow bool ham_type_builder_reset(ham_type_builder *builder);

ham_api const ham_type *ham_type_builder_instantiate(ham_type_builder *builder, ham_typeset *ts);

ham_api ham_nothrow bool ham_type_builder_set_parent(ham_type_builder *builder, const ham_type *parent_type);
ham_api ham_nothrow bool ham_type_builder_set_vptr(ham_type_builder *builder, const ham_object_vtable *vptr);

ham_api bool ham_type_builder_add_member(ham_type_builder *builder, ham_str8 name, const ham_type *type);
ham_api bool ham_type_builder_add_method(ham_type_builder *builder, ham_str8 name, ham_usize num_params, const ham_str8 *param_names, const ham_type *const *param_types);

/**
 * @}
 */

HAM_C_API_END

#ifdef __cplusplus

namespace ham{
	class type{
		public:
			using pointer = const ham_type*;

			constexpr type(pointer ptr_ = nullptr) noexcept: m_ptr(ptr_){}

			constexpr operator bool() const noexcept{ return !!m_ptr; }
			constexpr operator pointer() const noexcept{ return m_ptr; }

			constexpr bool operator==(type other) const noexcept{ return m_ptr == other.m_ptr; }
			constexpr bool operator!=(type other) const noexcept{ return m_ptr != other.m_ptr; }

			constexpr pointer ptr() const noexcept{ return m_ptr; }

		private:
			pointer m_ptr;
	};

	template<typename ... Tags>
	class basic_typeset_view{
		public:
			static constexpr bool is_mutable = meta::type_list_contains_v<meta::type_list<Tags...>, mutable_tag>;

			using pointer = std::conditional_t<is_mutable, ham_typeset*, const ham_typeset*>;

			constexpr basic_typeset_view(pointer ptr = nullptr) noexcept: m_ptr(ptr){}

			constexpr operator bool() const noexcept{ return !!m_ptr; }
			constexpr operator pointer() const noexcept{ return m_ptr; }

			constexpr pointer ptr() const noexcept{ return m_ptr; }

			type get(str8 name) const noexcept{ return ham_typeset_get(m_ptr, name); }

		private:
			pointer m_ptr;
	};

	using typeset_view = basic_typeset_view<mutable_tag>;
	using const_typeset_view = basic_typeset_view<>;

	class typeset{
		public:
			typeset(): m_handle(ham_typeset_create()){}

			operator typeset_view() noexcept{ return m_handle.get(); }
			operator const_typeset_view() const noexcept{ return m_handle.get(); }

		private:
			unique_handle<ham_typeset*, ham_typeset_destroy> m_handle;
	};

	class type_builder{
		public:
			type_builder(): m_handle(ham_type_builder_create()){}

		private:
			unique_handle<ham_type_builder*, ham_type_builder_destroy> m_handle;
	};
}

#endif // __cplusplus

/**
 * @}
 */

#endif // !HAM_TYPESYS_H
