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

#ifndef HAM_TYPEDEFS_H
#define HAM_TYPEDEFS_H 1

/**
 * @defgroup HAM_TYPEDEFS Typedefs
 * @ingroup HAM
 * @{
 */

#include "ham/config.h"
#include "pp.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <uchar.h>

HAM_C_API_BEGIN

static_assert(sizeof(float)  == 4,  "Ham expects 32-bit floats");
static_assert(sizeof(double) == 8, "Ham expects 64-bit doubles");

typedef struct alignas(char) { char _pad; } ham_unit;

static_assert(sizeof (ham_unit) == 1, "ham_unit is improperly sized.");
static_assert(alignof(ham_unit) == 1, "ham_unit is improperly aligned.");

typedef     char ham_char8;
typedef char16_t ham_char16;
typedef char32_t ham_char32;

typedef   int8_t ham_i8;
typedef  uint8_t ham_u8;
typedef  int16_t ham_i16;
typedef uint16_t ham_u16;
typedef  int32_t ham_i32;
typedef uint32_t ham_u32;
typedef  int64_t ham_i64;
typedef uint64_t ham_u64;

typedef  intptr_t ham_iptr;
typedef uintptr_t ham_uptr;

typedef ham_iptr ham_isize;
typedef ham_uptr ham_usize;

typedef float  ham_f32;
typedef double ham_f64;

#if defined(__GNUC__) && defined(__SIZEOF_INT128__)
#	define HAM_INT128 1
	typedef __int128 ham_i128;
	typedef unsigned __int128 ham_u128;
#endif

#if defined(__GNUC__) && defined(__SIZEOF_FLOAT128__)
#	define HAM_FLOAT128 1
	typedef __float128 ham_f128;
#endif

#if defined(__GNUC__) && defined(__SIZEOF_FLOAT16__)
#	define HAM_FLOAT16 1
	typedef _Float16 ham_f16;
#endif

#define HAM_I8_MIN    INT8_MIN
#define HAM_I8_MAX    INT8_MAX
#define HAM_U8_MIN           0
#define HAM_U8_MAX   UINT8_MAX
#define HAM_I16_MIN  INT16_MIN
#define HAM_I16_MAX  INT16_MAX
#define HAM_U16_MIN          0
#define HAM_U16_MAX UINT16_MAX
#define HAM_I32_MIN  INT32_MIN
#define HAM_I32_MAX  INT32_MAX
#define HAM_U32_MIN          0
#define HAM_U32_MAX UINT32_MAX
#define HAM_I64_MIN  INT64_MIN
#define HAM_I64_MAX  INT64_MAX
#define HAM_U64_MIN          0
#define HAM_U64_MAX UINT64_MAX

#define HAM_IPTR_MIN  INTPTR_MIN
#define HAM_IPTR_MAX  INTPTR_MAX
#define HAM_UPTR_MIN UINTPTR_MIN
#define HAM_UPTR_MAX UINTPTR_MAX

#define HAM_ISIZE_MIN HAM_IPTR_MIN
#define HAM_ISIZE_MAX HAM_IPTR_MAX
#define HAM_USIZE_MIN            0
#define HAM_USIZE_MAX HAM_UPTR_MAX

/**
 * @defgroup HAM_STATIC_BUFFERS Static buffer types
 * @{
 */

typedef ham_char8  ham_name_buffer_utf8 [HAM_NAME_BUFFER_SIZE];
typedef ham_char16 ham_name_buffer_utf16[HAM_NAME_BUFFER_SIZE];
typedef ham_char32 ham_name_buffer_utf32[HAM_NAME_BUFFER_SIZE];

typedef ham_char8  ham_path_buffer_utf8 [HAM_PATH_BUFFER_SIZE];
typedef ham_char16 ham_path_buffer_utf16[HAM_PATH_BUFFER_SIZE];
typedef ham_char32 ham_path_buffer_utf32[HAM_PATH_BUFFER_SIZE];

typedef ham_char8  ham_message_buffer_utf8 [HAM_MESSAGE_BUFFER_SIZE];
typedef ham_char16 ham_message_buffer_utf16[HAM_MESSAGE_BUFFER_SIZE];
typedef ham_char32 ham_message_buffer_utf32[HAM_MESSAGE_BUFFER_SIZE];

typedef ham_char8  ham_uuid_buffer_utf8 [HAM_UUID_BUFFER_SIZE];
typedef ham_char16 ham_uuid_buffer_utf16[HAM_UUID_BUFFER_SIZE];
typedef ham_char32 ham_uuid_buffer_utf32[HAM_UUID_BUFFER_SIZE];

#define HAM_NAME_BUFFER_UTF(n) HAM_CONCAT(ham_name_buffer_utf, n)
#define HAM_PATH_BUFFER_UTF(n) HAM_CONCAT(ham_path_buffer_utf, n)
#define HAM_MESSAGE_BUFFER_UTF(n) HAM_CONCAT(ham_message_buffer_utf, n)
#define HAM_UUID_BUFFER_UTF(n) HAM_CONCAT(ham_uuid_buffer_utf, n)

typedef HAM_NAME_BUFFER_UTF(HAM_UTF)    ham_name_buffer;
typedef HAM_PATH_BUFFER_UTF(HAM_UTF)    ham_path_buffer;
typedef HAM_MESSAGE_BUFFER_UTF(HAM_UTF) ham_message_buffer;
typedef HAM_UUID_BUFFER_UTF(HAM_UTF)    ham_uuid_buffer;

/**
 * @}
 */

/**
 * @defgroup HAM_VERSIONS Versions
 * @{
 */

typedef struct ham_version{
	ham_u16 major, minor, patch;
} ham_version;

#define HAM_VERSION ((ham_version){HAM_VERSION_MAJOR,HAM_VERSION_MINOR,HAM_VERSION_PATCH})

ham_constexpr ham_nothrow static inline int ham_version_cmp(ham_version lhs, ham_version rhs){
	const int dmajor = (int)lhs.major - (int)rhs.major;
	if(dmajor != 0) return dmajor;

	const int dminor = (int)lhs.minor - (int)rhs.minor;
	if(dminor != 0) return dminor;

	const int dpatch = (int)lhs.patch - (int)rhs.patch;
	return dpatch;
}

ham_constexpr ham_nothrow static inline bool ham_version_is_compatible(ham_version supplied, ham_version requested){
	return supplied.major == requested.major && supplied.minor >= requested.minor;
}

/**
 * @}
 */

/**
 * @defgroup HAM_UUIDS UUIDs
 * @{
 */

typedef union ham_uuid{
	char bytes[16];
	ham_u8 u8s[16];
	ham_u16 u16s[8];
	ham_u32 u32s[4];
	ham_u64 u64s[2];
#ifdef HAM_INT128
	ham_u128 u128;
#endif
} ham_uuid;

ham_constexpr ham_nothrow static inline int ham_uuid_cmp(ham_uuid a, ham_uuid b){
	for(ham_usize i = 0; i < 16; i++){
		const int d = a.u8s[i] - b.u8s[i];
		if(d != 0) return d;
	}

	return 0;
}

ham_constexpr ham_nothrow static inline bool ham_uuid_eq (ham_uuid a, ham_uuid b){ return ham_uuid_cmp(a, b) == 0; }
ham_constexpr ham_nothrow static inline bool ham_uuid_neq(ham_uuid a, ham_uuid b){ return ham_uuid_cmp(a, b) != 0; }
ham_constexpr ham_nothrow static inline bool ham_uuid_lt (ham_uuid a, ham_uuid b){ return ham_uuid_cmp(a, b) <  0; }
ham_constexpr ham_nothrow static inline bool ham_uuid_lte(ham_uuid a, ham_uuid b){ return ham_uuid_cmp(a, b) <= 0; }
ham_constexpr ham_nothrow static inline bool ham_uuid_gt (ham_uuid a, ham_uuid b){ return ham_uuid_cmp(a, b) >  0; }
ham_constexpr ham_nothrow static inline bool ham_uuid_gte(ham_uuid a, ham_uuid b){ return ham_uuid_cmp(a, b) >= 0; }

/**
 * @}
 */

/**
 * @defgroup HAM_UTF_CP UTF Codepoints
 * @{
 */

typedef ham_u32 ham_utf_cp;

/**
 * Check for unicode property ``White_Space``.
 */
ham_constexpr ham_nothrow static inline bool ham_utf_is_whitespace(ham_utf_cp cp){
	return
		(cp > 0x8 && cp < 0xE) || // basic latin spaces
		(cp == 0x20) || // SPACE
		(cp == 0x85) || // NEXT LINE
		(cp == 0xA0) || // NO-BREAK SPACE
		(cp == 0x1680) || // OGHAM SPACE MARK
		(cp > 0x2000 && cp < 0x200A) || // WHOLE BUNCH OF WEIRD SPACES
		(cp == 0x2028) || // LINE SEPARATOR
		(cp == 0x2029) || // PARAGRAPH SEPARATOR
		(cp == 0x202F) || // NARROW NO-BREAK SPACE
		(cp == 0x205F) || // MEDIUM MATHEMATICAL SPACE
		(cp == 0x3000) // IDEOGRAPHIC SPACE
	;
}

/**
 * Check for numeric digit codepoints.
 * @note currently only supports ASCII subset
 */
ham_constexpr ham_nothrow static inline bool ham_utf_is_digit(ham_utf_cp cp){
	return
		(cp > 0x2F && cp < 0x3A) // ASCII
	;
}

/**
 * Check for quote codepoints.
 * @note currently only supports ASCII subset
 */
ham_constexpr ham_nothrow static inline bool ham_utf_is_quote(ham_utf_cp cp){
	switch(cp){
		case U'\'':
		case U'"':
			return true;

		default: return false;
	}
}

/**
 * Check for alphabetic codepoints.
 * @note currently only supports ASCII subset
 */
ham_constexpr ham_nothrow static inline bool ham_utf_is_alpha(ham_utf_cp cp){
	return
		(cp > 0x40 && cp < 0x5B) || (cp > 0x60 && cp < 0x7B) // ASCII
	;
}

/**
 * Check whether a codepoint is considered an "operator" codepoint
 * @note currently only supports ASCII subset
 */
ham_constexpr ham_nothrow static inline bool ham_utf_is_op(ham_utf_cp cp){
	switch(cp){
		case U'!':
		case U'%':
		case U'&':
		case U'*':
		case U'+':
		case U'-':
		case U'/':
		case U'<':
		case U'=':
		case U'>':
		case U'~':
		case U'^':
		case U'|':
			return true;

		default: return false;
	}
}

/**
 * Check whether a codepoint is considered a bracket
 * @note currently only supports ASCII subset
 */
ham_constexpr ham_nothrow static inline bool ham_utf_is_bracket(ham_utf_cp cp){
	switch(cp){
		case U'(':
		case U')':
		case U'[':
		case U']':
		case U'{':
		case U'}':
			return true;

		default: return false;
	}
}

ham_constexpr ham_nothrow static inline ham_utf_cp ham_utf_to_lower(ham_utf_cp cp){
	if(cp < 91 && cp > 64) return cp + 33;
	else return cp;
}

ham_constexpr ham_nothrow static inline ham_utf_cp ham_utf_to_upper(ham_utf_cp cp){
	if(cp < 123 && cp > 96) return cp - 33;
	else return cp;
}

ham_constexpr ham_nothrow static inline char ham_to_lower(char c){
	if(c < 91 && c > 64) return c + 33;
	else return c;
}

ham_constexpr ham_nothrow static inline char ham_to_upper(char c){
	if(c < 123 && c > 96) return c - 33;
	else return c;
}

ham_constexpr ham_nothrow static inline ham_u32 ham_char_hex_value(char c){
	c = ham_to_upper(c);
	if(c > 47 && c < 58) return (ham_u32)(c - 48);
	else if(c > 64 && c < 71) return 10u + (ham_u32)(c - 65);
	else return (ham_u32)-1;
}

#define HAM_UTF8_NON_ASCII_BIT 0x80
#define HAM_UTF8_BYTE_MASK 0xF0
#define HAM_UTF16_SURROGATE_HIGH_MIN 0xD800
#define HAM_UTF16_SURROGATE_HIGH_MAX 0xDBFF

ham_constexpr ham_nothrow static inline ham_usize ham_codepoint_num_chars_utf8(const ham_char8 *cp, ham_usize max_len){
	if(!cp || max_len == 0){
		return 0;
	}
	else if(*cp & HAM_UTF8_NON_ASCII_BIT){
		const ham_u32 byte_mask = (ham_u8)*cp & (ham_u8)HAM_UTF8_BYTE_MASK;
		const ham_u32 count = ham_popcnt32(byte_mask);
		return (max_len < count) ? (ham_usize)-1 : count;
	}
	else{
		return 1;
	}
}

ham_constexpr ham_nothrow static inline ham_usize ham_codepoint_num_chars_utf16(const ham_char16 *cp, ham_usize max_len){
	if(!cp || max_len == 0) return 0;

	const ham_char16 high = *cp;

	if(high <= HAM_UTF16_SURROGATE_HIGH_MAX && high >= HAM_UTF16_SURROGATE_HIGH_MIN){
		// surrogate pair
		return max_len >= 2 ? 2 : (ham_usize)-1;
	}
	else{
		return 1;
	}
}

ham_constexpr ham_nothrow static inline ham_usize ham_codepoint_num_chars_utf32(const ham_char32 *cp, ham_usize max_len){
	return (!cp || max_len == 0) ? 0 : 1;
}

#define HAM_CHAR_UTF(n) HAM_CONCAT(ham_char, n)

typedef HAM_CHAR_UTF(HAM_UTF) ham_uchar;

/**
 * @}
 */

/**
 * @defgroup HAM_STR Strings
 * @{
 */

typedef struct ham_str8 { const ham_char8  *ptr; ham_uptr len; } ham_str8;
typedef struct ham_str16{ const ham_char16 *ptr; ham_uptr len; } ham_str16;
typedef struct ham_str32{ const ham_char32 *ptr; ham_uptr len; } ham_str32;

//! @cond ignore

#ifdef __GNUC__

#	define HAM_IMPL_STR_CMP(a, b) \
		({	const ham_auto a_ = (a); const ham_auto b_ = (b);\
			const ham_usize max_len_ = a_.len > b_.len ? b_.len : a_.len; \
			int i_ = 0; \
			int res_ = 0; \
			for(; i_ < max_len_; i_++){ \
				res_ = a_.ptr[i_] - b_.ptr[i_]; \
				if(res_ != 0) break;\
			} \
			if(res_ == 0 && i_ != max_len_){ \
				res_ = (max_len_ == a_.len) ? b_.ptr[max_len_] : a_.ptr[max_len_]; \
			} \
			res_; })

#	define HAM_IMPL_STR_EQ(a, b) \
		({	const ham_auto a_ = (a); const ham_auto b_ = (b); \
			if(a_.len != b_.len) return false; \
			bool res_ = true; \
			for(ham_usize i_ = 0; i_ < a_.len; i_++){ \
				if(a_.ptr[i_] != b_.ptr[i_]){ \
					res_ = false; \
					break; \
				} \
			} \
			res_; })

#	define HAM_IMPL_STR_NEQ(a, b) \
		({	const ham_auto a__ = (a); const ham_auto b__ = (b); \
			(a__.len == b__.len) ? false : HAM_IMPL_STR_CMP(a__, b__) != 0; })

#else // __cplusplus

#	define HAM_IMPL_STR_CMP(a, b) \
		([](const auto a_, const auto b_) constexpr{ \
			const ham_usize max_len_ = a_.len > b_.len ? b_.len : a_.len; \
			for(ham_usize i_ = 0; i_ < max_len_; i_++){ \
				const int res_ = a_.ptr[i_] - b_.ptr[i_]; \
				if(res_ != 0) return res_;\
			} \
			if(a_.len == b_.len) return 0; \
			else return (max_len_ == a_.len) ? b_.ptr[max_len_] : a_.ptr[max_len_]; \
		}((a), (b)))

#	define HAM_IMPL_STR_EQ(a, b) \
		([](const auto a_, const auto b_) constexpr{ \
			if(a_.len != b_.len) return false; \
			for(ham_usize i_ = 0; i_ < max_len_; i_++){ \
				if(a_.ptr[i_] != b_.ptr[i_]) return false;\
			} \
			return true; \
		}((a), (b)))

#	define HAM_IMPL_STR_NEQ(a, b) \
		([](const auto a__, const auto b__) constexpr{ \
			return (a__.len == b__.len) ? false : HAM_IMPL_STR_CMP(a__, b__) != 0; \
		}((a), (b)))

#endif // __GNUC__

//! @endcond

ham_constexpr ham_nothrow static inline ham_usize ham_str_next_codepoint_utf8(ham_utf_cp *cp, const ham_char8 *str, ham_usize max_chars){
	const ham_usize req_chars = ham_codepoint_num_chars_utf8(str, max_chars);
	if(req_chars == (ham_usize)-1) return req_chars;

	if(!cp || max_chars < req_chars) return (ham_usize)-1;

	switch(req_chars){
		case 1:{
			if(str[0] < 0) return (ham_usize)-1;

			*cp = str[0];
			break;
		}

		case 2:{
			if(((ham_u8)str[0] == 0xED) && (((ham_u8)str[1] & 0xA0) == 0xA0)){
				// invalid character in range 0xD800 -> 0xDFFF
				return (ham_usize)-1;
			}

			*cp = ((str[0] & 0b00011111) << 6) | (str[1] & 0b00111111);
			break;
		}

		case 3:{
			if(((ham_u8)str[0] == 0xED) && (((ham_u8)str[1] & 0xA0) == 0xA0)){
				// invalid character in range 0xD800 -> 0xDFFF
				return (ham_usize)-1;
			}

			*cp =
				((str[0] & 0b00001111) << 12) |
				((str[1] & 0b00111111) << 6)  |
				(str[2] & 0b00111111);

			break;
		}

		case 4:{
			if(((ham_u8)str[0] == 0xED) && (((ham_u8)str[1] & 0xA0) == 0xA0)){
				// invalid character in range 0xD800 -> 0xDFFF
				return (ham_usize)-1;
			}

			*cp =
				((str[0] & 0b00000111) << 18) |
				((str[1] & 0b00111111) << 12) |
				((str[2] & 0b00111111) << 6)  |
				(str[3] & 0b00111111);

			break;
		}

		default: return (ham_usize)-1;
	}

	return req_chars;
}

ham_constexpr ham_nothrow static inline ham_usize ham_str_next_codepoint_utf16(ham_utf_cp *cp, const ham_char16 *str, ham_usize max_chars){
	const ham_usize req_chars = ham_codepoint_num_chars_utf16(str, max_chars);
	if(req_chars == (ham_usize)-1) return req_chars;

	if(!cp || max_chars < req_chars) return (ham_usize)-1;

	switch(req_chars){
		case 0:{
			*cp = '\0';
			break;
		}

		case 1:{
			if(str[0] < 0) return (ham_usize)-1;
			*cp = str[0];
			break;
		}

		case 2:{
			const auto hi = str[0];
			const auto lo = str[1];
			*cp = (hi << 10) + (lo - 0x35fdc00);
			break;
		}

		default: return (ham_usize)-1;
	}

	return req_chars;
}

ham_constexpr ham_nothrow static inline ham_usize ham_str_next_codepoint_utf32(ham_utf_cp *cp, const ham_char32 *str, ham_usize max_chars){
	const ham_usize req_chars = ham_codepoint_num_chars_utf32(str, max_chars);
	if(req_chars == (ham_usize)-1) return req_chars;

	switch(req_chars){
		case 0:{
			*cp = '\0';
			break;
		}

		case 1:{
			*cp = str[0];
			break;
		}

		default: return (ham_usize)-1;
	}

	return req_chars;
}

ham_constexpr ham_nothrow static inline ham_usize ham_str_num_codepoints_utf8(ham_str8 str){
	if(!str.ptr || !str.len) return 0;

	ham_usize ret = 0;

	ham_usize i = 0;
	while(i < str.len){
		const ham_usize cp_nchars = ham_codepoint_num_chars_utf8(str.ptr + i, str.len - i);
		if(cp_nchars == (ham_usize)-1){
			return cp_nchars;
		}

		i += cp_nchars;
		++ret;
	}

	return ret;
}

ham_constexpr ham_nothrow static inline ham_usize ham_str_num_codepoints_utf16(ham_str16 str){
	if(!str.ptr || !str.len) return 0;

	ham_usize ret = 0;

	ham_usize i = 0;
	while(i < str.len){
		const ham_usize cp_nchars = ham_codepoint_num_chars_utf16(str.ptr + i, str.len - i);
		if(cp_nchars == (ham_usize)-1){
			return cp_nchars;
		}

		i += cp_nchars;
		++ret;
	}

	return ret;
}

ham_constexpr ham_nothrow static inline ham_usize ham_str_num_codepoints_utf32(ham_str32 str){
	return str.ptr ? str.len : 0;
}

ham_constexpr ham_nothrow static inline int  ham_str_cmp_utf8(ham_str8 a, ham_str8 b){ return HAM_IMPL_STR_CMP(a, b); }
ham_constexpr ham_nothrow static inline bool  ham_str_eq_utf8(ham_str8 a, ham_str8 b){ return HAM_IMPL_STR_EQ(a, b); }
ham_constexpr ham_nothrow static inline bool ham_str_neq_utf8(ham_str8 a, ham_str8 b){ return HAM_IMPL_STR_NEQ(a, b); }

ham_constexpr ham_nothrow static inline int  ham_str_cmp_utf16(ham_str16 a, ham_str16 b){ return HAM_IMPL_STR_CMP(a, b); }
ham_constexpr ham_nothrow static inline bool  ham_str_eq_utf16(ham_str16 a, ham_str16 b){ return HAM_IMPL_STR_EQ(a, b); }
ham_constexpr ham_nothrow static inline bool ham_str_neq_utf16(ham_str16 a, ham_str16 b){ return HAM_IMPL_STR_NEQ(a, b); }

ham_constexpr ham_nothrow static inline int  ham_str_cmp_utf32(ham_str32 a, ham_str32 b){ return HAM_IMPL_STR_CMP(a, b); }
ham_constexpr ham_nothrow static inline bool  ham_str_eq_utf32(ham_str32 a, ham_str32 b){ return HAM_IMPL_STR_EQ(a, b); }
ham_constexpr ham_nothrow static inline bool ham_str_neq_utf32(ham_str32 a, ham_str32 b){ return HAM_IMPL_STR_NEQ(a, b); }

ham_constexpr ham_nothrow static inline ham_uuid ham_str_to_uuid_utf8(ham_str8 uuid_str){
	if(!uuid_str.ptr || uuid_str.len >= HAM_UUID_BUFFER_SIZE) return (ham_uuid){ .u64s = { (ham_u64)-1, (ham_u64)-1 } };

	ham_uuid_buffer_utf8 uuid_buf;

	ham_usize written = 0;
	for(ham_usize i = 0; i < uuid_str.len; i++){
		const char c = uuid_str.ptr[i];
		if(c == '\0') break;

		if(ham_utf_is_alpha(c)){
			uuid_buf[written++] = ham_to_upper(c);
		}
		else if(ham_utf_is_digit(c)){
			uuid_buf[written++] = c;
		}
		else continue;
	}

	if(written != 32){
		return (ham_uuid){ .u64s = { (ham_u64)-1, (ham_u64)-1 } };
	}

	ham_uuid ret;

	for(ham_usize i = 0; i < 32; i++){
		const char c = uuid_buf[i];
		const ham_usize bit_off = (i * 4) % 64;
		const ham_usize u64_off = i / sizeof(ham_u64);
		ret.u64s[u64_off] |= ((ham_char_hex_value(c) & 0xF) << bit_off);
	}

	return ret;
}

ham_nothrow static inline ham_usize ham_str_conv_utf16_utf8(ham_str16 str, ham_char8 *buf, ham_usize buf_len){
	if(!buf || buf_len == 0){
		return (ham_usize)-1;
	}
	else if(!str.ptr || !str.len){
		buf[0] = '\0';
		return 0;
	}

	ham_usize written = 0;
	ham_char8 mb_buf[4];
	mbstate_t mbstate;
	memset(&mbstate, 0, sizeof(mbstate));

	for(ham_usize i = 0; i < str.len; i++){
		const ham_usize res = c16rtomb(mb_buf, str.ptr[i], &mbstate);
		if(res == (ham_usize)-1){
			// bad string
			return (ham_usize)-1;
		}
		else if(res > 0){
			const ham_usize total = written + res;
			if(total >= buf_len){
				// string too long
				return (ham_usize)-1;
			}

			memcpy(buf + written, mb_buf, res);
			written = total;
		}
	}

	buf[written] = '\0';
	return written;
}

ham_nothrow static inline ham_usize ham_str_conv_utf32_utf8(ham_str32 str, ham_char8 *buf, ham_usize buf_len){
	if(!buf || buf_len == 0){
		return (ham_usize)-1;
	}
	else if(!str.ptr || !str.len){
		buf[0] = '\0';
		return 0;
	}

	ham_usize written = 0;
	ham_char8 mb_buf[4];
	mbstate_t mbstate;
	memset(&mbstate, 0, sizeof(mbstate));

	for(ham_usize i = 0; i < str.len; i++){
		const ham_usize res = c32rtomb(mb_buf, str.ptr[i], &mbstate);
		if(res == (ham_usize)-1){
			// bad string
			return (ham_usize)-1;
		}
		else if(res > 0){
			const ham_usize total = written + res;
			if(total >= buf_len){
				// string too long
				return (ham_usize)-1;
			}

			memcpy(buf + written, mb_buf, res);
			written = total;
		}
	}

	buf[written] = '\0';
	return written;
}

#define HAM_STR_CMP_UTF(n) HAM_CONCAT(ham_str_cmp_utf, n)
#define HAM_STR_EQ_UTF(n)  HAM_CONCAT(ham_str_eq_utf, n)
#define HAM_STR_NEQ_UTF(n) HAM_CONCAT(ham_str_neq_utf, n)

#define ham_str_cmp HAM_STR_CMP_UTF(HAM_UTF)
#define ham_str_eq  HAM_STR_EQ_UTF(HAM_UTF)
#define ham_str_neq HAM_STR_NEQ_UTF(HAM_UTF)

#define HAM_STR_UTF(n)  HAM_CONCAT(ham_str, n)

#define HAM_EMPTY_STR_UTF(n) ((HAM_STR_UTF(n)){ ham_null, 0 })

#define HAM_EMPTY_STR8  HAM_EMPTY_STR_UTF(8)
#define HAM_EMPTY_STR16 HAM_EMPTY_STR_UTF(16)
#define HAM_EMPTY_STR32 HAM_EMPTY_STR_UTF(32)

#define HAM_LIT_C_UTF8(lit) (lit)
#define HAM_LIT_C_UTF16(lit) (HAM_CONCAT(u, lit))
#define HAM_LIT_C_UTF32(lit) (HAM_CONCAT(U, lit))

//! @cond ignore
#define HAM_IMPL_LIT_UTF(n, lit_prefixed) ((HAM_STR_UTF(n)){ (lit_prefixed), (sizeof(lit_prefixed)/sizeof(*(lit_prefixed)))-1 })
//! @endcond

#define HAM_LIT_UTF8(lit)  HAM_IMPL_LIT_UTF(8, lit)
#define HAM_LIT_UTF16(lit) HAM_IMPL_LIT_UTF(16, HAM_CONCAT(u, lit))
#define HAM_LIT_UTF32(lit) HAM_IMPL_LIT_UTF(32, HAM_CONCAT(U, lit))

#define HAM_LIT_UTF_(n, lit) HAM_LIT_UTF##n(lit)
#define HAM_LIT_UTF(n, lit) HAM_LIT_UTF_(n, lit)

#define HAM_LIT_C_UTF(n, lit) HAM_APPLY(HAM_CONCAT(HAM_LIT_C_UTF, n), lit)

#define HAM_LIT_C(lit) HAM_LIT_C_UTF(HAM_UTF, lit)
#define HAM_LIT(lit)   HAM_LIT_UTF(HAM_UTF, lit)

#define HAM_EMPTY_STR HAM_EMPTY_STR_UTF(HAM_UTF)

typedef HAM_STR_UTF(HAM_UTF)  ham_str;

/**
 * @}
 */

HAM_C_API_END

#ifdef __cplusplus

#include <ostream>
#include <type_traits>
#include <exception>

namespace ham{
	namespace typedefs{
		using unit = ham_unit;

		using char8  = ham_char8;
		using char16 = ham_char16;
		using char32 = ham_char32;

		using i8  = ham_i8;
		using u8  = ham_u8;
		using i16 = ham_i16;
		using u16 = ham_u16;
		using i16 = ham_i16;
		using u16 = ham_u16;
		using i32 = ham_i32;
		using u32 = ham_u32;
		using i64 = ham_i64;
		using u64 = ham_u64;

		using iptr = ham_iptr;
		using uptr = ham_uptr;

		using isize = ham_isize;
		using usize = ham_usize;

	#ifdef HAM_INT128
		using i128 = ham_i128;
		using u128 = ham_u128;
	#endif

	#ifdef HAM_FLOAT16
		using f16 = ham_f16;
	#endif

		using f32 = ham_f32;
		using f64 = ham_f64;

	#ifdef HAM_FLOAT128
		using f128 = ham_f128;
	#endif

		using utf_cp = ham_utf_cp;
		using uchar  = ham_uchar;
	}

	using namespace typedefs;

	class exception: public std::exception{
		public:
			virtual const char *api() const noexcept = 0;

		private:
			const char *m_api;
	};

	template<typename Handle, auto Destroyer, auto NullHandle = Handle(0)>
	class unique_handle{
		public:
			constexpr unique_handle(Handle handle_ = NullHandle) noexcept
				: m_handle(handle_){}

			constexpr unique_handle(unique_handle &&other) noexcept
				: m_handle(other.m_handle)
			{
				other.m_handle = NullHandle;
			}

			constexpr ~unique_handle(){
				if(m_handle != NullHandle){
					Destroyer(m_handle);
				}
			}

			constexpr operator bool() const noexcept{ return m_handle != NullHandle; }

			constexpr unique_handle &operator=(unique_handle &&other) noexcept{
				if(this != &other){
					if(m_handle != NullHandle) Destroyer(m_handle);
					m_handle = other.m_handle;
					other.m_handle = NullHandle;
				}

				return *this;
			}

			constexpr Handle get() const noexcept{ return m_handle; }

		private:
			Handle m_handle;
	};

	namespace meta{
		template<typename T>
		struct id{ using type = T; };

		template<typename T, T ... Vals>
		struct seq{};

		namespace detail{
			template<usize N, usize ... Built>
			struct index_seq_builder: index_seq_builder<N-1, Built..., N-1>{};

			template<usize ... Built>
			struct index_seq_builder<0, Built...>{ using type = seq<usize, Built...>; };
		}

		template<usize ... Is>
		using index_seq = seq<usize, Is...>;

		template<usize N>
		using make_index_seq = typename detail::index_seq_builder<N>::type;

		template<typename T, T Value>
		struct constant_value{
			using type = T;
			constexpr static T value = Value;
		};

		template<auto Fn, auto ... Args>
		struct constant_call: constant_value<decltype(Fn(Args...)), Fn(Args...)>{};

		template<auto Fn>
		struct static_fn{
			template<typename ... Args>
			constexpr auto operator()(Args &&... args) const noexcept(noexcept(Fn(std::forward<Args>(args)...))){
				return Fn(std::forward<Args>(args)...);
			}
		};
	}

	namespace detail{
		template<typename T> struct id{ using type = T; };

		template<
			typename Char,
			typename Utf8T,
			typename Utf16T,
			typename Utf32T
		>
		struct utf_conditional
			: std::conditional<
				std::is_same_v<Char, char32>, Utf32T,
				std::conditional_t<
					std::is_same_v<Char, char16>, Utf16T,
					Utf8T
				>
			>{};

		template<typename Char, typename Utf8T, typename Utf16T, typename Utf32T>
		using utf_conditional_t = typename utf_conditional<Char, Utf8T, Utf16T, Utf32T>::type;

		template<typename Char>
		struct str_ctype: utf_conditional<Char, ham_str8, ham_str16, ham_str32>{};

		template<typename Char>
		using str_ctype_t = typename str_ctype<Char>::type;

		template<typename Char>
		constexpr inline auto cstr_cmp = utf_conditional_t<
			Char,
			meta::static_fn<ham_str_cmp_utf8>,
			meta::static_fn<ham_str_cmp_utf16>,
			meta::static_fn<ham_str_cmp_utf32>
		>{};

		template<typename Char>
		constexpr inline auto cstr_eq = utf_conditional_t<
			Char,
			meta::static_fn<ham_str_eq_utf8>,
			meta::static_fn<ham_str_eq_utf16>,
			meta::static_fn<ham_str_eq_utf32>
		>{};

		template<typename Char>
		constexpr inline auto cstr_neq = utf_conditional_t<
			Char,
			meta::static_fn<ham_str_neq_utf8>,
			meta::static_fn<ham_str_neq_utf16>,
			meta::static_fn<ham_str_neq_utf32>
		>{};

		template<typename Char>
		constexpr inline auto cstr_next_codepoint = utf_conditional_t<
			Char,
			meta::static_fn<ham_str_next_codepoint_utf8>,
			meta::static_fn<ham_str_next_codepoint_utf16>,
			meta::static_fn<ham_str_next_codepoint_utf32>
		>{};

		template<typename Char>
		constexpr inline auto ccodepoint_num_chars = utf_conditional_t<
			Char,
			meta::static_fn<ham_codepoint_num_chars_utf8>,
			meta::static_fn<ham_codepoint_num_chars_utf16>,
			meta::static_fn<ham_codepoint_num_chars_utf32>
		>{};

		template<typename Char>
		constexpr inline auto cstr_num_codepoints = utf_conditional_t<
			Char,
			meta::static_fn<ham_str_num_codepoints_utf8>,
			meta::static_fn<ham_str_num_codepoints_utf16>,
			meta::static_fn<ham_str_num_codepoints_utf32>
		>{};

		template<typename Char>
		constexpr inline usize strlen_utf(const Char *str) noexcept{
			if(!str) return (usize)-1;
			usize count = 0;
			while(str[count]) ++count;
			return count;
		}
	}

	template<typename T, typename U>
	struct layout_is_same: std::bool_constant<
		(std::is_standard_layout_v<T> && std::is_standard_layout_v<U>) &&
		(alignof(T) == alignof(U)) &&
		(sizeof(T) == sizeof(U))
	>{};

	template<typename T, typename U>
	constexpr inline bool layout_is_same_v = layout_is_same<T, U>::value;

	template<typename...>
	struct type_tag{};

	template<typename T>
	constexpr inline type_tag<T> type_tag_v;

	template<typename Char>
	class basic_str{
		public:
			using ctype = detail::str_ctype_t<Char>;

			using iterator = const Char*;

			constexpr static usize npos = (usize)-1;

			constexpr basic_str() noexcept: m_val{ nullptr, 0 }{}

			constexpr basic_str(const Char *ptr, usize len) noexcept: m_val{ ptr, len }{}
			constexpr basic_str(const ctype &str_) noexcept: m_val(str_){}

			constexpr explicit basic_str(const Char *c_str) noexcept: m_val{ c_str, detail::strlen_utf<Char>(c_str) }{}

			template<usize N>
			constexpr basic_str(const Char(&lit)[N]) noexcept: m_val{ lit, N-1 }{}

			constexpr basic_str(const basic_str&) noexcept = default;

			constexpr operator const ctype&() const noexcept{ return m_val; }

			constexpr basic_str &operator=(const basic_str&) noexcept = default;

			constexpr const Char &operator[](usize idx) const noexcept{ return m_val.ptr[idx]; }

			constexpr int compare(const basic_str &other) const noexcept{ return detail::cstr_cmp<Char>(m_val, other.m_val); }

			constexpr bool operator==(const basic_str &other) const noexcept{ return detail::cstr_eq<Char> (m_val, other.m_val); }
			constexpr bool operator!=(const basic_str &other) const noexcept{ return detail::cstr_neq<Char>(m_val, other.m_val); }
			constexpr bool operator< (const basic_str &other) const noexcept{ return compare(other) <  0; }
			constexpr bool operator<=(const basic_str &other) const noexcept{ return compare(other) <= 0; }
			constexpr bool operator> (const basic_str &other) const noexcept{ return compare(other) >  0; }
			constexpr bool operator>=(const basic_str &other) const noexcept{ return compare(other) >= 0; }

			constexpr bool is_empty() const noexcept{ return m_val.len == 0; }

			constexpr usize num_codepoints() const noexcept{ return detail::cstr_num_codepoints<Char>(m_val); }

			constexpr usize len() const noexcept{ return m_val.len; }
			constexpr usize size() const noexcept{ return len(); }
			constexpr usize length() const noexcept{ return len(); }

			constexpr const Char *ptr() const noexcept{ return m_val.ptr; }
			constexpr const Char *data() const noexcept{ return ptr(); }

			constexpr iterator begin() const noexcept{ return ptr(); }
			constexpr iterator end() const noexcept{ return ptr() + len(); }

			constexpr usize find(const basic_str &str_, usize pos = 0) const noexcept{
				if(str_.len() == 0) return pos;
				else if(str_.len() > len()) return npos;

				while(pos < len() && (len() - pos >= str_.len())){
					const auto sub = basic_str(m_val.ptr + pos, str_.len());
					if(sub == str_) return pos;
					++pos;
				}

				return npos;
			}

			constexpr usize rfind(const basic_str &str_, usize pos = HAM_USIZE_MAX) const noexcept{
				if(str_.len() == 0) return pos;
				else if(str_.len() > len()) return pos;

				pos = ham_min(pos, len() - str_.len());

				while(pos != npos){
					const auto sub = basic_str(m_val.ptr + pos, str_.len());
					if(sub == str_) return pos;
					--pos;
				}

				return npos;
			}

			constexpr basic_str substr(usize req_from, usize req_len = HAM_USIZE_MAX) const noexcept{
				const usize actual_from = ham_min(req_from, len());
				const usize actual_len = ham_min(req_len, len() - actual_from);
				return basic_str(ptr() + actual_from, actual_len);
			}

		// breaks constexpr
		//private:
			ctype m_val;
	};

	class uuid{
		public:
			constexpr uuid() noexcept = default;

			constexpr uuid(ham_uuid val_) noexcept: m_val(val_){}

			constexpr uuid(const uuid&) noexcept = default;

			constexpr explicit uuid(const basic_str<char8> &uuid_str) noexcept
				: m_val(ham_str_to_uuid_utf8(uuid_str)){}

			constexpr operator ham_uuid&() &noexcept{ return m_val; }
			constexpr operator const ham_uuid&() const& noexcept{ return m_val; }

			constexpr uuid &operator=(const uuid&) noexcept = default;

			constexpr bool operator==(const uuid &other) const noexcept{ return ham_uuid_eq (m_val, other.m_val); }
			constexpr bool operator!=(const uuid &other) const noexcept{ return ham_uuid_neq(m_val, other.m_val); }
			constexpr bool operator< (const uuid &other) const noexcept{ return ham_uuid_lt (m_val, other.m_val); }
			constexpr bool operator<=(const uuid &other) const noexcept{ return ham_uuid_lte(m_val, other.m_val); }
			constexpr bool operator> (const uuid &other) const noexcept{ return ham_uuid_gt (m_val, other.m_val); }
			constexpr bool operator>=(const uuid &other) const noexcept{ return ham_uuid_gte(m_val, other.m_val); }

		//private:
			ham_uuid m_val;
	};

	basic_str(const ham_str8&)  -> basic_str<char8>;
	basic_str(const ham_str16&) -> basic_str<char16>;
	basic_str(const ham_str32&) -> basic_str<char32>;

	template<typename Char, usize N>
	basic_str(const Char(&lit)[N]) -> basic_str<Char>;

	template<typename Char>
	basic_str(const Char*) -> basic_str<Char>;

	using str8  = basic_str<char8>;
	using str16 = basic_str<char16>;
	using str32 = basic_str<char32>;

	static_assert(layout_is_same_v<str8,  ham_str8>);
	static_assert(layout_is_same_v<str16, ham_str16>);
	static_assert(layout_is_same_v<str32, ham_str32>);

	using str = basic_str<uchar>;

	namespace str_literals{
		constexpr inline auto operator""_str(const char8  *ptr, usize len){ return str8 (ptr, len); }
		constexpr inline auto operator""_str(const char16 *ptr, usize len){ return str16(ptr, len); }
		constexpr inline auto operator""_str(const char32 *ptr, usize len){ return str32(ptr, len); }
	}

	namespace uuid_literals{
		constexpr inline uuid operator""_uuid(const char8 *ptr, usize len){ return uuid(str8(ptr, len)); }
	}

	namespace literals{
		using namespace str_literals;
		using namespace uuid_literals;
	}

	using namespace literals;

	namespace meta{
		// pretty much useless
		template<unit Value> using constant_unit = constant_value<unit, Value>;

		template<bool Value> using constant_bool = constant_value<bool, Value>;

		template<i8  Value> using constant_i8  = constant_value<i8,  Value>;
		template<u8  Value> using constant_u8  = constant_value<u8,  Value>;
		template<i16 Value> using constant_i16 = constant_value<i16, Value>;
		template<u16 Value> using constant_u16 = constant_value<u16, Value>;
		template<i32 Value> using constant_i32 = constant_value<i32, Value>;
		template<u32 Value> using constant_u32 = constant_value<u32, Value>;
		template<i64 Value> using constant_i64 = constant_value<i64, Value>;
		template<u64 Value> using constant_u64 = constant_value<u64, Value>;

	#ifdef HAM_INT128
		template<i128 Value> using constant_i128 = constant_value<i128, Value>;
		template<u128 Value> using constant_u128 = constant_value<u128, Value>;
	#endif

		template<iptr Value> using constant_iptr = constant_value<iptr, Value>;
		template<uptr Value> using constant_uptr = constant_value<uptr, Value>;

		template<isize Value> using constant_isize = constant_value<isize, Value>;
		template<usize Value> using constant_usize = constant_value<usize, Value>;

	#ifdef HAM_FLOAT16
		template<f16 Value> using constant_f16 = constant_value<f16, Value>;
	#endif

		template<f32 Value> using constant_f32 = constant_value<f32, Value>;
		template<f64 Value> using constant_f64 = constant_value<f64, Value>;

	#ifdef HAM_FLOAT128
		template<f128 Value> using constant_f128 = constant_value<f128, Value>;
	#endif

		template<uuid  Value> using constant_uuid  = constant_value<uuid,  Value>;
		template<str8  Value> using constant_str8  = constant_value<str8,  Value>;
		template<str16 Value> using constant_str16 = constant_value<str16, Value>;
		template<str32 Value> using constant_str32 = constant_value<str32, Value>;
	}
}

template<typename Char>
inline std::basic_ostream<Char> &operator<<(std::basic_ostream<Char> &stream, const ham::basic_str<Char> &str){
	return stream.write(str.ptr(), str.len());
}

template<typename Char>
inline std::basic_ostream<Char> &operator<<(std::basic_ostream<Char> &stream, const ham::detail::str_ctype_t<Char> &str){
	return stream.write(str.ptr, str.len);
}

#endif // __cplusplus

/**
 * @}
 */

#endif // !HAM_TYPEDEFS_H
