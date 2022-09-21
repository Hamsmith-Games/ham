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

#ifndef HAM_HASH_H
#define HAM_HASH_H 1

/**
 * @defgroup HAM_HASH Hashing
 * @ingroup HAM
 * @{
 */

#include "typedefs.h"

HAM_C_API_BEGIN

#define HAM_FNV1A_OFFSET_BIAS_32 0x811c9dc5
#define HAM_FNV1A_OFFSET_BIAS_64 0xcbf29ce484222325

#define HAM_FNV1A_PRIME_32 0x01000193
#define HAM_FNV1A_PRIME_64 0x811c9dc5

//
// Byte hashing functions
//

ham_constexpr static inline ham_u32 ham_hash_fnv1a_32(const char *bytes, ham_usize len){
	ham_u32 hash = HAM_FNV1A_OFFSET_BIAS_32;

	for(const char *it = bytes; it != (bytes + len); ++it){
		hash ^= *it;
		hash *= HAM_FNV1A_PRIME_32;
	}

	return hash;
}

ham_constexpr static inline ham_u64 ham_hash_fnv1a_64(const char *bytes, ham_usize len){
	ham_u64 hash = HAM_FNV1A_OFFSET_BIAS_64;

	for(const char *it = bytes; it != (bytes + len); ++it){
		hash ^= *it;
		hash *= HAM_FNV1A_PRIME_64;
	}

	return hash;
}

//
// UUID hashing functions
//

ham_constexpr static inline ham_u32 ham_uuid_hash32(ham_uuid uuid){ return ham_hash_fnv1a_32(uuid.bytes, 16); }
ham_constexpr static inline ham_u64 ham_uuid_hash64(ham_uuid uuid){ return ham_hash_fnv1a_64(uuid.bytes, 16); }

#define ham_uuid_hash ham_uuid_hash64

//
// String hashing functions
//

ham_constexpr static inline ham_u32 ham_str_hash32_utf8(ham_str8 str){
	return ham_hash_fnv1a_32(str.ptr, str.len);
}

ham_constexpr static inline ham_u32 ham_str_hash32_utf16(ham_str16 str){
	ham_u32 hash = HAM_FNV1A_OFFSET_BIAS_32;

	for(ham_usize i = 0 ; i < str.len; i++){
		const ham_char16 c = str.ptr[i];
		const char cs[2] = {
			(char)(c),
			(char)(c >> 8),
		};

		for(ham_usize j = 0; j < 2; j++){
			hash ^= cs[j];
			hash *= HAM_FNV1A_PRIME_32;
		}
	}

	return hash;
}

ham_constexpr static inline ham_u32 ham_str_hash32_utf32(ham_str32 str){
	ham_u32 hash = HAM_FNV1A_OFFSET_BIAS_32;

	for(ham_usize i = 0 ; i < str.len; i++){
		const ham_char32 c = str.ptr[i];
		const char cs[4] = {
			(char)(c),
			(char)(c >> 8),
			(char)(c >> 16),
			(char)(c >> 24),
		};

		for(ham_usize j = 0; j < 4; j++){
			hash ^= cs[j];
			hash *= HAM_FNV1A_PRIME_32;
		}
	}

	return hash;
}

ham_constexpr static inline ham_u64 ham_str_hash64_utf8(ham_str8 str){
	return ham_hash_fnv1a_64(str.ptr, str.len);
}

ham_constexpr static inline ham_u64 ham_str_hash64_utf16(ham_str16 str){
	ham_u64 hash = HAM_FNV1A_OFFSET_BIAS_64;

	for(ham_usize i = 0 ; i < str.len; i++){
		const ham_char16 c = str.ptr[i];
		const char cs[2] = {
			(char)(c),
			(char)(c >> 8),
		};

		for(ham_usize j = 0; j < 2; j++){
			hash ^= cs[j];
			hash *= HAM_FNV1A_PRIME_64;
		}
	}

	return hash;
}

ham_constexpr static inline ham_u64 ham_str_hash64_utf32(ham_str32 str){
	ham_u64 hash = HAM_FNV1A_OFFSET_BIAS_64;

	for(ham_usize i = 0 ; i < str.len; i++){
		const ham_char32 c = str.ptr[i];
		const char cs[4] = {
			(char)(c),
			(char)(c >> 8),
			(char)(c >> 16),
			(char)(c >> 24),
		};

		for(ham_usize j = 0; j < 4; j++){
			hash ^= cs[j];
			hash *= HAM_FNV1A_PRIME_64;
		}
	}

	return hash;
}

#define ham_hash32 ham_hash_fnv1a_32
#define ham_hash64 ham_hash_fnv1a_64

#define ham_hash ham_hash64

#define HAM_STR_HASH_UTF(n) HAM_CONCAT(ham_str_hash64_utf, n)

#define ham_str_hash_utf8  HAM_STR_HASH_UTF(8)
#define ham_str_hash_utf16 HAM_STR_HASH_UTF(16)
#define ham_str_hash_utf32 HAM_STR_HASH_UTF(32)

#define ham_str_hash HAM_STR_HASH_UTF(HAM_UTF)

#define HAM_HASHED(lit_) (ham_str_hash(HAM_TEXT(lit_)))

HAM_C_API_END

#ifdef __cplusplus

namespace ham{
	namespace hash_literals{
		constexpr static inline uptr operator""_hash(const char8 *ptr, uptr len) noexcept{
			return ham_str_hash_utf8((ham_str8){ ptr, len });
		}

		constexpr static inline uptr operator""_hash(const char16 *ptr, uptr len) noexcept{
			return ham_str_hash_utf16((ham_str16){ ptr, len });
		}

		constexpr static inline uptr operator""_hash(const char32 *ptr, uptr len) noexcept{
			return ham_str_hash_utf32((ham_str32){ ptr, len });
		}
	}

	namespace literals{
		using namespace hash_literals;
	}

	template<typename T>
	struct hash_functor;

	template<>
	struct hash_functor<str8>{
		constexpr ham_uptr operator()(const str8 &s) const noexcept{ return ham_str_hash_utf8(s); }
	};

	template<>
	struct hash_functor<str16>{
		constexpr ham_uptr operator()(const str16 &s) const noexcept{ return ham_str_hash_utf16(s); }
	};

	template<>
	struct hash_functor<str32>{
		constexpr ham_uptr operator()(const str32 &s) const noexcept{ return ham_str_hash_utf32(s); }
	};

	template<>
	struct hash_functor<uuid>{
		constexpr ham_uptr operator()(const uuid &uuid_) const noexcept{ return ham_uuid_hash(uuid_); }
	};

	template<> struct hash_functor<ham_str8>: hash_functor<str8>{};
	template<> struct hash_functor<ham_str16>: hash_functor<str16>{};
	template<> struct hash_functor<ham_str32>: hash_functor<str32>{};

	template<> struct hash_functor<ham_uuid>: hash_functor<uuid>{};

	template<typename T>
	constexpr static inline ham_uptr hash(const T &val) noexcept{
		constexpr hash_functor<T> functor;
		return functor(val);
	}
}

template<>
struct std::hash<ham::str8>: ham::hash_functor<ham::str8>{};

template<>
struct std::hash<ham::str16>: ham::hash_functor<ham::str16>{};

template<>
struct std::hash<ham::str32>: ham::hash_functor<ham::str32>{};

template<>
struct std::hash<ham::uuid>: ham::hash_functor<ham::uuid>{};

template<>
struct std::hash<ham_str8>: ham::hash_functor<ham_str8>{};

template<>
struct std::hash<ham_str16>: ham::hash_functor<ham_str16>{};

template<>
struct std::hash<ham_str32>: ham::hash_functor<ham_str32>{};

template<>
struct std::hash<ham_uuid>: ham::hash_functor<ham_uuid>{};

#endif

/**
 * @}
 */

#endif // !HAM_HASH_H
