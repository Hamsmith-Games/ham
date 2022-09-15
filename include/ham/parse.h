#ifndef HAM_PARSE_H
#define HAM_PARSE_H 1

/**
 * @defgroup HAM_PARSE Expression parsing
 * @ingroup HAM
 * @{
 */

#include "lex.h"
#include "math.h"
#include "memory.h"

HAM_C_API_BEGIN

typedef enum ham_expr_kind{
	HAM_EXPR_ERROR,
	HAM_EXPR_BINDING,
	HAM_EXPR_REF,
	HAM_EXPR_UNRESOLVED,

	HAM_EXPR_LIT_INT,
	HAM_EXPR_LIT_REAL,
	HAM_EXPR_LIT_STR,

	HAM_EXPR_KIND_COUNT,
	HAM_EXPR_KIND_ERROR = HAM_EXPR_KIND_COUNT,
} ham_expr_kind;

#define HAM_PARSE_CONTEXT_UTF(n) HAM_CONCAT(ham_parse_context_utf, n)
#define HAM_PARSE_SCOPE_UTF(n) HAM_CONCAT(ham_parse_scope_utf, n)

#define HAM_EXPR_UTF_(n, name) ham_expr_##name##_utf##n
#define HAM_EXPR_UTF(n, name) HAM_EXPR_UTF_(n, name)

#define HAM_PARSE_X_UTF 8
#include "parse.x.h"

#define HAM_PARSE_X_UTF 16
#include "parse.x.h"

#define HAM_PARSE_X_UTF 32
#include "parse.x.h"

//
// Contexts
//

ham_api ham_parse_context_utf8  *ham_parse_context_create_utf8();
ham_api ham_parse_context_utf16 *ham_parse_context_create_utf16();
ham_api ham_parse_context_utf32 *ham_parse_context_create_utf32();

#define HAM_PARSE_CONTEXT_CREATE_UTF(n) HAM_CONCAT(ham_parse_context_create_utf, n)

ham_api void ham_parse_context_destroy_utf8 (ham_parse_context_utf8  *ctx);
ham_api void ham_parse_context_destroy_utf16(ham_parse_context_utf16 *ctx);
ham_api void ham_parse_context_destroy_utf32(ham_parse_context_utf32 *ctx);

#define HAM_PARSE_CONTEXT_DESTROY_UTF(n) HAM_CONCAT(ham_parse_context_destroy_utf, n)

//
// Scopes
//

ham_api ham_parse_scope_utf8  *ham_parse_scope_create_utf8 (ham_parse_context_utf8  *ctx, const ham_parse_scope_utf8  *parent);
ham_api ham_parse_scope_utf16 *ham_parse_scope_create_utf16(ham_parse_context_utf16 *ctx, const ham_parse_scope_utf16 *parent);
ham_api ham_parse_scope_utf32 *ham_parse_scope_create_utf32(ham_parse_context_utf32 *ctx, const ham_parse_scope_utf32 *parent);

#define HAM_PARSE_SCOPE_CREATE_UTF(n) HAM_CONCAT(ham_parse_scope_create_utf, n)

ham_api void ham_parse_scope_destroy_utf8 (ham_parse_scope_utf8  *scope);
ham_api void ham_parse_scope_destroy_utf16(ham_parse_scope_utf16 *scope);
ham_api void ham_parse_scope_destroy_utf32(ham_parse_scope_utf32 *scope);

#define HAM_PARSE_SCOPE_DESTROY_UTF(n) HAM_CONCAT(ham_parse_scope_destroy_utf, n)

ham_api bool ham_parse_scope_bind_utf8 (ham_parse_scope_utf8  *scope, const ham_expr_binding_utf8  *binding);
ham_api bool ham_parse_scope_bind_utf16(ham_parse_scope_utf16 *scope, const ham_expr_binding_utf16 *binding);
ham_api bool ham_parse_scope_bind_utf32(ham_parse_scope_utf32 *scope, const ham_expr_binding_utf32 *binding);

#define HAM_PARSE_SCOPE_BIND_UTF(n) HAM_CONCAT(ham_parse_scope_bind_utf, n)

ham_api const ham_expr_binding_utf8  *ham_parse_scope_resolve_utf8 (const ham_parse_scope_utf8  *scope, ham_str8  name);
ham_api const ham_expr_binding_utf16 *ham_parse_scope_resolve_utf16(const ham_parse_scope_utf16 *scope, ham_str16 name);
ham_api const ham_expr_binding_utf32 *ham_parse_scope_resolve_utf32(const ham_parse_scope_utf32 *scope, ham_str32 name);

#define HAM_PARSE_SCOPE_RESOLVE_UTF(n) HAM_CONCAT(ham_parse_scope_resolve_utf, n)

//
// Expressions
//

ham_api ham_expr_base_utf8  *ham_parse_context_new_expr_utf8 (ham_parse_context_utf8  *ctx, ham_expr_kind kind, ham_token_range_utf8  tokens);
ham_api ham_expr_base_utf16 *ham_parse_context_new_expr_utf16(ham_parse_context_utf16 *ctx, ham_expr_kind kind, ham_token_range_utf16 tokens);
ham_api ham_expr_base_utf32 *ham_parse_context_new_expr_utf32(ham_parse_context_utf32 *ctx, ham_expr_kind kind, ham_token_range_utf32 tokens);

#define HAM_PARSE_CONTEXT_NEW_EXPR_UTF(n) HAM_CONCAT(ham_parse_context_new_expr_utf, n)

ham_api const ham_expr_error_utf8  *ham_parse_context_new_error_utf8 (ham_parse_context_utf8  *ctx, const char *fmt_str, ...);
ham_api const ham_expr_error_utf16 *ham_parse_context_new_error_utf16(ham_parse_context_utf16 *ctx, const char *fmt_str, ...);
ham_api const ham_expr_error_utf32 *ham_parse_context_new_error_utf32(ham_parse_context_utf32 *ctx, const char *fmt_str, ...);

#define HAM_PARSE_CONTEXT_NEW_ERROR_UTF(n) HAM_CONCAT(ham_parse_context_new_error_utf, n)

//
// Parsing
//

ham_api const ham_expr_base_utf8  *ham_parse_utf8 (ham_parse_context_utf8  *ctx, ham_parse_scope_utf8  *scope, ham_token_range_utf8  tokens);
ham_api const ham_expr_base_utf16 *ham_parse_utf16(ham_parse_context_utf16 *ctx, ham_parse_scope_utf16 *scope, ham_token_range_utf16 tokens);
ham_api const ham_expr_base_utf32 *ham_parse_utf32(ham_parse_context_utf32 *ctx, ham_parse_scope_utf32 *scope, ham_token_range_utf32 tokens);

#define HAM_PARSE_UTF(n) HAM_CONCAT(ham_parse_utf, n)

//
// Default aliases
//

typedef HAM_PARSE_CONTEXT_UTF(HAM_UTF) ham_parse_context;
typedef HAM_PARSE_SCOPE_UTF(HAM_UTF)   ham_parse_scope;

typedef HAM_EXPR_UTF(HAM_UTF, base)       ham_expr;
typedef HAM_EXPR_UTF(HAM_UTF, error)      ham_expr_error;
typedef HAM_EXPR_UTF(HAM_UTF, binding)    ham_expr_binding;
typedef HAM_EXPR_UTF(HAM_UTF, ref)        ham_expr_ref;
typedef HAM_EXPR_UTF(HAM_UTF, unresolved) ham_expr_unresolved;

typedef HAM_EXPR_UTF(HAM_UTF, lit_int)    ham_expr_lit_int;
typedef HAM_EXPR_UTF(HAM_UTF, lit_real)   ham_expr_lit_real;
typedef HAM_EXPR_UTF(HAM_UTF, lit_str)    ham_expr_lit_str;

#define ham_parse_context_create HAM_PARSE_CONTEXT_CREATE_UTF(HAM_UTF)
#define ham_parse_context_destroy HAM_PARSE_CONTEXT_DESTROY_UTF(HAM_UTF)

#define ham_parse_scope_create HAM_PARSE_SCOPE_CREATE_UTF(HAM_UTF)
#define ham_parse_scope_destroy HAM_PARSE_SCOPE_DESTROY_UTF(HAM_UTF)
#define ham_parse_scope_bind HAM_PARSE_SCOPE_BIND_UTF(HAM_UTF)
#define ham_parse_scope_resolve HAM_PARSE_SCOPE_RESOLVE_UTF(HAM_UTF)

#define ham_parse_context_new_expr HAM_PARSE_CONTEXT_NEW_EXPR_UTF(HAM_UTF)
#define ham_parse_context_new_error HAM_PARSE_CONTEXT_NEW_ERROR_UTF(HAM_UTF)

#define ham_parse HAM_PARSE_UTF(HAM_UTF)

HAM_C_API_END

/**
 * @}
 */

#endif // !HAM_PARSE_H
