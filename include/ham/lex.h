#ifndef HAM_LEX_H
#define HAM_LEX_H 1

/**
 * @defgroup HAM_LEX Lexing
 * @ingroup HAM
 * @{
 */

#include "config.h"
#include "typedefs.h"

HAM_C_API_BEGIN

#define HAM_LEX_ERROR_MAX_LENGTH 512

typedef enum ham_token_kind{
	HAM_TOKEN_EOF,
	HAM_TOKEN_ERROR,
	HAM_TOKEN_NEWLINE,
	HAM_TOKEN_SPACE,
	HAM_TOKEN_ID,
	HAM_TOKEN_NAT,
	HAM_TOKEN_REAL,
	HAM_TOKEN_STR,
	HAM_TOKEN_OP,
	HAM_TOKEN_BRACKET,

	HAM_TOKEN_KIND_COUNT,
	HAM_TOKEN_KIND_ERROR = HAM_TOKEN_KIND_COUNT,
} ham_token_kind;

#define HAM_SOURCE_LOCATION_UTF(n) HAM_CONCAT(ham_source_location_utf, n)
#define HAM_TOKEN_UTF(n) HAM_CONCAT(ham_token_utf, n)
#define HAM_TOKEN_KIND_STR_UTF(n) HAM_CONCAT(ham_token_kind_str_utf, n)

#define HAM_LEX_X_UTF 8
#include "lex.x.h"

#define HAM_LEX_X_UTF 16
#include "lex.x.h"

#define HAM_LEX_X_UTF 32
#include "lex.x.h"

#define HAM_LEX_UTF(n) HAM_CONCAT(ham_lex_utf, n)

ham_api bool ham_lex_utf8 (ham_source_location_utf8  *loc, ham_str8  src, ham_token_utf8  *ret);
ham_api bool ham_lex_utf16(ham_source_location_utf16 *loc, ham_str16 src, ham_token_utf16 *ret);
ham_api bool ham_lex_utf32(ham_source_location_utf32 *loc, ham_str32 src, ham_token_utf32 *ret);

typedef HAM_SOURCE_LOCATION_UTF(HAM_UTF) ham_source_location;
typedef HAM_TOKEN_UTF(HAM_UTF) ham_token;

#define ham_token_kind_str HAM_TOKEN_KIND_STR_UTF(HAM_UTF)
#define ham_lex HAM_LEX_UTF(HAM_UTF)

HAM_C_API_END

/**
 * @}
 */

#endif // !HAM_LEX_H
