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

#define ham_hash32 ham_hash_fnv1a_32
#define ham_hash64 ham_hash_fnv1a_64

#define ham_hash ham_hash64

ham_constexpr static inline ham_uptr ham_str_hash_utf8(ham_str8 str){ return ham_hash(str.ptr, str.len); }
static inline ham_uptr ham_str_hash_utf16(ham_str16 str){ return ham_hash((const char*)str.ptr, str.len * sizeof(ham_char16)); }
static inline ham_uptr ham_str_hash_utf32(ham_str32 str){ return ham_hash((const char*)str.ptr, str.len * sizeof(ham_char32)); }

#define HAM_STR_HASH_UTF(n) HAM_CONCAT(ham_str_hash_utf, n)

#define ham_str_hash HAM_STR_HASH_UTF(HAM_UTF)

#define HAM_HASHED(lit_) (ham_str_hash(HAM_TEXT(lit_)))

HAM_C_API_END

#ifdef __cplusplus

namespace ham{
	namespace hash_literals{
		constexpr static inline uptr operator""_hash(const char8 *ptr, uptr len) noexcept{
			return ham_str_hash_utf8((ham_str8){ ptr, len });
		}

		static inline uptr operator""_hash(const char16 *ptr, uptr len) noexcept{
			return ham_str_hash_utf16((ham_str16){ ptr, len });
		}

		static inline uptr operator""_hash(const char32 *ptr, uptr len) noexcept{
			return ham_str_hash_utf32((ham_str32){ ptr, len });
		}
	}

	using namespace hash_literals;

	template<typename T>
	struct hash_functor;

	template<>
	struct hash_functor<str8>{
		constexpr ham_uptr operator()(const str8 &s) const noexcept{
			return ham_str_hash_utf8(s);
		}
	};

	template<>
	struct hash_functor<str16>{
		ham_uptr operator()(const str16 &s) const noexcept{
			return ham_str_hash_utf16(s);
		}
	};

	template<>
	struct hash_functor<str32>{
		ham_uptr operator()(const str32 &s) const noexcept{
			return ham_str_hash_utf32(s);
		}
	};

	template<> struct hash_functor<ham_str8>: hash_functor<str8>{};
	template<> struct hash_functor<ham_str16>: hash_functor<str16>{};
	template<> struct hash_functor<ham_str32>: hash_functor<str32>{};

	template<typename T>
	constexpr static inline ham_uptr hash(const T &val) noexcept{
		constexpr hash_functor<T> functor;
		return functor(val);
	}
}

template<>
struct std::hash<ham::str8>{
	constexpr auto operator()(ham::str8 s) const noexcept{ return ham::hash(s); }
};

template<>
struct std::hash<ham::str16>{
	auto operator()(ham::str16 s) const noexcept{ return ham::hash(s); }
};

template<>
struct std::hash<ham::str32>{
	auto operator()(ham::str32 s) const noexcept{ return ham::hash(s); }
};

#endif

/**
 * @}
 */

#endif // !HAM_HASH_H
