/**
 * The Ham Programming Language
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

#ifndef HAM_PARSE_H
#define HAM_PARSE_H 1

/**
 * @defgroup HAM_PARSE Expression parsing
 * @ingroup HAM
 * @{
 */

#include "lex.h"
#include "math.h"

HAM_C_API_BEGIN

typedef struct ham_parse_context_utf8  ham_parse_context_utf8;
typedef struct ham_parse_context_utf16 ham_parse_context_utf16;
typedef struct ham_parse_context_utf32 ham_parse_context_utf32;

typedef struct ham_parse_scope_utf8  ham_parse_scope_utf8;
typedef struct ham_parse_scope_utf16 ham_parse_scope_utf16;
typedef struct ham_parse_scope_utf32 ham_parse_scope_utf32;

typedef struct ham_expr_base_utf8  ham_expr_base_utf8;
typedef struct ham_expr_base_utf16 ham_expr_base_utf16;
typedef struct ham_expr_base_utf32 ham_expr_base_utf32;

typedef struct ham_expr_error_utf8  ham_expr_error_utf8;
typedef struct ham_expr_error_utf16 ham_expr_error_utf16;
typedef struct ham_expr_error_utf32 ham_expr_error_utf32;

typedef struct ham_expr_binding_utf8  ham_expr_binding_utf8;
typedef struct ham_expr_binding_utf16 ham_expr_binding_utf16;
typedef struct ham_expr_binding_utf32 ham_expr_binding_utf32;

typedef struct ham_expr_ref_utf8  ham_expr_ref_utf8;
typedef struct ham_expr_ref_utf16 ham_expr_ref_utf16;
typedef struct ham_expr_ref_utf32 ham_expr_ref_utf32;

typedef struct ham_expr_unresolved_utf8  ham_expr_unresolved_utf8;
typedef struct ham_expr_unresolved_utf16 ham_expr_unresolved_utf16;
typedef struct ham_expr_unresolved_utf32 ham_expr_unresolved_utf32;

typedef struct ham_expr_fn_utf8  ham_expr_fn_utf8;
typedef struct ham_expr_fn_utf16 ham_expr_fn_utf16;
typedef struct ham_expr_fn_utf32 ham_expr_fn_utf32;

typedef struct ham_expr_call_utf8  ham_expr_call_utf8;
typedef struct ham_expr_call_utf16 ham_expr_call_utf16;
typedef struct ham_expr_call_utf32 ham_expr_call_utf32;

typedef struct ham_expr_block_utf8  ham_expr_block_utf8;
typedef struct ham_expr_block_utf16 ham_expr_block_utf16;
typedef struct ham_expr_block_utf32 ham_expr_block_utf32;

typedef struct ham_expr_unary_op_utf8  ham_expr_unary_op_utf8;
typedef struct ham_expr_unary_op_utf16 ham_expr_unary_op_utf16;
typedef struct ham_expr_unary_op_utf32 ham_expr_unary_op_utf32;

typedef struct ham_expr_binary_op_utf8  ham_expr_binary_op_utf8;
typedef struct ham_expr_binary_op_utf16 ham_expr_binary_op_utf16;
typedef struct ham_expr_binary_op_utf32 ham_expr_binary_op_utf32;

typedef struct ham_expr_lit_int_utf8  ham_expr_lit_int_utf8;
typedef struct ham_expr_lit_int_utf16 ham_expr_lit_int_utf16;
typedef struct ham_expr_lit_int_utf32 ham_expr_lit_int_utf32;

typedef struct ham_expr_lit_real_utf8  ham_expr_lit_real_utf8;
typedef struct ham_expr_lit_real_utf16 ham_expr_lit_real_utf16;
typedef struct ham_expr_lit_real_utf32 ham_expr_lit_real_utf32;

typedef struct ham_expr_lit_str_utf8  ham_expr_lit_str_utf8;
typedef struct ham_expr_lit_str_utf16 ham_expr_lit_str_utf16;
typedef struct ham_expr_lit_str_utf32 ham_expr_lit_str_utf32;

typedef enum ham_expr_kind{
	HAM_EXPR_ERROR,
	HAM_EXPR_BINDING,
	HAM_EXPR_REF,
	HAM_EXPR_UNRESOLVED,
	HAM_EXPR_FN,
	HAM_EXPR_CALL,
	HAM_EXPR_BLOCK,

	HAM_EXPR_UNARY_OP,
	HAM_EXPR_BINARY_OP,

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

//! @cond ignore
#define HAM_IMPL_EXPR_BASE_MEMBERS_UTF(n) \
	ham_expr_kind kind; \
	HAM_TOKEN_RANGE_UTF(n) tokens;

#define HAM_IMPL_EXPR_ERROR_MEMBERS_UTF(n) \
	HAM_EXPR_UTF(n, base) super; \
	const HAM_EXPR_UTF(n, base) *prev; \
	ham_str8 message;

#define HAM_IMPL_EXPR_BINDING_MEMBERS_UTF(n) \
	HAM_EXPR_UTF(n, base) super; \
	HAM_STR_UTF(n) name; \
	const HAM_EXPR_UTF(n, base) *type, *value;

#define HAM_IMPL_EXPR_REF_MEMBERS_UTF(n) \
	HAM_EXPR_UTF(n, base) super; \
	const HAM_EXPR_UTF(n, binding) *refed;

#define HAM_IMPL_EXPR_UNRESOLVED_MEMBERS_UTF(n) \
	HAM_EXPR_UTF(n, base) super; \
	HAM_STR_UTF(n) id;

#define HAM_IMPL_EXPR_FN_MEMBERS_UTF(n) \
	HAM_EXPR_UTF(n, base) super; \
	ham_usize num_params; \
	const HAM_EXPR_UTF(n, base) *const *params; \
	const HAM_EXPR_UTF(n, base) *return_type; \
	const HAM_EXPR_UTF(n, base) *body;

#define HAM_IMPL_EXPR_CALL_MEMBERS_UTF(n) \
	HAM_EXPR_UTF(n, base) super; \
	const HAM_EXPR_UTF(n, base) *fn; \
	ham_usize num_args; \
	const HAM_EXPR_UTF(n, base) *const *args;

#define HAM_IMPL_EXPR_BLOCK_MEMBERS_UTF(n) \
	HAM_EXPR_UTF(n, base) super; \
	ham_usize num_exprs; \
	const HAM_EXPR_UTF(n, base) *const *exprs;

#define HAM_IMPL_EXPR_UNARY_OP_MEMBERS_UTF(n) \
	HAM_EXPR_UTF(n, base) super; \
	HAM_STR_UTF(n) op; \
	const HAM_EXPR_UTF(n, base) *expr;

#define HAM_IMPL_EXPR_BINARY_OP_MEMBERS_UTF(n) \
	HAM_EXPR_UTF(n, base) super; \
	HAM_STR_UTF(n) op; \
	const HAM_EXPR_UTF(n, base) *lhs, *rhs;

#define HAM_IMPL_EXPR_LIT_INT_UTF(n) \
	HAM_EXPR_UTF(n, base) super; \
	ham_aint value;

#define HAM_IMPL_EXPR_LIT_REAL_UTF(n) \
	HAM_EXPR_UTF(n, base) super; \
	ham_areal value;

#define HAM_IMPL_EXPR_LIT_STR_UTF(n) \
	HAM_EXPR_UTF(n, base) super; \
	HAM_STR_UTF(n) value;

#define HAM_IMPL_PARSE_CONTEXT_MEMBERS_UTF(n) \


//! @endcond

//
// Expressions
//

struct ham_expr_base_utf8 { HAM_IMPL_EXPR_BASE_MEMBERS_UTF(8)  };
struct ham_expr_base_utf16{ HAM_IMPL_EXPR_BASE_MEMBERS_UTF(16) };
struct ham_expr_base_utf32{ HAM_IMPL_EXPR_BASE_MEMBERS_UTF(32) };

struct ham_expr_error_utf8 { HAM_IMPL_EXPR_ERROR_MEMBERS_UTF(8);  };
struct ham_expr_error_utf16{ HAM_IMPL_EXPR_ERROR_MEMBERS_UTF(16); };
struct ham_expr_error_utf32{ HAM_IMPL_EXPR_ERROR_MEMBERS_UTF(32); };

struct ham_expr_binding_utf8 { HAM_IMPL_EXPR_BINDING_MEMBERS_UTF(8)  };
struct ham_expr_binding_utf16{ HAM_IMPL_EXPR_BINDING_MEMBERS_UTF(16) };
struct ham_expr_binding_utf32{ HAM_IMPL_EXPR_BINDING_MEMBERS_UTF(32) };

struct ham_expr_ref_utf8 { HAM_IMPL_EXPR_REF_MEMBERS_UTF(8)  };
struct ham_expr_ref_utf16{ HAM_IMPL_EXPR_REF_MEMBERS_UTF(16) };
struct ham_expr_ref_utf32{ HAM_IMPL_EXPR_REF_MEMBERS_UTF(32) };

struct ham_expr_unresolved_utf8 { HAM_IMPL_EXPR_UNRESOLVED_MEMBERS_UTF(8)  };
struct ham_expr_unresolved_utf16{ HAM_IMPL_EXPR_UNRESOLVED_MEMBERS_UTF(16) };
struct ham_expr_unresolved_utf32{ HAM_IMPL_EXPR_UNRESOLVED_MEMBERS_UTF(32) };

struct ham_expr_fn_utf8 { HAM_IMPL_EXPR_FN_MEMBERS_UTF(8)  };
struct ham_expr_fn_utf16{ HAM_IMPL_EXPR_FN_MEMBERS_UTF(16) };
struct ham_expr_fn_utf32{ HAM_IMPL_EXPR_FN_MEMBERS_UTF(32) };

struct ham_expr_call_utf8 { HAM_IMPL_EXPR_CALL_MEMBERS_UTF(8)  };
struct ham_expr_call_utf16{ HAM_IMPL_EXPR_CALL_MEMBERS_UTF(16) };
struct ham_expr_call_utf32{ HAM_IMPL_EXPR_CALL_MEMBERS_UTF(32) };

struct ham_expr_block_utf8 { HAM_IMPL_EXPR_BLOCK_MEMBERS_UTF(8)  };
struct ham_expr_block_utf16{ HAM_IMPL_EXPR_BLOCK_MEMBERS_UTF(16) };
struct ham_expr_block_utf32{ HAM_IMPL_EXPR_BLOCK_MEMBERS_UTF(32) };

struct ham_expr_unary_op_utf8 { HAM_IMPL_EXPR_UNARY_OP_MEMBERS_UTF(8)  };
struct ham_expr_unary_op_utf16{ HAM_IMPL_EXPR_UNARY_OP_MEMBERS_UTF(16) };
struct ham_expr_unary_op_utf32{ HAM_IMPL_EXPR_UNARY_OP_MEMBERS_UTF(32) };

struct ham_expr_binary_op_utf8 { HAM_IMPL_EXPR_BINARY_OP_MEMBERS_UTF(8)  };
struct ham_expr_binary_op_utf16{ HAM_IMPL_EXPR_BINARY_OP_MEMBERS_UTF(16) };
struct ham_expr_binary_op_utf32{ HAM_IMPL_EXPR_BINARY_OP_MEMBERS_UTF(32) };

struct ham_expr_lit_int_utf8 { HAM_IMPL_EXPR_LIT_INT_UTF(8)  };
struct ham_expr_lit_int_utf16{ HAM_IMPL_EXPR_LIT_INT_UTF(16) };
struct ham_expr_lit_int_utf32{ HAM_IMPL_EXPR_LIT_INT_UTF(32) };

struct ham_expr_lit_real_utf8 { HAM_IMPL_EXPR_LIT_REAL_UTF(8)  };
struct ham_expr_lit_real_utf16{ HAM_IMPL_EXPR_LIT_REAL_UTF(16) };
struct ham_expr_lit_real_utf32{ HAM_IMPL_EXPR_LIT_REAL_UTF(32) };

struct ham_expr_lit_str_utf8 { HAM_IMPL_EXPR_LIT_STR_UTF(8)  };
struct ham_expr_lit_str_utf16{ HAM_IMPL_EXPR_LIT_STR_UTF(16) };
struct ham_expr_lit_str_utf32{ HAM_IMPL_EXPR_LIT_STR_UTF(32) };

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

ham_api ham_parse_scope_utf8  *ham_parse_scope_create_utf8 (ham_parse_context_utf8  *ctx, ham_parse_scope_utf8  *parent);
ham_api ham_parse_scope_utf16 *ham_parse_scope_create_utf16(ham_parse_context_utf16 *ctx, ham_parse_scope_utf16 *parent);
ham_api ham_parse_scope_utf32 *ham_parse_scope_create_utf32(ham_parse_context_utf32 *ctx, ham_parse_scope_utf32 *parent);

#define HAM_PARSE_SCOPE_CREATE_UTF(n) HAM_CONCAT(ham_parse_scope_create_utf, n)

ham_api void ham_parse_scope_destroy_utf8 (ham_parse_scope_utf8  *scope);
ham_api void ham_parse_scope_destroy_utf16(ham_parse_scope_utf16 *scope);
ham_api void ham_parse_scope_destroy_utf32(ham_parse_scope_utf32 *scope);

#define HAM_PARSE_SCOPE_DESTROY_UTF(n) HAM_CONCAT(ham_parse_scope_destroy_utf, n)

ham_api ham_parse_context_utf8  *ham_parse_scope_context_utf8 (const ham_parse_scope_utf8  *scope);
ham_api ham_parse_context_utf16 *ham_parse_scope_context_utf16(const ham_parse_scope_utf16 *scope);
ham_api ham_parse_context_utf32 *ham_parse_scope_context_utf32(const ham_parse_scope_utf32 *scope);

#define HAM_PARSE_SCOPE_CONTEXT_UTF(n) HAM_CONCAT(ham_parse_scope_context_utf, n)

ham_api ham_parse_scope_utf8  *ham_parse_scope_parent_utf8 (ham_parse_scope_utf8  *scope);
ham_api ham_parse_scope_utf16 *ham_parse_scope_parent_utf16(ham_parse_scope_utf16 *scope);
ham_api ham_parse_scope_utf32 *ham_parse_scope_parent_utf32(ham_parse_scope_utf32 *scope);

#define HAM_PARSE_SCOPE_PARENT_UTF(n) HAM_CONCAT(ham_parse_scope_parent_utf, n)

ham_api ham_str8  ham_parse_scope_get_indent_utf8 (const ham_parse_scope_utf8  *scope);
ham_api ham_str16 ham_parse_scope_get_indent_utf16(const ham_parse_scope_utf16 *scope);
ham_api ham_str32 ham_parse_scope_get_indent_utf32(const ham_parse_scope_utf32 *scope);

#define HAM_PARSE_SCOPE_GET_INDENT_UTF(n) HAM_CONCAT(ham_parse_scope_get_indent_utf, n)

ham_api bool ham_parse_scope_set_indent_utf8 (ham_parse_scope_utf8  *scope, ham_str8  new_indent);
ham_api bool ham_parse_scope_set_indent_utf16(ham_parse_scope_utf16 *scope, ham_str16 new_indent);
ham_api bool ham_parse_scope_set_indent_utf32(ham_parse_scope_utf32 *scope, ham_str32 new_indent);

#define HAM_PARSE_SCOPE_SET_INDENT_UTF(n) HAM_CONCAT(ham_parse_scope_set_indent_utf, n)

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

ham_api const ham_expr_error_utf8  *ham_parse_context_new_error_utf8 (ham_parse_context_utf8  *ctx, ham_token_range_utf8  tokens, const ham_expr_base_utf8  *prev, const char *fmt_str, ...);
ham_api const ham_expr_error_utf16 *ham_parse_context_new_error_utf16(ham_parse_context_utf16 *ctx, ham_token_range_utf16 tokens, const ham_expr_base_utf16 *prev, const char *fmt_str, ...);
ham_api const ham_expr_error_utf32 *ham_parse_context_new_error_utf32(ham_parse_context_utf32 *ctx, ham_token_range_utf32 tokens, const ham_expr_base_utf32 *prev, const char *fmt_str, ...);

#define HAM_PARSE_CONTEXT_NEW_ERROR_UTF(n) HAM_CONCAT(ham_parse_context_new_error_utf, n)

ham_api const ham_expr_binding_utf8  *ham_parse_context_new_binding_utf8 (ham_parse_context_utf8  *ctx, ham_token_range_utf8  tokens, ham_str8  name, const ham_expr_base_utf8  *type, const ham_expr_base_utf8  *value);
ham_api const ham_expr_binding_utf16 *ham_parse_context_new_binding_utf16(ham_parse_context_utf16 *ctx, ham_token_range_utf16 tokens, ham_str16 name, const ham_expr_base_utf16 *type, const ham_expr_base_utf16 *value);
ham_api const ham_expr_binding_utf32 *ham_parse_context_new_binding_utf32(ham_parse_context_utf32 *ctx, ham_token_range_utf32 tokens, ham_str32 name, const ham_expr_base_utf32 *type, const ham_expr_base_utf32 *value);

#define HAM_PARSE_CONTEXT_NEW_BINDING_UTF(n) HAM_CONCAT(ham_parse_context_new_binding_utf, n)

ham_api const ham_expr_ref_utf8  *ham_parse_context_new_ref_utf8 (ham_parse_context_utf8  *ctx, ham_token_range_utf8  tokens, const ham_expr_binding_utf8  *refed);
ham_api const ham_expr_ref_utf16 *ham_parse_context_new_ref_utf16(ham_parse_context_utf16 *ctx, ham_token_range_utf16 tokens, const ham_expr_binding_utf16 *refed);
ham_api const ham_expr_ref_utf32 *ham_parse_context_new_ref_utf32(ham_parse_context_utf32 *ctx, ham_token_range_utf32 tokens, const ham_expr_binding_utf32 *refed);

#define HAM_PARSE_CONTEXT_NEW_REF_UTF(n) HAM_CONCAT(ham_parse_context_new_ref_utf, n)

ham_api const ham_expr_unresolved_utf8  *ham_parse_context_new_unresolved_utf8 (ham_parse_context_utf8  *ctx, ham_token_range_utf8  tokens, ham_str8  id);
ham_api const ham_expr_unresolved_utf16 *ham_parse_context_new_unresolved_utf16(ham_parse_context_utf16 *ctx, ham_token_range_utf16 tokens, ham_str16 id);
ham_api const ham_expr_unresolved_utf32 *ham_parse_context_new_unresolved_utf32(ham_parse_context_utf32 *ctx, ham_token_range_utf32 tokens, ham_str32 id);

#define HAM_PARSE_CONTEXT_NEW_UNRESOLVED_UTF(n) HAM_CONCAT(ham_parse_context_new_unresolved_utf, n)

ham_api const ham_expr_fn_utf8  *ham_parse_context_new_fn_utf8 (ham_parse_context_utf8  *ctx, ham_token_range_utf8  tokens, ham_usize num_params, const ham_expr_base_utf8  *const *params, const ham_expr_base_utf8  *return_type);
ham_api const ham_expr_fn_utf16 *ham_parse_context_new_fn_utf16(ham_parse_context_utf16 *ctx, ham_token_range_utf16 tokens, ham_usize num_params, const ham_expr_base_utf16 *const *params, const ham_expr_base_utf16 *return_type);
ham_api const ham_expr_fn_utf32 *ham_parse_context_new_fn_utf32(ham_parse_context_utf32 *ctx, ham_token_range_utf32 tokens, ham_usize num_params, const ham_expr_base_utf32 *const *params, const ham_expr_base_utf32 *return_type);

#define HAM_PARSE_CONTEXT_NEW_FN_UTF(n) HAM_CONCAT(ham_parse_context_new_fn_utf, n)

ham_api const ham_expr_call_utf8  *ham_parse_context_new_call_utf8 (ham_parse_context_utf8  *ctx, ham_token_range_utf8  tokens, const ham_expr_base_utf8  *fn, ham_usize num_args, const ham_expr_base_utf8  *const *args);
ham_api const ham_expr_call_utf16 *ham_parse_context_new_call_utf16(ham_parse_context_utf16 *ctx, ham_token_range_utf16 tokens, const ham_expr_base_utf16 *fn, ham_usize num_args, const ham_expr_base_utf16 *const *args);
ham_api const ham_expr_call_utf32 *ham_parse_context_new_call_utf32(ham_parse_context_utf32 *ctx, ham_token_range_utf32 tokens, const ham_expr_base_utf32 *fn, ham_usize num_args, const ham_expr_base_utf32 *const *args);

#define HAM_PARSE_CONTEXT_NEW_CALL_UTF(n) HAM_CONCAT(ham_parse_context_new_call_utf, n)

ham_api const ham_expr_block_utf8  *ham_parse_context_new_block_utf8 (ham_parse_context_utf8  *ctx, ham_token_range_utf8  tokens, ham_usize num_exprs, const ham_expr_base_utf8  *const *exprs);
ham_api const ham_expr_block_utf16 *ham_parse_context_new_block_utf16(ham_parse_context_utf16 *ctx, ham_token_range_utf16 tokens, ham_usize num_exprs, const ham_expr_base_utf16 *const *exprs);
ham_api const ham_expr_block_utf32 *ham_parse_context_new_block_utf32(ham_parse_context_utf32 *ctx, ham_token_range_utf32 tokens, ham_usize num_exprs, const ham_expr_base_utf32 *const *exprs);

#define HAM_PARSE_CONTEXT_NEW_BLOCK_UTF(n) HAM_CONCAT(ham_parse_context_new_block_utf, n)

ham_api const ham_expr_unary_op_utf8  *ham_parse_context_new_unary_op_utf8 (ham_parse_context_utf8  *ctx, ham_token_range_utf8  tokens, ham_str8  op, const ham_expr_base_utf8  *expr);
ham_api const ham_expr_unary_op_utf16 *ham_parse_context_new_unary_op_utf16(ham_parse_context_utf16 *ctx, ham_token_range_utf16 tokens, ham_str16 op, const ham_expr_base_utf16 *expr);
ham_api const ham_expr_unary_op_utf32 *ham_parse_context_new_unary_op_utf32(ham_parse_context_utf32 *ctx, ham_token_range_utf32 tokens, ham_str32 op, const ham_expr_base_utf32 *expr);

#define HAM_PARSE_CONTEXT_NEW_UNARY_OP_UTF(n) HAM_CONCAT(ham_parse_context_new_unary_op_utf, n)

ham_api const ham_expr_binary_op_utf8  *ham_parse_context_new_binary_op_utf8 (ham_parse_context_utf8  *ctx, ham_token_range_utf8  tokens, ham_str8  op, const ham_expr_base_utf8  *lhs, const ham_expr_base_utf8  *rhs);
ham_api const ham_expr_binary_op_utf16 *ham_parse_context_new_binary_op_utf16(ham_parse_context_utf16 *ctx, ham_token_range_utf16 tokens, ham_str16 op, const ham_expr_base_utf16 *lhs, const ham_expr_base_utf16 *rhs);
ham_api const ham_expr_binary_op_utf32 *ham_parse_context_new_binary_op_utf32(ham_parse_context_utf32 *ctx, ham_token_range_utf32 tokens, ham_str32 op, const ham_expr_base_utf32 *lhs, const ham_expr_base_utf32 *rhs);

#define HAM_PARSE_CONTEXT_NEW_BINARY_OP_UTF(n) HAM_CONCAT(ham_parse_context_new_binary_op_utf, n)

ham_api const ham_expr_lit_int_utf8  *ham_parse_context_new_lit_int_utf8 (ham_parse_context_utf8  *ctx, ham_token_range_utf8  tokens, ham_str8  val);
ham_api const ham_expr_lit_int_utf16 *ham_parse_context_new_lit_int_utf16(ham_parse_context_utf16 *ctx, ham_token_range_utf16 tokens, ham_str16 val);
ham_api const ham_expr_lit_int_utf32 *ham_parse_context_new_lit_int_utf32(ham_parse_context_utf32 *ctx, ham_token_range_utf32 tokens, ham_str32 val);

#define HAM_PARSE_CONTEXT_NEW_LIT_INT_UTF(n) HAM_CONCAT(ham_parse_context_new_lit_int_utf, n)

ham_api const ham_expr_lit_real_utf8  *ham_parse_context_new_lit_real_utf8 (ham_parse_context_utf8  *ctx, ham_token_range_utf8  tokens, ham_str8  val);
ham_api const ham_expr_lit_real_utf16 *ham_parse_context_new_lit_real_utf16(ham_parse_context_utf16 *ctx, ham_token_range_utf16 tokens, ham_str16 val);
ham_api const ham_expr_lit_real_utf32 *ham_parse_context_new_lit_real_utf32(ham_parse_context_utf32 *ctx, ham_token_range_utf32 tokens, ham_str32 val);

#define HAM_PARSE_CONTEXT_NEW_LIT_REAL_UTF(n) HAM_CONCAT(ham_parse_context_new_lit_real_utf, n)

ham_api const ham_expr_lit_str_utf8  *ham_parse_context_new_lit_str_utf8 (ham_parse_context_utf8  *ctx, ham_token_range_utf8  tokens, ham_str8  val);
ham_api const ham_expr_lit_str_utf16 *ham_parse_context_new_lit_str_utf16(ham_parse_context_utf16 *ctx, ham_token_range_utf16 tokens, ham_str16 val);
ham_api const ham_expr_lit_str_utf32 *ham_parse_context_new_lit_str_utf32(ham_parse_context_utf32 *ctx, ham_token_range_utf32 tokens, ham_str32 val);

#define HAM_PARSE_CONTEXT_NEW_LIT_STR_UTF(n) HAM_CONCAT(ham_parse_context_new_lit_str_utf, n)

//
// Parsing
//

ham_api const ham_expr_base_utf8  *ham_parse_utf8 (ham_parse_context_utf8  *ctx, ham_token_range_utf8  tokens);
ham_api const ham_expr_base_utf16 *ham_parse_utf16(ham_parse_context_utf16 *ctx, ham_token_range_utf16 tokens);
ham_api const ham_expr_base_utf32 *ham_parse_utf32(ham_parse_context_utf32 *ctx, ham_token_range_utf32 tokens);

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
typedef HAM_EXPR_UTF(HAM_UTF, fn)         ham_expr_fn;
typedef HAM_EXPR_UTF(HAM_UTF, call)       ham_expr_call;
typedef HAM_EXPR_UTF(HAM_UTF, block)      ham_expr_block;

typedef HAM_EXPR_UTF(HAM_UTF, unary_op)  ham_expr_unary_op;
typedef HAM_EXPR_UTF(HAM_UTF, binary_op) ham_expr_binary_op;

typedef HAM_EXPR_UTF(HAM_UTF, lit_int)    ham_expr_lit_int;
typedef HAM_EXPR_UTF(HAM_UTF, lit_real)   ham_expr_lit_real;
typedef HAM_EXPR_UTF(HAM_UTF, lit_str)    ham_expr_lit_str;

#define ham_parse_context_create  HAM_PARSE_CONTEXT_CREATE_UTF(HAM_UTF)
#define ham_parse_context_destroy HAM_PARSE_CONTEXT_DESTROY_UTF(HAM_UTF)

#define ham_parse_scope_create     HAM_PARSE_SCOPE_CREATE_UTF(HAM_UTF)
#define ham_parse_scope_destroy    HAM_PARSE_SCOPE_DESTROY_UTF(HAM_UTF)
#define ham_parse_scope_context    HAM_PARSE_SCOPE_CONTEXT_UTF(HAM_UTF)
#define ham_parse_scope_get_indent HAM_PARSE_SCOPE_GET_INDENT_UTF(HAM_UTF)
#define ham_parse_scope_set_indent HAM_PARSE_SCOPE_SET_INDENT_UTF(HAM_UTF)
#define ham_parse_scope_bind       HAM_PARSE_SCOPE_BIND_UTF(HAM_UTF)
#define ham_parse_scope_resolve    HAM_PARSE_SCOPE_RESOLVE_UTF(HAM_UTF)

#define ham_parse_context_new_expr       HAM_PARSE_CONTEXT_NEW_EXPR_UTF(HAM_UTF)
#define ham_parse_context_new_error      HAM_PARSE_CONTEXT_NEW_ERROR_UTF(HAM_UTF)
#define ham_parse_context_new_binding    HAM_PARSE_CONTEXT_NEW_BINDING_UTF(HAM_UTF)
#define ham_parse_context_new_ref        HAM_PARSE_CONTEXT_NEW_REF_UTF(HAM_UTF)
#define ham_parse_context_new_unresolved HAM_PARSE_CONTEXT_NEW_UNRESOLVED_UTF(HAM_UTF)
#define ham_parse_context_new_fn         HAM_PARSE_CONTEXT_NEW_FN_UTF(HAM_UTF)
#define ham_parse_context_new_call       HAM_PARSE_CONTEXT_NEW_CALL_UTF(HAM_UTF)
#define ham_parse_context_new_block      HAM_PARSE_CONTEXT_NEW_BLOCK_UTF(HAM_UTF)

#define ham_parse_context_new_lit_int  HAM_PARSE_CONTEXT_NEW_LIT_INT_UTF(HAM_UTF)
#define ham_parse_context_new_lit_real HAM_PARSE_CONTEXT_NEW_LIT_REAL_UTF(HAM_UTF)
#define ham_parse_context_new_lit_str  HAM_PARSE_CONTEXT_NEW_LIT_STR_UTF(HAM_UTF)

#define ham_parse HAM_PARSE_UTF(HAM_UTF)

HAM_C_API_END

#ifdef __cplusplus

namespace ham{
	enum class expr_kind: std::underlying_type_t<ham_expr_kind>{
		error      = HAM_EXPR_ERROR,
		binding    = HAM_EXPR_BINDING,
		ref        = HAM_EXPR_REF,
		unresolved = HAM_EXPR_UNRESOLVED,
		fn         = HAM_EXPR_FN,
		call       = HAM_EXPR_CALL,
		block      = HAM_EXPR_BLOCK,

		unary_op  = HAM_EXPR_UNARY_OP,
		binary_op = HAM_EXPR_BINARY_OP,

		lit_int  = HAM_EXPR_LIT_INT,
		lit_real = HAM_EXPR_LIT_REAL,
		lit_str  = HAM_EXPR_LIT_STR,

		base, // special c++ only kind
	};

	constexpr str8 expr_kind_str8(expr_kind kind){
		switch(kind){
		#define HAM_CASE(kind_) case (kind_): return #kind_;

			HAM_CASE(expr_kind::error)
			HAM_CASE(expr_kind::binding)
			HAM_CASE(expr_kind::ref)
			HAM_CASE(expr_kind::unresolved)
			HAM_CASE(expr_kind::fn)
			HAM_CASE(expr_kind::call)
			HAM_CASE(expr_kind::block)

			HAM_CASE(expr_kind::unary_op)
			HAM_CASE(expr_kind::binary_op)

			HAM_CASE(expr_kind::lit_int)
			HAM_CASE(expr_kind::lit_real)
			HAM_CASE(expr_kind::lit_str)

		#undef HAM_CASE
			default: return "unknown";
		}
	}

	template<typename Char, bool Mutable = true>
	class basic_parse_context_view;

	template<typename Char>
	class basic_parse_context;

	template<typename Char>
	class basic_parse_scope;

	template<typename Char, bool Mutable = true>
	class basic_parse_scope_view;

	template<typename Char, bool Mutable = false>
	class basic_expr_iterator;

	template<typename Char, bool Mutable = false>
	class basic_expr_range;

	template<typename Char, expr_kind Kind = expr_kind::base, bool Mutable = false>
	class basic_expr;

	//! @cond ignore
	namespace detail{
		template<typename Char>
		using parse_context_ctype_t = utf_conditional_t<
			Char,
			ham_parse_context_utf8,
			ham_parse_context_utf16,
			ham_parse_context_utf32
		>;

		template<typename Char>
		using parse_scope_ctype_t = utf_conditional_t<
			Char,
			ham_parse_scope_utf8,
			ham_parse_scope_utf16,
			ham_parse_scope_utf32
		>;

		template<typename Char>
		using expr_base_ctype_t = utf_conditional_t<
			Char,
			ham_expr_base_utf8,
			ham_expr_base_utf16,
			ham_expr_base_utf32
		>;

		template<typename Char>
		using expr_error_ctype_t = utf_conditional_t<
			Char,
			ham_expr_error_utf8,
			ham_expr_error_utf16,
			ham_expr_error_utf32
		>;

		template<typename Char>
		using expr_binding_ctype_t = utf_conditional_t<
			Char,
			ham_expr_binding_utf8,
			ham_expr_binding_utf16,
			ham_expr_binding_utf32
		>;

		template<typename Char>
		using expr_ref_ctype_t = utf_conditional_t<
			Char,
			ham_expr_ref_utf8,
			ham_expr_ref_utf16,
			ham_expr_ref_utf32
		>;

		template<typename Char>
		using expr_unresolved_ctype_t = utf_conditional_t<
			Char,
			ham_expr_unresolved_utf8,
			ham_expr_unresolved_utf16,
			ham_expr_unresolved_utf32
		>;

		template<typename Char>
		using expr_fn_ctype_t = utf_conditional_t<
			Char,
			ham_expr_fn_utf8,
			ham_expr_fn_utf16,
			ham_expr_fn_utf32
		>;

		template<typename Char>
		using expr_call_ctype_t = utf_conditional_t<
			Char,
			ham_expr_call_utf8,
			ham_expr_call_utf16,
			ham_expr_call_utf32
		>;

		template<typename Char>
		using expr_block_ctype_t = utf_conditional_t<
			Char,
			ham_expr_block_utf8,
			ham_expr_block_utf16,
			ham_expr_block_utf32
		>;

		template<typename Char>
		using expr_unary_op_ctype_t = utf_conditional_t<
			Char,
			ham_expr_unary_op_utf8,
			ham_expr_unary_op_utf16,
			ham_expr_unary_op_utf32
		>;

		template<typename Char>
		using expr_binary_op_ctype_t = utf_conditional_t<
			Char,
			ham_expr_binary_op_utf8,
			ham_expr_binary_op_utf16,
			ham_expr_binary_op_utf32
		>;

		template<typename Char>
		using expr_lit_int_ctype_t = utf_conditional_t<
			Char,
			ham_expr_lit_int_utf8,
			ham_expr_lit_int_utf16,
			ham_expr_lit_int_utf32
		>;

		template<typename Char>
		using expr_lit_real_ctype_t = utf_conditional_t<
			Char,
			ham_expr_lit_real_utf8,
			ham_expr_lit_real_utf16,
			ham_expr_lit_real_utf32
		>;

		template<typename Char>
		using expr_lit_str_ctype_t = utf_conditional_t<
			Char,
			ham_expr_lit_str_utf8,
			ham_expr_lit_str_utf16,
			ham_expr_lit_str_utf32
		>;

		template<typename Char, expr_kind Kind> struct expr_type_from_kind;
		template<typename Char> struct expr_type_from_kind<Char, expr_kind::base>:       id<expr_base_ctype_t<Char>>{};
		template<typename Char> struct expr_type_from_kind<Char, expr_kind::error>:      id<expr_error_ctype_t<Char>>{};
		template<typename Char> struct expr_type_from_kind<Char, expr_kind::binding>:    id<expr_binding_ctype_t<Char>>{};
		template<typename Char> struct expr_type_from_kind<Char, expr_kind::ref>:        id<expr_ref_ctype_t<Char>>{};
		template<typename Char> struct expr_type_from_kind<Char, expr_kind::unresolved>: id<expr_unresolved_ctype_t<Char>>{};

		template<typename Char> struct expr_type_from_kind<Char, expr_kind::fn>:    id<expr_fn_ctype_t<Char>>{};
		template<typename Char> struct expr_type_from_kind<Char, expr_kind::call>:  id<expr_call_ctype_t<Char>>{};
		template<typename Char> struct expr_type_from_kind<Char, expr_kind::block>: id<expr_block_ctype_t<Char>>{};

		template<typename Char> struct expr_type_from_kind<Char, expr_kind::unary_op>:  id<expr_unary_op_ctype_t<Char>>{};
		template<typename Char> struct expr_type_from_kind<Char, expr_kind::binary_op>: id<expr_binary_op_ctype_t<Char>>{};

		template<typename Char> struct expr_type_from_kind<Char, expr_kind::lit_int>:  id<expr_lit_int_ctype_t<Char>>{};
		template<typename Char> struct expr_type_from_kind<Char, expr_kind::lit_real>: id<expr_lit_real_ctype_t<Char>>{};
		template<typename Char> struct expr_type_from_kind<Char, expr_kind::lit_str>:  id<expr_lit_str_ctype_t<Char>>{};

		template<typename Char, expr_kind Kind>
		using expr_type_from_kind_t = typename expr_type_from_kind<Char, Kind>::type;

		//
		// Context c api selectors
		//

		template<typename Char>
		constexpr inline auto parse_context_ctype_create = utf_conditional_t<
			Char,
			meta::static_fn<ham_parse_context_create_utf8>,
			meta::static_fn<ham_parse_context_create_utf16>,
			meta::static_fn<ham_parse_context_create_utf32>
		>{};

		template<typename Char>
		constexpr inline auto parse_context_ctype_destroy = utf_conditional_t<
			Char,
			meta::static_fn<ham_parse_context_destroy_utf8>,
			meta::static_fn<ham_parse_context_destroy_utf16>,
			meta::static_fn<ham_parse_context_destroy_utf32>
		>{};

		template<typename Char>
		constexpr inline auto parse_context_ctype_new_expr = utf_conditional_t<
			Char,
			meta::static_fn<ham_parse_context_new_expr_utf8>,
			meta::static_fn<ham_parse_context_new_expr_utf16>,
			meta::static_fn<ham_parse_context_new_expr_utf32>
		>{};

		template<typename Char>
		constexpr inline auto parse_context_ctype_new_error = utf_conditional_t<
			Char,
			meta::static_fn<ham_parse_context_new_error_utf8>,
			meta::static_fn<ham_parse_context_new_error_utf16>,
			meta::static_fn<ham_parse_context_new_error_utf32>
		>{};

		template<typename Char>
		constexpr inline auto parse_context_ctype_new_binding = utf_conditional_t<
			Char,
			meta::static_fn<ham_parse_context_new_binding_utf8>,
			meta::static_fn<ham_parse_context_new_binding_utf16>,
			meta::static_fn<ham_parse_context_new_binding_utf32>
		>{};

		template<typename Char>
		constexpr inline auto parse_context_ctype_new_ref = utf_conditional_t<
			Char,
			meta::static_fn<ham_parse_context_new_ref_utf8>,
			meta::static_fn<ham_parse_context_new_ref_utf16>,
			meta::static_fn<ham_parse_context_new_ref_utf32>
		>{};

		template<typename Char>
		constexpr inline auto parse_context_ctype_new_unresolved = utf_conditional_t<
			Char,
			meta::static_fn<ham_parse_context_new_unresolved_utf8>,
			meta::static_fn<ham_parse_context_new_unresolved_utf16>,
			meta::static_fn<ham_parse_context_new_unresolved_utf32>
		>{};

		template<typename Char>
		constexpr inline auto parse_context_ctype_new_fn = utf_conditional_t<
			Char,
			meta::static_fn<ham_parse_context_new_fn_utf8>,
			meta::static_fn<ham_parse_context_new_fn_utf16>,
			meta::static_fn<ham_parse_context_new_fn_utf32>
		>{};

		template<typename Char>
		constexpr inline auto parse_context_ctype_new_call = utf_conditional_t<
			Char,
			meta::static_fn<ham_parse_context_new_call_utf8>,
			meta::static_fn<ham_parse_context_new_call_utf16>,
			meta::static_fn<ham_parse_context_new_call_utf32>
		>{};

		template<typename Char>
		constexpr inline auto parse_context_ctype_new_block = utf_conditional_t<
			Char,
			meta::static_fn<ham_parse_context_new_block_utf8>,
			meta::static_fn<ham_parse_context_new_block_utf16>,
			meta::static_fn<ham_parse_context_new_block_utf32>
		>{};

		template<typename Char>
		constexpr inline auto parse_context_ctype_new_unary_op = utf_conditional_t<
			Char,
			meta::static_fn<ham_parse_context_new_unary_op_utf8>,
			meta::static_fn<ham_parse_context_new_unary_op_utf16>,
			meta::static_fn<ham_parse_context_new_unary_op_utf32>
		>{};

		template<typename Char>
		constexpr inline auto parse_context_ctype_new_binary_op = utf_conditional_t<
			Char,
			meta::static_fn<ham_parse_context_new_binary_op_utf8>,
			meta::static_fn<ham_parse_context_new_binary_op_utf16>,
			meta::static_fn<ham_parse_context_new_binary_op_utf32>
		>{};

		template<typename Char>
		constexpr inline auto parse_context_ctype_new_lit_int = utf_conditional_t<
			Char,
			meta::static_fn<ham_parse_context_new_lit_int_utf8>,
			meta::static_fn<ham_parse_context_new_lit_int_utf16>,
			meta::static_fn<ham_parse_context_new_lit_int_utf32>
		>{};

		template<typename Char>
		constexpr inline auto parse_context_ctype_new_lit_real = utf_conditional_t<
			Char,
			meta::static_fn<ham_parse_context_new_lit_real_utf8>,
			meta::static_fn<ham_parse_context_new_lit_real_utf16>,
			meta::static_fn<ham_parse_context_new_lit_real_utf32>
		>{};

		template<typename Char>
		constexpr inline auto parse_context_ctype_new_lit_str = utf_conditional_t<
			Char,
			meta::static_fn<ham_parse_context_new_lit_str_utf8>,
			meta::static_fn<ham_parse_context_new_lit_str_utf16>,
			meta::static_fn<ham_parse_context_new_lit_str_utf32>
		>{};

		//
		// Scope c api selectors
		//

		template<typename Char>
		constexpr inline auto parse_scope_ctype_create = utf_conditional_t<
			Char,
			meta::static_fn<ham_parse_scope_create_utf8>,
			meta::static_fn<ham_parse_scope_create_utf16>,
			meta::static_fn<ham_parse_scope_create_utf32>
		>{};

		template<typename Char>
		constexpr inline auto parse_scope_ctype_destroy = utf_conditional_t<
			Char,
			meta::static_fn<ham_parse_scope_destroy_utf8>,
			meta::static_fn<ham_parse_scope_destroy_utf16>,
			meta::static_fn<ham_parse_scope_destroy_utf32>
		>{};

		template<typename Char>
		constexpr inline auto parse_scope_ctype_parent = utf_conditional_t<
			Char,
			meta::static_fn<ham_parse_scope_parent_utf8>,
			meta::static_fn<ham_parse_scope_parent_utf16>,
			meta::static_fn<ham_parse_scope_parent_utf32>
		>{};

		template<typename Char>
		constexpr inline auto parse_scope_ctype_context = utf_conditional_t<
			Char,
			meta::static_fn<ham_parse_scope_context_utf8>,
			meta::static_fn<ham_parse_scope_context_utf16>,
			meta::static_fn<ham_parse_scope_context_utf32>
		>{};

		template<typename Char>
		constexpr inline auto parse_scope_ctype_get_indent = utf_conditional_t<
			Char,
			meta::static_fn<ham_parse_scope_get_indent_utf8>,
			meta::static_fn<ham_parse_scope_get_indent_utf16>,
			meta::static_fn<ham_parse_scope_get_indent_utf32>
		>{};

		template<typename Char>
		constexpr inline auto parse_scope_ctype_set_indent = utf_conditional_t<
			Char,
			meta::static_fn<ham_parse_scope_set_indent_utf8>,
			meta::static_fn<ham_parse_scope_set_indent_utf16>,
			meta::static_fn<ham_parse_scope_set_indent_utf32>
		>{};

		template<typename Char>
		constexpr inline auto parse_scope_ctype_bind = utf_conditional_t<
			Char,
			meta::static_fn<ham_parse_scope_bind_utf8>,
			meta::static_fn<ham_parse_scope_bind_utf16>,
			meta::static_fn<ham_parse_scope_bind_utf32>
		>{};

		template<typename Char>
		constexpr inline auto parse_scope_ctype_resolve = utf_conditional_t<
			Char,
			meta::static_fn<ham_parse_scope_resolve_utf8>,
			meta::static_fn<ham_parse_scope_resolve_utf16>,
			meta::static_fn<ham_parse_scope_resolve_utf32>
		>{};
	} // namespace detail
	//! @endcond

	template<typename Char, bool Mutable>
	class basic_parse_context_view{
		public:
			using ctype            = detail::parse_context_ctype_t<Char>;
			using str_type         = basic_str<Char>;
			using handle_type      = std::conditional_t<Mutable, ctype*, const ctype*>;
			using token_range_type = basic_token_range<Char>;

			template<expr_kind Kind = expr_kind::base, bool UMutable = false>
			using expr_type = basic_expr<Char, Kind, UMutable>;

			template<bool UMutable = false>
			using expr_iterator_type = basic_expr_iterator<Char, UMutable>;

			template<bool UMutable = false>
			using expr_range_type = basic_expr_range<Char, UMutable>;

			basic_parse_context_view(handle_type handle_ = nullptr) noexcept
				: m_handle(handle_){}

			operator handle_type() const noexcept{ return m_handle; }

			template<expr_kind Kind, bool Enable = Mutable>
			auto new_expr(const token_range_type &tokens = {})
				-> std::enable_if_t<Enable, basic_expr<Char, Kind, true>>
			{
				return (detail::expr_type_from_kind_t<Char, Kind>*)detail::parse_context_ctype_new_expr<Char>(handle(), static_cast<ham_expr_kind>(Kind), tokens);
			}

			template<typename ... Args, bool Enable = Mutable>
			auto new_error(const token_range_type &tokens, const expr_type<> &prev, const char *fmt_str, Args &&... args)
				-> std::enable_if_t<Enable, expr_type<expr_kind::error>>
			{
				return detail::parse_context_ctype_new_error<Char>(handle(), tokens, prev, fmt_str, std::forward<Args>(args)...);
			}

			template<typename ... Args, bool Enable = Mutable>
			auto new_error(const basic_expr<Char, expr_kind::base> &prev, const char *fmt_str, Args &&... args)
				-> std::enable_if_t<Enable, expr_type<expr_kind::error>>
			{
				return detail::parse_context_ctype_new_error<Char>(handle(), { nullptr, nullptr }, prev, fmt_str, std::forward<Args>(args)...);
			}

			template<bool Enable = Mutable>
			auto new_binding(const token_range_type &tokens, const str_type &name, const expr_type<> &value)
				-> std::enable_if_t<Enable, expr_type<expr_kind::binding>>
			{
				return detail::parse_context_ctype_new_binding<Char>(handle(), tokens, name, value);
			}

			template<bool Enable = Mutable>
			auto new_ref(const token_range_type &tokens, const expr_type<expr_kind::binding> &refed)
				-> std::enable_if_t<Enable, expr_type<expr_kind::ref>>
			{
				return detail::parse_context_ctype_new_ref<Char>(handle(), tokens, refed);
			}

			template<bool Enable = Mutable>
			auto new_unresolved(const token_range_type &tokens, const str_type &id)
				-> std::enable_if_t<Enable, expr_type<expr_kind::unresolved>>
			{
				return detail::parse_context_ctype_new_unresolved<Char>(handle(), tokens, id);
			}

			template<bool Enable = Mutable>
			auto new_fn(const token_range_type &tokens, const expr_range_type<false> &params, const expr_type<> &return_type)
					-> std::enable_if_t<Enable, expr_type<expr_kind::fn>>
			{
				return detail::parse_context_ctype_new_fn<Char>(handle(), tokens, params.size(), params.begin(), return_type);
			}

			template<bool Enable = Mutable>
			auto new_call(const token_range_type &tokens, const expr_type<> &fn, const expr_range_type<false> &args)
					-> std::enable_if_t<Enable, expr_type<expr_kind::call>>
			{
				return detail::parse_context_ctype_new_call<Char>(handle(), tokens, fn, args.size(), args.begin());
			}

			template<bool Enable = Mutable>
			auto new_block(const token_range_type &tokens, const expr_range_type<false> &exprs)
					-> std::enable_if_t<Enable, expr_type<expr_kind::block>>
			{
				return detail::parse_context_ctype_new_call<Char>(handle(), tokens, exprs.size(), exprs.begin());
			}

			template<bool Enable = Mutable>
			auto new_unary_op(const token_range_type &tokens, const str_type &op, const expr_type<> &expr)
					-> std::enable_if_t<Enable, expr_type<expr_kind::unary_op>>
			{
				return detail::parse_context_ctype_new_unary_op<Char>(handle(), tokens, op, expr);
			}

			template<bool Enable = Mutable>
			auto new_binary_op(const token_range_type &tokens, const str_type &op, const expr_type<> &lhs, const expr_type<> &rhs)
					-> std::enable_if_t<Enable, expr_type<expr_kind::binary_op>>
			{
				return detail::parse_context_ctype_new_binary_op<Char>(handle(), tokens, op, lhs, rhs);
			}

			template<bool Enable = Mutable>
			auto new_lit_int(const token_range_type &tokens, const str_type &value)
				-> std::enable_if_t<Enable, expr_type<expr_kind::lit_int>>
			{
				return detail::parse_context_ctype_new_lit_int<Char>(handle(), tokens, value);
			}

			template<bool Enable = Mutable>
			auto new_lit_real(const token_range_type &tokens, const str_type &value)
				-> std::enable_if_t<Enable, expr_type<expr_kind::lit_real>>
			{
				return detail::parse_context_ctype_new_lit_real<Char>(handle(), tokens, value);
			}

			template<bool Enable = Mutable>
			auto new_lit_str(const token_range_type &tokens, const str_type &value)
				-> std::enable_if_t<Enable, expr_type<expr_kind::lit_str>>
			{
				return detail::parse_context_ctype_new_lit_str<Char>(handle(), tokens, value);
			}

			handle_type handle() const noexcept{ return m_handle; }

		private:
			handle_type m_handle;
	};

	using parse_context_view_utf8  = basic_parse_context_view<char8>;
	using parse_context_view_utf16 = basic_parse_context_view<char16>;
	using parse_context_view_utf32 = basic_parse_context_view<char32>;

	using parse_context_view = basic_parse_context_view<uchar>;

	template<typename Char>
	class basic_parse_context{
		public:
			using ctype             = detail::parse_context_ctype_t<Char>;
			using cexpr_base_type   = detail::expr_base_ctype_t<Char>;
			using str_type          = basic_str<Char>;
			using handle_type       = ctype*;
			using const_handle_type = const ctype*;
			using token_range_type  = basic_token_range<Char>;

			template<expr_kind Kind = expr_kind::base, bool Mutable = false>
			using expr_type = basic_expr<Char, Kind, Mutable>;

			template<bool Mutable = false>
			using expr_iterator_type = basic_expr_iterator<Char, Mutable>;

			template<bool Mutable = false>
			using expr_range_type = basic_expr_range<Char, Mutable>;

			template<bool Mutable = false>
			using view_type = basic_parse_context_view<Char, Mutable>;

			explicit basic_parse_context(handle_type handle_ = nullptr) noexcept
				: m_handle(handle_ ? handle_ : detail::parse_context_ctype_create<Char>()){}

			basic_parse_context(basic_parse_context&&) noexcept = default;

			basic_parse_context &operator=(basic_parse_context&&) noexcept = default;

			operator bool() const& noexcept{ return (bool)m_handle; }

			operator bool() const&& = delete;

			operator view_type<true>() & noexcept{ return m_handle; }
			operator view_type<false>() const& noexcept{ return m_handle; }

			operator view_type<true>() && = delete;
			operator view_type<false>() const&& = delete;

			operator handle_type() & noexcept{ return m_handle.get(); }
			operator const_handle_type() const& noexcept{ return m_handle.get(); }

			operator const_handle_type() const&& = delete;

			template<expr_kind Kind>
			expr_type<Kind, true> new_expr(const token_range_type &tokens = {}){
				return (detail::expr_type_from_kind_t<Char, Kind>*)detail::parse_context_ctype_new_expr<Char>(handle(), Kind, tokens);
			}

			template<typename ... Args>
			expr_type<expr_kind::error> new_error(const token_range_type &tokens, const char *fmt_str, Args &&... args){
				return detail::parse_context_ctype_new_error<Char>(handle(), tokens, fmt_str, std::forward<Args>(args)...);
			}

			template<typename ... Args>
			expr_type<expr_kind::error> new_error(const char *fmt_str, Args &&... args){
				return detail::parse_context_ctype_new_error<Char>(handle(), { nullptr, nullptr }, fmt_str, std::forward<Args>(args)...);
			}

			expr_type<expr_kind::binding> new_binding(const token_range_type &tokens, const str_type &name, const expr_type<> &value){
				return detail::parse_context_ctype_new_binding<Char>(handle(), tokens, name, value);
			}

			expr_type<expr_kind::ref> new_ref(const token_range_type &tokens, const expr_type<expr_kind::binding> &refed){
				return detail::parse_context_ctype_new_ref<Char>(handle(), tokens, refed);
			}

			expr_type<expr_kind::unresolved> new_unresolved(const token_range_type &tokens, const str_type &id){
				return detail::parse_context_ctype_new_unresolved<Char>(handle(), tokens, id);
			}

			expr_type<expr_kind::fn> new_fn(const token_range_type &tokens, const expr_range_type<> &params, const expr_type<> &return_type){
				return detail::parse_context_ctype_new_fn<Char>(handle(), tokens, params.size(), params.begin(), return_type);
			}

			expr_type<expr_kind::call> new_call(const token_range_type &tokens, const expr_type<> &fn, const expr_range_type<> &args){
				return detail::parse_context_ctype_new_call<Char>(handle(), tokens, fn, args.size(), args.begin());
			}

			expr_type<expr_kind::block> new_block(const token_range_type &tokens, const expr_range_type<> &exprs){
				return detail::parse_context_ctype_new_block<Char>(handle(), tokens, exprs.size(), exprs.begin());
			}

			expr_type<expr_kind::unary_op> new_unary_op(const token_range_type &tokens, const str_type &op, const expr_type<> &expr){
				return detail::parse_context_ctype_new_unary_op<Char>(handle(), tokens, op, expr);
			}

			expr_type<expr_kind::binary_op> new_binary_op(const token_range_type &tokens, const str_type &op, const expr_type<> &lhs, const expr_type<> &rhs){
				return detail::parse_context_ctype_new_binary_op<Char>(handle(), tokens, op, lhs, rhs);
			}

			expr_type<expr_kind::lit_int> new_lit_int(const token_range_type &tokens, const str_type &val){
				return detail::parse_context_ctype_new_lit_int<Char>(handle(), tokens, val);
			}

			expr_type<expr_kind::lit_real> new_lit_real(const token_range_type &tokens, const str_type &val){
				return detail::parse_context_ctype_new_lit_real<Char>(handle(), tokens, val);
			}

			expr_type<expr_kind::lit_str> new_lit_str(const token_range_type &tokens, const str_type &val){
				return detail::parse_context_ctype_new_lit_str<Char>(handle(), tokens, val);
			}

			handle_type handle() noexcept{ return m_handle.get(); }
			const_handle_type handle() const noexcept{ return m_handle.get(); }

		private:
			unique_handle<handle_type, detail::parse_context_ctype_destroy<Char>> m_handle;
	};

	using parse_context_utf8  = basic_parse_context<char8>;
	using parse_context_utf16 = basic_parse_context<char16>;
	using parse_context_utf32 = basic_parse_context<char32>;

	using parse_context = basic_parse_context<uchar>;

	template<typename Char, bool Mutable>
	class basic_parse_scope_view{
		public:
			using ctype = detail::parse_scope_ctype_t<Char>;

			using str_type = basic_str<Char>;
			using handle_type = std::conditional_t<Mutable, ctype*, const ctype*>;

			template<expr_kind Kind = expr_kind::base, bool UMutable = false>
			using expr_type = basic_expr<Char, Kind, UMutable>;

			using context_view_type = basic_parse_context_view<Char, Mutable>;

			basic_parse_scope_view(handle_type handle_ = nullptr) noexcept
				: m_handle(handle_){}

			operator bool() const noexcept{ return m_handle != nullptr; }

			operator handle_type() const noexcept{ return m_handle; }

			context_view_type context() const noexcept{
				return detail::parse_scope_ctype_context<Char>(m_handle);
			}

			basic_parse_scope_view<Char, Mutable> parent() const noexcept{
				return detail::parse_scope_ctype_parent<Char>(m_handle);
			}

			template<bool Enable = Mutable>
			auto bind(const expr_type<expr_kind::binding> &binding) -> std::enable_if_t<Enable, bool>{
				return detail::parse_scope_ctype_bind<Char>(m_handle, binding);
			}

			str_type get_indent() const noexcept{
				return detail::parse_scope_ctype_get_indent<Char>(m_handle);
			}

			template<bool Enable = Mutable>
			auto set_indent(const str_type &new_indent) noexcept -> std::enable_if_t<Enable, bool>{
				return detail::parse_scope_ctype_set_indent<Char>(m_handle, new_indent);
			}

			expr_type<expr_kind::binding> resolve(str_type name) const noexcept{
				return detail::parse_scope_ctype_resolve<Char>(m_handle, name);
			}

			handle_type handle() const noexcept{ return m_handle; }

		private:
			handle_type m_handle;
	};

	using parse_scope_view_utf8  = basic_parse_scope_view<char8>;
	using parse_scope_view_utf16 = basic_parse_scope_view<char16>;
	using parse_scope_view_utf32 = basic_parse_scope_view<char32>;

	using parse_scope_view = basic_parse_scope_view<uchar>;

	template<typename Char>
	class basic_parse_scope{
		public:
			using ctype = detail::parse_scope_ctype_t<Char>;
			using cparse_context = detail::parse_context_ctype_t<Char>;

			using handle_type = ctype*;
			using const_handle_type = const ctype*;

			template<bool Mutable>
			using view_type = basic_parse_scope_view<Char, Mutable>;

			using str_type = basic_str<Char>;

			template<expr_kind Kind = expr_kind::base, bool Mutable = false>
			using expr_type = basic_expr<Char, Kind, Mutable>;

			using context_view_type = basic_parse_context_view<Char, true>;

			explicit basic_parse_scope(handle_type handle_) noexcept
				: m_handle(handle_){}

			explicit basic_parse_scope(cparse_context *ctx, const ctype *parent_ = nullptr)
				: m_handle(detail::parse_scope_ctype_create<Char>(ctx, parent_)){}

			explicit basic_parse_scope(basic_parse_context<Char> &ctx, const ctype *parent_ = nullptr)
				: basic_parse_scope(ctx.handle(), parent_){}

			explicit basic_parse_scope(basic_parse_scope &parent)
				: basic_parse_scope(parent.context(), parent.handle()){}

			basic_parse_scope(basic_parse_scope&&) noexcept = default;

			operator basic_parse_scope_view<Char, true>() & noexcept{ return m_handle; }
			operator basic_parse_scope_view<Char, false>() const& noexcept{ return m_handle; }

			operator basic_parse_scope_view<Char, true>() && = delete;
			operator basic_parse_scope_view<Char, false>() const&& = delete;

			operator bool() const& noexcept{ return (bool)m_handle; }

			operator bool() const&& = delete;

			operator handle_type() & noexcept{ return m_handle.get(); }
			operator const_handle_type() const& noexcept{ return m_handle.get(); }

			operator handle_type() && = delete;
			operator const_handle_type() const&& = delete;

			basic_parse_scope &operator=(basic_parse_scope&&) noexcept = default;

			view_type<true>  view()       noexcept{ return m_handle.get(); }
			view_type<false> view() const noexcept{ return m_handle.get(); }

			str_type get_indent() const noexcept{
				return detail::parse_scope_ctype_get_indent<Char>(m_handle.get());
			}

			bool set_indent(const str_type &new_indent) noexcept{
				return detail::parse_scope_ctype_set_indent<Char>(m_handle.get(), new_indent);
			}

			bool bind(const expr_type<expr_kind::binding> &binding){
				return detail::parse_scope_ctype_bind<Char>(m_handle.get(), binding);
			}

			expr_type<expr_kind::binding> resolve(str_type name) noexcept{
				return detail::parse_scope_ctype_resolve<Char>(m_handle.get(), name);
			}

			context_view_type context() const noexcept{
				return detail::parse_scope_ctype_context<Char>(m_handle.get());
			}

			handle_type handle() noexcept{ return m_handle.get(); }
			const_handle_type handle() const noexcept{ return m_handle.get(); }

		private:
			unique_handle<handle_type, detail::parse_scope_ctype_destroy<Char>> m_handle;
	};

	using parse_scope_utf8  = basic_parse_scope<char8>;
	using parse_scope_utf16 = basic_parse_scope<char16>;
	using parse_scope_utf32 = basic_parse_scope<char32>;

	using parse_scope = basic_parse_scope<uchar>;

	template<typename Char, bool Mutable>
	class basic_expr_iterator{
		public:
			using ptr_type = std::conditional_t<Mutable, basic_expr<Char, expr_kind::base, Mutable>*, const basic_expr<Char, expr_kind::base, Mutable>*>;
			using cptr_type = std::conditional_t<Mutable, detail::expr_base_ctype_t<Char>* const*, const detail::expr_base_ctype_t<Char>* const*>;

			basic_expr_iterator(const basic_expr_iterator&) noexcept = default;

			basic_expr_iterator(cptr_type cptr_it) noexcept
				: m_val{ .cptr = cptr_it }{}

			operator ptr_type()  const noexcept{ return m_val.ptr; }
			operator cptr_type() const noexcept{ return m_val.cptr; }

			basic_expr_iterator &operator=(const basic_expr_iterator&) noexcept = default;

			basic_expr<Char, expr_kind::base, Mutable> operator*() const noexcept{ return *m_val.ptr; }

			ptr_type operator->() const noexcept{ return ptr(); }

			ptr_type  ptr()  const noexcept{ return m_val.ptr; }
			cptr_type cptr() const noexcept{ return m_val.cptr; }

		private:
			union {
				ptr_type ptr;
				cptr_type cptr;
			} m_val;
	};

	template<typename Char, bool Mutable>
	class basic_expr_range{
		public:
			using iterator = basic_expr_iterator<Char, Mutable>;

			basic_expr_range() noexcept
				: m_beg{nullptr}, m_end{nullptr}{}

			basic_expr_range(iterator beg_, iterator end_) noexcept
				: m_beg{beg_}, m_end{end_}{}

			iterator begin() const noexcept{ return m_beg; }
			iterator end()   const noexcept{ return m_end; }

			usize size() const noexcept{ return ((uptr)m_end.cptr() - (uptr)m_beg.cptr())/sizeof(void*); }
			usize length() const noexcept{ return size(); }

		private:
			iterator m_beg, m_end;
	};

	//! @cond ignore
	namespace detail{
		template<typename Self, typename Char, expr_kind Kind, bool Mutable>
		class basic_expr_ext{};

		template<typename Self, typename Char, bool Mutable>
		class basic_expr_ext<Self, Char, expr_kind::binding, Mutable>{
			public:
				const Self *self() const noexcept{ return static_cast<const Self*>(this); }

				basic_str<Char> name() const noexcept{ return self()->handle()->name; }
				basic_expr<Char, expr_kind::base> value() const noexcept{ return self()->handle()->value; }
		};

		template<typename Self, typename Char, bool Mutable>
		class basic_expr_ext<Self, Char, expr_kind::error, Mutable>{
			public:
				Self *self() noexcept{ return static_cast<Self*>(this); }
				const Self *self() const noexcept{ return static_cast<const Self*>(this); }

				str8 message() const noexcept{ return self()->handle()->message; }

				template<bool Enable = Mutable>
				auto set_message(const str8 &new_message) -> std::enable_if_t<Enable>{
					self()->handle()->message = new_message;
				}
		};

		template<typename Self, typename Char, bool Mutable>
		class basic_expr_ext<Self, Char, expr_kind::ref, Mutable>{
			public:
				const Self *self() const noexcept{ return static_cast<const Self*>(this); }

				basic_expr<Char, expr_kind::binding> refed() const noexcept{ return self()->handle()->refed; }
		};

		template<typename Self, typename Char, bool Mutable>
		class basic_expr_ext<Self, Char, expr_kind::unresolved, Mutable>{
			public:
				const Self *self() const noexcept{ return static_cast<const Self*>(this); }

				basic_str<Char> id() const noexcept{ return self()->handle()->id; }
		};
	}
	//! @endcond

	template<typename Char, expr_kind Kind, bool Mutable>
	class basic_expr: public detail::basic_expr_ext<basic_expr<Char, Kind, Mutable>, Char, Kind, Mutable>{
		public:
			enum{ is_mutable = Mutable };

			using ctype = detail::expr_type_from_kind_t<Char, Kind>;

			using cexpr_base_type = std::conditional_t<
				Mutable, detail::expr_base_ctype_t<Char>, const detail::expr_base_ctype_t<Char>
			>;

			using handle_type = std::conditional_t<Mutable, ctype*, const ctype*>;

			basic_expr(handle_type handle_ = nullptr) noexcept
				: m_expr(handle_){}

			template<expr_kind SelfKind = Kind>
			operator std::enable_if_t<SelfKind != expr_kind::base, basic_expr<Char, expr_kind::base, Mutable>>() const noexcept{
				return (cexpr_base_type*)m_expr;
			}

			operator bool() const noexcept{ return m_expr != nullptr; }

			operator handle_type() const noexcept{ return m_expr; }

			basic_token_range<Char> tokens() const noexcept{
				if constexpr(Kind == expr_kind::base){
					return m_expr->tokens;
				}
				else{
					return m_expr->super.tokens;
				}
			}

			bool is_of_kind(expr_kind kind_) const noexcept{
				if constexpr(Kind == expr_kind::base){
					return kind() == kind_;
				}
				else{
					return Kind == kind_;
				}
			}

			bool is_error()      const noexcept{ return is_of_kind(expr_kind::error); }
			bool is_binding()    const noexcept{ return is_of_kind(expr_kind::binding); }
			bool is_ref()        const noexcept{ return is_of_kind(expr_kind::ref); }
			bool is_unresolved() const noexcept{ return is_of_kind(expr_kind::unresolved); }
			bool is_fn()         const noexcept{ return is_of_kind(expr_kind::fn); }
			bool is_call()       const noexcept{ return is_of_kind(expr_kind::call); }
			bool is_block()      const noexcept{ return is_of_kind(expr_kind::block); }

			bool is_unary_op()   const noexcept{ return is_of_kind(expr_kind::unary_op); }
			bool is_binary_op()  const noexcept{ return is_of_kind(expr_kind::binary_op); }

			bool is_lit_int()  const noexcept{ return is_of_kind(expr_kind::lit_int); }
			bool is_lit_real() const noexcept{ return is_of_kind(expr_kind::lit_real); }
			bool is_lit_str()  const noexcept{ return is_of_kind(expr_kind::lit_str); }

			handle_type handle() const noexcept{ return m_expr; }

			expr_kind kind() const noexcept{
				if constexpr(Kind == expr_kind::base){
					return static_cast<expr_kind>(m_expr->kind);
				}
				else{
					return static_cast<expr_kind>(m_expr->super.kind);
				}
			}

		private:
			handle_type m_expr;
	};

	template<expr_kind Kind, bool Mutable = false>
	using expr_utf8 = basic_expr<char8, Kind, Mutable>;

	using expr_base_utf8       = expr_utf8<expr_kind::base>;
	using expr_error_utf8      = expr_utf8<expr_kind::error>;
	using expr_binding_utf8    = expr_utf8<expr_kind::binding>;
	using expr_ref_utf8        = expr_utf8<expr_kind::ref>;
	using expr_unresolved_utf8 = expr_utf8<expr_kind::unresolved>;
	using expr_fn_utf8         = expr_utf8<expr_kind::fn>;
	using expr_call_utf8       = expr_utf8<expr_kind::call>;
	using expr_block_utf8      = expr_utf8<expr_kind::block>;

	using expr_unary_op_utf8  = expr_utf8<expr_kind::unary_op>;
	using expr_binary_op_utf8 = expr_utf8<expr_kind::binary_op>;

	using expr_lit_int_utf8  = expr_utf8<expr_kind::lit_int>;
	using expr_lit_real_utf8 = expr_utf8<expr_kind::lit_real>;
	using expr_lit_str_utf8  = expr_utf8<expr_kind::lit_str>;

	template<expr_kind Kind, bool Mutable = false>
	using expr_utf16 = basic_expr<char16, Kind, Mutable>;

	using expr_base_utf16       = expr_utf16<expr_kind::base>;
	using expr_error_utf16      = expr_utf16<expr_kind::error>;
	using expr_binding_utf16    = expr_utf16<expr_kind::binding>;
	using expr_ref_utf16        = expr_utf16<expr_kind::ref>;
	using expr_unresolved_utf16 = expr_utf16<expr_kind::unresolved>;
	using expr_fn_utf16         = expr_utf16<expr_kind::fn>;
	using expr_call_utf16       = expr_utf16<expr_kind::call>;
	using expr_block_utf16      = expr_utf16<expr_kind::block>;

	using expr_unary_op_utf16  = expr_utf16<expr_kind::unary_op>;
	using expr_binary_op_utf16 = expr_utf16<expr_kind::binary_op>;

	using expr_lit_int_utf16  = expr_utf16<expr_kind::lit_int>;
	using expr_lit_real_utf16 = expr_utf16<expr_kind::lit_real>;
	using expr_lit_str_utf16  = expr_utf16<expr_kind::lit_str>;

	template<expr_kind Kind, bool Mutable = false>
	using expr_utf32 = basic_expr<char32, Kind, Mutable>;

	using expr_base_utf32       = expr_utf32<expr_kind::base>;
	using expr_error_utf32      = expr_utf32<expr_kind::error>;
	using expr_binding_utf32    = expr_utf32<expr_kind::binding>;
	using expr_ref_utf32        = expr_utf32<expr_kind::ref>;
	using expr_unresolved_utf32 = expr_utf32<expr_kind::unresolved>;
	using expr_fn_utf32         = expr_utf32<expr_kind::fn>;
	using expr_call_utf32       = expr_utf32<expr_kind::call>;
	using expr_block_utf32      = expr_utf32<expr_kind::block>;

	using expr_unary_op_utf32  = expr_utf32<expr_kind::unary_op>;
	using expr_binary_op_utf32 = expr_utf32<expr_kind::binary_op>;

	using expr_lit_int_utf32  = expr_utf32<expr_kind::lit_int>;
	using expr_lit_real_utf32 = expr_utf32<expr_kind::lit_real>;
	using expr_lit_str_utf32  = expr_utf32<expr_kind::lit_str>;

	template<expr_kind Kind, bool Mutable = false>
	using expr = basic_expr<uchar, Kind, Mutable>;

	using expr_base       = expr<expr_kind::base>;
	using expr_error      = expr<expr_kind::error>;
	using expr_binding    = expr<expr_kind::binding>;
	using expr_ref        = expr<expr_kind::ref>;
	using expr_unresolved = expr<expr_kind::unresolved>;
	using expr_fn         = expr<expr_kind::fn>;
	using expr_call       = expr<expr_kind::call>;
	using expr_block      = expr<expr_kind::block>;

	using expr_unary_op  = expr<expr_kind::unary_op>;
	using expr_binary_op = expr<expr_kind::binary_op>;

	using expr_lit_int  = expr<expr_kind::lit_int>;
	using expr_lit_real = expr<expr_kind::lit_real>;
	using expr_lit_str  = expr<expr_kind::lit_str>;

	//! @cond ignore
	namespace detail{
		template<typename Char>
		constexpr inline auto cparse = utf_conditional_t<
			Char,
			meta::static_fn<ham_parse_utf8>,
			meta::static_fn<ham_parse_utf16>,
			meta::static_fn<ham_parse_utf32>
		>{};
	}
	//! @endcond

	template<typename Char>
	static inline basic_expr<Char, expr_kind::base> parse(const basic_parse_context_view<Char, true> &ctx, const basic_token_range<Char> &tokens){
		return detail::cparse<Char>(ctx, tokens);
	}

	template<typename Char>
	static inline basic_expr<Char, expr_kind::base> parse(basic_parse_context<Char> &ctx, const basic_token_range<Char> &tokens){
		return detail::cparse<Char>(ctx.handle(), tokens);
	}
}

#endif // __cplusplus

/**
 * @}
 */

#endif // !HAM_PARSE_H
