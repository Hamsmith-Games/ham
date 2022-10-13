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

#include "typedefs.h"

HAM_C_API_BEGIN

typedef struct ham_typeset ham_typeset;

typedef enum ham_type_kind_flag{
	HAM_TYPE_THEORETIC,
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

ham_constexpr ham_nothrow static inline ham_u32 ham_make_type_flags(ham_type_kind_flag kind, ham_type_info_flag info){
	return
		((kind & HAM_TYPE_FLAGS_KIND_MASK) << HAM_TYPE_FLAGS_KIND_SHIFT) |
		((info & HAM_TYPE_FLAGS_INFO_MASK) << HAM_TYPE_FLAGS_INFO_SHIFT)
	;
}

ham_constexpr ham_nothrow static inline ham_type_kind_flag ham_type_flags_kind(ham_u32 flags){
	return (ham_type_kind_flag)((flags & HAM_TYPE_FLAGS_KIND_MASK) >> HAM_TYPE_FLAGS_KIND_SHIFT);
}

ham_constexpr ham_nothrow static inline ham_type_info_flag ham_type_flags_info(ham_u32 flags){
	return (ham_type_info_flag)((flags & HAM_TYPE_FLAGS_INFO_MASK) >> HAM_TYPE_FLAGS_INFO_SHIFT);
}

ham_api const char *ham_type_name(const ham_type *type);



/**
 * @defgroup HAM_TYPESYS_TYPESET Typesets
 * @{
 */

ham_api ham_typeset *ham_typeset_create();

ham_api void ham_typeset_destroy(ham_typeset *ts);

ham_api const ham_type *ham_typeset_get(const ham_typeset *ts, ham_str8 name);

ham_api const ham_type *ham_typeset_void(const ham_typeset *ts);
ham_api const ham_type *ham_typeset_unit(const ham_typeset *ts);
ham_api const ham_type *ham_typeset_top(const ham_typeset *ts);
ham_api const ham_type *ham_typeset_bottom(const ham_typeset *ts);
ham_api const ham_type *ham_typeset_ref(const ham_typeset *ts, const ham_type *refed);

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

HAM_C_API_END

#ifdef __cplusplus

namespace ham{
	template<typename TypeClass = void>
	class basic_type{
		public:
			using pointer = const ham_type*;

			basic_type(pointer ptr_ = nullptr) noexcept
				: m_ptr(ptr_){}



		private:
			pointer m_ptr;
	};

	class typeset{
		public:
			typeset(): m_handle(ham_typeset_create()){}

		private:
			unique_handle<ham_typeset*, ham_typeset_destroy> m_handle;
	};
}

#endif // __cplusplus

/**
 * @}
 */

#endif // !HAM_TYPESYS_H
