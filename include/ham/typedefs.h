#ifndef HAM_TYPEDEFS_H
#define HAM_TYPEDEFS_H 1

/**
 * @defgroup HAM_TYPEDEFS Typedefs
 * @ingroup HAM
 * @{
 */

#include "config.h"

#include <stdint.h>

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

typedef ham_u32 ham_utf_cp;

HAM_C_API_BEGIN

typedef struct ham_str8 { const ham_char8  *ptr; ham_uptr len; } ham_str8;
typedef struct ham_str16{ const ham_char16 *ptr; ham_uptr len; } ham_str16;
typedef struct ham_str32{ const ham_char32 *ptr; ham_uptr len; } ham_str32;

HAM_C_API_END

#define HAM_CHAR_UTF_(n) ham_char##n
#define HAM_CHAR_UTF(n) HAM_CHAR_UTF_(n)

#define HAM_STR_UTF_(n) ham_str##n
#define HAM_STR_UTF(n) HAM_STR_UTF_(n)

#define HAM_LIT_C_UTF8(lit) (lit)
#define HAM_LIT_C_UTF16(lit) (HAM_CONCAT_(u, lit))
#define HAM_LIT_C_UTF32(lit) (HAM_CONCAT_(U, lit))

#define HAM_LIT_UTF8(lit) ((HAM_STR_UTF_(8)){ (lit), (sizeof(lit)-1) })
#define HAM_LIT_UTF16(lit) ((HAM_STR_UTF_(16)){ (u##lit), ((sizeof(u##lit)/sizeof(*(u##lit)))-1) })
#define HAM_LIT_UTF32(lit) ((HAM_STR_UTF_(32)){ (U##lit), ((sizeof(U##lit)/sizeof(*(U##lit)))-1) })

#define HAM_LIT_UTF_(n, lit) HAM_LIT_UTF##n(lit)
#define HAM_LIT_UTF(n, lit) HAM_LIT_UTF_(n, lit)

#define HAM_LIT_C_UTF__(macro_n, lit) macro_n(lit)
#define HAM_LIT_C_UTF_(n, lit) HAM_LIT_C_UTF__(HAM_LIT_C_UTF##n, lit)
#define HAM_LIT_C_UTF(n, lit) HAM_LIT_C_UTF_(n, lit)

#define HAM_LIT_C(lit) HAM_LIT_C_UTF(HAM_UTF, lit)
#define HAM_LIT(lit) HAM_LIT_UTF(HAM_UTF, lit)

typedef HAM_CHAR_UTF(HAM_UTF) ham_uchar;
typedef HAM_STR_UTF(HAM_UTF) ham_str;

#ifdef __cplusplus

#include <ostream>
#include <type_traits>

namespace ham{
	namespace typedefs{
		typedef ham_char8  char8;
		typedef ham_char16 char16;
		typedef ham_char32 char32;

		typedef ham_i8 i8;
		typedef ham_u8 u8;
		typedef ham_i16 i16;
		typedef ham_u16 u16;
		typedef ham_i32 i32;
		typedef ham_u32 u32;
		typedef ham_i64 i64;
		typedef ham_u64 u64;

		typedef ham_iptr iptr;
		typedef ham_uptr uptr;

		typedef ham_isize isize;
		typedef ham_usize usize;

		typedef ham_utf_cp utf_cp;

		typedef ham_uchar uchar;
	}

	using namespace typedefs;

	namespace detail{
		template<typename T> struct id{ using type = T; };

		template<typename Char> struct str_ctype;
		template<> struct str_ctype<ham_char8>:  id<ham_str8>{};
		template<> struct str_ctype<ham_char16>: id<ham_str16>{};
		template<> struct str_ctype<ham_char32>: id<ham_str32>{};

		template<typename Char>
		using str_ctype_t = typename str_ctype<Char>::type;
	}

	template<typename>
	struct type_tag{};

	template<typename T>
	constexpr inline type_tag<T> type_tag_v;

	template<typename Char>
	class basic_str{
		public:
			using ctype = detail::str_ctype_t<Char>;

			using iterator = const Char*;

			constexpr basic_str() noexcept: m_val{ nullptr, 0 }{}

			constexpr basic_str(const Char *ptr, usize len) noexcept: m_val{ ptr, len }{}
			constexpr basic_str(const ctype &str_) noexcept: m_val(str_){}

			explicit basic_str(const Char *c_str) noexcept: m_val{ c_str, strlen(c_str) }{}

			template<usize N>
			constexpr basic_str(const Char(&lit)[N]) noexcept: m_val{ lit, N-1 }{}

			constexpr basic_str(const basic_str&) noexcept = default;

			constexpr operator const ctype&() const noexcept{ return m_val; }

			constexpr basic_str &operator=(const basic_str&) noexcept = default;

			constexpr int compare(const basic_str &other) const noexcept{ return ham_str_cmp(m_val, other.m_val); }

			constexpr bool operator==(const basic_str &other) const noexcept{ return ham_str_eq (m_val, other.m_val); }
			constexpr bool operator!=(const basic_str &other) const noexcept{ return ham_str_neq(m_val, other.m_val); }
			constexpr bool operator< (const basic_str &other) const noexcept{ return compare(other) <  0; }
			constexpr bool operator<=(const basic_str &other) const noexcept{ return compare(other) <= 0; }
			constexpr bool operator> (const basic_str &other) const noexcept{ return compare(other) >  0; }
			constexpr bool operator>=(const basic_str &other) const noexcept{ return compare(other) >= 0; }

			constexpr usize len() const noexcept{ return m_val.len; }
			constexpr usize size() const noexcept{ return len(); }
			constexpr usize length() const noexcept{ return len(); }

			constexpr const Char *ptr() const noexcept{ return m_val.ptr; }
			constexpr const Char *data() const noexcept{ return ptr(); }

			constexpr iterator begin() const noexcept{ return ptr(); }
			constexpr iterator end() const noexcept{ return ptr() + len(); }

			constexpr basic_str substr(usize req_from, usize req_len = HAM_USIZE_MAX) const noexcept{
				const usize actual_from = gpwe_min(req_from, len());
				const usize actual_len = gpwe_min(req_len, len() - actual_from);
				return basic_str(ptr() + actual_from, actual_len);
			}

		private:
			ctype m_val;
	};

	using str8  = basic_str<char8>;
	using str16 = basic_str<char16>;
	using str32 = basic_str<char32>;

	using str = basic_str<uchar>;
}

template<typename Char>
std::basic_ostream<Char> &operator<<(std::basic_ostream<Char> &stream, ham::basic_str<Char> str){
	return stream.write(str.ptr(), str.len());
}

#endif // __cplusplus

/**
 * @}
 */

#endif // !HAM_TYPEDEFS_H
