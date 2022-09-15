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

typedef ham_char8  ham_name_buffer_utf8[HAM_NAME_BUFFER_SIZE];
typedef ham_char16 ham_name_buffer_utf16[HAM_NAME_BUFFER_SIZE];
typedef ham_char32 ham_name_buffer_utf32[HAM_NAME_BUFFER_SIZE];

typedef ham_char8  ham_path_buffer_utf8[HAM_PATH_BUFFER_SIZE];
typedef ham_char16 ham_path_buffer_utf16[HAM_PATH_BUFFER_SIZE];
typedef ham_char32 ham_path_buffer_utf32[HAM_PATH_BUFFER_SIZE];

typedef ham_char8  ham_message_buffer_utf8[HAM_MESSAGE_BUFFER_SIZE];
typedef ham_char16 ham_message_buffer_utf16[HAM_MESSAGE_BUFFER_SIZE];
typedef ham_char32 ham_message_buffer_utf32[HAM_MESSAGE_BUFFER_SIZE];

#define HAM_NAME_BUFFER_UTF(n) HAM_CONCAT(ham_name_buffer_utf, n)
#define HAM_PATH_BUFFER_UTF(n) HAM_CONCAT(ham_path_buffer_utf, n)
#define HAM_MESSAGE_BUFFER_UTF(n) HAM_CONCAT(ham_message_buffer_utf, n)

HAM_C_API_BEGIN

typedef struct ham_str8 { const ham_char8  *ptr; ham_uptr len; } ham_str8;
typedef struct ham_str16{ const ham_char16 *ptr; ham_uptr len; } ham_str16;
typedef struct ham_str32{ const ham_char32 *ptr; ham_uptr len; } ham_str32;

//! @cond ignore

#ifdef __GNUC__

#	define HAM_IMPL_STR_CMP(a, b) \
		({	const ham_auto a_ = (a); const ham_auto b_ = (b);\
			const ham_usize max_len_ = a_.len > b_.len ? b_.len : a_.len; \
			ham_usize i_ = 0; \
			int res_ = 0; \
			for(; i_ < max_len_; i_++){ \
				res_ = a_.ptr[i_] - b_.ptr[i_]; \
				if(res_ != 0) break;\
			} \
			if(res_ != 0){ \
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

ham_constexpr static inline int  ham_str_cmp_utf8(ham_str8 a, ham_str8 b){ return HAM_IMPL_STR_CMP(a, b); }
ham_constexpr static inline bool  ham_str_eq_utf8(ham_str8 a, ham_str8 b){ return HAM_IMPL_STR_EQ(a, b); }
ham_constexpr static inline bool ham_str_neq_utf8(ham_str8 a, ham_str8 b){ return HAM_IMPL_STR_NEQ(a, b); }

ham_constexpr static inline int  ham_str_cmp_utf16(ham_str16 a, ham_str16 b){ return HAM_IMPL_STR_CMP(a, b); }
ham_constexpr static inline bool  ham_str_eq_utf16(ham_str16 a, ham_str16 b){ return HAM_IMPL_STR_EQ(a, b); }
ham_constexpr static inline bool ham_str_neq_utf16(ham_str16 a, ham_str16 b){ return HAM_IMPL_STR_NEQ(a, b); }

ham_constexpr static inline int  ham_str_cmp_utf32(ham_str32 a, ham_str32 b){ return HAM_IMPL_STR_CMP(a, b); }
ham_constexpr static inline bool  ham_str_eq_utf32(ham_str32 a, ham_str32 b){ return HAM_IMPL_STR_EQ(a, b); }
ham_constexpr static inline bool ham_str_neq_utf32(ham_str32 a, ham_str32 b){ return HAM_IMPL_STR_NEQ(a, b); }

#define HAM_STR_CMP_UTF(n) HAM_CONCAT(ham_str_cmp_utf, n)
#define HAM_STR_EQ_UTF(n) HAM_CONCAT(ham_str_eq_utf, n)
#define HAM_STR_NEQ_UTF(n) HAM_CONCAT(ham_str_neq_utf, n)

#define ham_str_cmp HAM_STR_CMP_UTF(HAM_UTF)
#define ham_str_eq HAM_STR_EQ_UTF(HAM_UTF)
#define ham_str_neq HAM_STR_NEQ_UTF(HAM_UTF)

HAM_C_API_END

#define HAM_CHAR_UTF_(n) ham_char##n
#define HAM_CHAR_UTF(n) HAM_CHAR_UTF_(n)

#define HAM_STR_UTF_(n) ham_str##n
#define HAM_STR_UTF(n) HAM_STR_UTF_(n)

#define HAM_EMPTY_STR_UTF(n) ((HAM_CONCAT(ham_str, n)){ ham_null, 0 })

#define HAM_EMPTY_STR8  HAM_EMPTY_STR_UTF(8)
#define HAM_EMPTY_STR16 HAM_EMPTY_STR_UTF(16)
#define HAM_EMPTY_STR32 HAM_EMPTY_STR_UTF(32)

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

#define HAM_EMPTY_STR HAM_EMPTY_STR_UTF(HAM_UTF)

typedef HAM_CHAR_UTF(HAM_UTF) ham_uchar;
typedef HAM_STR_UTF(HAM_UTF)  ham_str;

typedef HAM_NAME_BUFFER_UTF(HAM_UTF)    ham_name_buffer;
typedef HAM_PATH_BUFFER_UTF(HAM_UTF)    ham_path_buffer;
typedef HAM_MESSAGE_BUFFER_UTF(HAM_UTF) ham_message_buffer;

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

		constexpr inline int cstr_cmp(const ham_str8 &a, const ham_str8 &b){ return ham_str_cmp_utf8(a, b); }
		constexpr inline int cstr_eq (const ham_str8 &a, const ham_str8 &b){ return  ham_str_eq_utf8(a, b); }
		constexpr inline int cstr_neq(const ham_str8 &a, const ham_str8 &b){ return ham_str_neq_utf8(a, b); }

		constexpr inline int cstr_cmp(const ham_str16 &a, const ham_str16 &b){ return ham_str_cmp_utf16(a, b); }
		constexpr inline int cstr_eq (const ham_str16 &a, const ham_str16 &b){ return  ham_str_eq_utf16(a, b); }
		constexpr inline int cstr_neq(const ham_str16 &a, const ham_str16 &b){ return ham_str_neq_utf16(a, b); }

		constexpr inline int cstr_cmp(const ham_str32 &a, const ham_str32 &b){ return ham_str_cmp_utf32(a, b); }
		constexpr inline int cstr_eq (const ham_str32 &a, const ham_str32 &b){ return  ham_str_eq_utf32(a, b); }
		constexpr inline int cstr_neq(const ham_str32 &a, const ham_str32 &b){ return ham_str_neq_utf32(a, b); }
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

			constexpr int compare(const basic_str &other) const noexcept{ return detail::cstr_cmp(m_val, other.m_val); }

			constexpr bool operator==(const basic_str &other) const noexcept{ return detail::cstr_eq (m_val, other.m_val); }
			constexpr bool operator!=(const basic_str &other) const noexcept{ return detail::cstr_neq(m_val, other.m_val); }
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
				const usize actual_from = ham_min(req_from, len());
				const usize actual_len = ham_min(req_len, len() - actual_from);
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
