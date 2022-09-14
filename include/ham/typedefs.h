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

typedef ham_u32 ham_utf_cp;

typedef struct ham_str8 { const ham_char8  *ptr; ham_uptr len; } ham_str8;
typedef struct ham_str16{ const ham_char16 *ptr; ham_uptr len; } ham_str16;
typedef struct ham_str32{ const ham_char32 *ptr; ham_uptr len; } ham_str32;

#define HAM_CHAR_UTF_(n) ham_char##n
#define HAM_CHAR_UTF(n) HAM_CHAR_UTF_(n)

#define HAM_STR_UTF_(n) ham_str##n
#define HAM_STR_UTF(n) HAM_STR_UTF_(n)

#define HAM_LIT_C_UTF8(lit) (lit)
#define HAM_LIT_C_UTF16(lit) (HAM_CONCAT(u, lit))
#define HAM_LIT_C_UTF32(lit) (HAM_CONCAT(U, lit))

#define HAM_LIT_UTF8(lit) ((HAM_STR_UTF_(8)){ (lit), (sizeof(lit)-1) })
#define HAM_LIT_UTF16(lit) ((HAM_STR_UTF_(16)){ (u##lit), ((sizeof(u##lit)/sizeof(*(u##lit)))-1) })
#define HAM_LIT_UTF32(lit) ((HAM_STR_UTF_(32)){ (U##lit), ((sizeof(U##lit)/sizeof(*(U##lit)))-1) })

#define HAM_LIT_UTF_(n, lit) HAM_LIT_UTF##n(lit)
#define HAM_LIT_UTF(n, lit) HAM_LIT_UTF_(n, lit)

#define HAM_LIT_C_UTF_(n, lit) HAM_LIT_C_UTF##n(lit)
#define HAM_LIT_C_UTF(n, lit) HAM_LIT_C_UTF_(n, lit)

#define HAM_LIT_C(lit) HAM_LIT_C_UTF(HAM_UTF, lit)
#define HAM_LIT(lit) HAM_LIT_UTF(HAM_UTF, lit)

typedef HAM_CHAR_UTF(HAM_UTF) ham_uchar;
typedef HAM_STR_UTF(HAM_UTF) ham_str;

#ifdef __cplusplus

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
}

#endif // __cplusplus

/**
 * @}
 */

#endif // !HAM_TYPEDEFS_H
