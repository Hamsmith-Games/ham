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
 * GNU General Public License for more details.
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
#include "memory.h"

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

ham_api ham_parse_scope_utf8  *ham_parse_scope_create_utf8 (ham_parse_context_utf8  *ctx, const ham_parse_scope_utf8  *parent);
ham_api ham_parse_scope_utf16 *ham_parse_scope_create_utf16(ham_parse_context_utf16 *ctx, const ham_parse_scope_utf16 *parent);
ham_api ham_parse_scope_utf32 *ham_parse_scope_create_utf32(ham_parse_context_utf32 *ctx, const ham_parse_scope_utf32 *parent);

#define HAM_PARSE_SCOPE_CREATE_UTF(n) HAM_CONCAT(ham_parse_scope_create_utf, n)

ham_api void ham_parse_scope_destroy_utf8 (ham_parse_scope_utf8  *scope);
ham_api void ham_parse_scope_destroy_utf16(ham_parse_scope_utf16 *scope);
ham_api void ham_parse_scope_destroy_utf32(ham_parse_scope_utf32 *scope);

#define HAM_PARSE_SCOPE_DESTROY_UTF(n) HAM_CONCAT(ham_parse_scope_destroy_utf, n)

ham_api ham_parse_context_utf8  *ham_parse_scope_context_utf8 (const ham_parse_scope_utf8  *scope);
ham_api ham_parse_context_utf16 *ham_parse_scope_context_utf16(const ham_parse_scope_utf16 *scope);
ham_api ham_parse_context_utf32 *ham_parse_scope_context_utf32(const ham_parse_scope_utf32 *scope);

#define HAM_PARSE_SCOPE_CONTEXT_UTF(n) HAM_CONCAT(ham_parse_scope_context_utf, n)

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
#define ham_parse_scope_context HAM_PARSE_SCOPE_CONTEXT_UTF(HAM_UTF)
#define ham_parse_scope_get_indent HAM_PARSE_SCOPE_GET_INDENT_UTF(HAM_UTF)
#define ham_parse_scope_set_indent HAM_PARSE_SCOPE_SET_INDENT_UTF(HAM_UTF)
#define ham_parse_scope_bind HAM_PARSE_SCOPE_BIND_UTF(HAM_UTF)
#define ham_parse_scope_resolve HAM_PARSE_SCOPE_RESOLVE_UTF(HAM_UTF)

#define ham_parse_context_new_expr HAM_PARSE_CONTEXT_NEW_EXPR_UTF(HAM_UTF)
#define ham_parse_context_new_error HAM_PARSE_CONTEXT_NEW_ERROR_UTF(HAM_UTF)
#define ham_parse_context_new_binding HAM_PARSE_CONTEXT_NEW_BINDING_UTF(HAM_UTF)
#define ham_parse_context_new_ref HAM_PARSE_CONTEXT_NEW_REF_UTF(HAM_UTF)
#define ham_parse_context_new_unresolved HAM_PARSE_CONTEXT_NEW_UNRESOLVED_UTF(HAM_UTF)
#define ham_parse_context_new_lit_int HAM_PARSE_CONTEXT_NEW_LIT_INT_UTF(HAM_UTF)
#define ham_parse_context_new_lit_real HAM_PARSE_CONTEXT_NEW_LIT_REAL_UTF(HAM_UTF)
#define ham_parse_context_new_lit_str HAM_PARSE_CONTEXT_NEW_LIT_STR_UTF(HAM_UTF)

#define ham_parse HAM_PARSE_UTF(HAM_UTF)

HAM_C_API_END

#ifdef __cplusplus

namespace ham{
	enum class expr_kind: std::underlying_type_t<ham_expr_kind>{
		error      = HAM_EXPR_ERROR,
		binding    = HAM_EXPR_BINDING,
		ref        = HAM_EXPR_REF,
		unresolved = HAM_EXPR_UNRESOLVED,

		lit_int  = HAM_EXPR_LIT_INT,
		lit_real = HAM_EXPR_LIT_REAL,
		lit_str  = HAM_EXPR_LIT_STR,

		base, // special c++ only kind
	};

	template<typename Char, bool Mutable = true>
	class basic_parse_context_view;

	template<typename Char>
	class basic_parse_context;

	template<typename Char>
	class basic_parse_scope;

	template<typename Char, bool Mutable = true>
	class basic_parse_scope_view;

	template<typename Char, expr_kind Kind, bool Mutable = false>
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
			static_fn<ham_parse_context_create_utf8>,
			static_fn<ham_parse_context_create_utf16>,
			static_fn<ham_parse_context_create_utf32>
		>{};

		template<typename Char>
		constexpr inline auto parse_context_ctype_destroy = utf_conditional_t<
			Char,
			static_fn<ham_parse_context_destroy_utf8>,
			static_fn<ham_parse_context_destroy_utf16>,
			static_fn<ham_parse_context_destroy_utf32>
		>{};

		template<typename Char>
		constexpr inline auto parse_context_ctype_new_expr = utf_conditional_t<
			Char,
			static_fn<ham_parse_context_new_expr_utf8>,
			static_fn<ham_parse_context_new_expr_utf16>,
			static_fn<ham_parse_context_new_expr_utf32>
		>{};

		template<typename Char>
		constexpr inline auto parse_context_ctype_new_error = utf_conditional_t<
			Char,
			static_fn<ham_parse_context_new_error_utf8>,
			static_fn<ham_parse_context_new_error_utf16>,
			static_fn<ham_parse_context_new_error_utf32>
		>{};

		template<typename Char>
		constexpr inline auto parse_context_ctype_new_binding = utf_conditional_t<
			Char,
			static_fn<ham_parse_context_new_binding_utf8>,
			static_fn<ham_parse_context_new_binding_utf16>,
			static_fn<ham_parse_context_new_binding_utf32>
		>{};

		template<typename Char>
		constexpr inline auto parse_context_ctype_new_ref = utf_conditional_t<
			Char,
			static_fn<ham_parse_context_new_ref_utf8>,
			static_fn<ham_parse_context_new_ref_utf16>,
			static_fn<ham_parse_context_new_ref_utf32>
		>{};

		template<typename Char>
		constexpr inline auto parse_context_ctype_new_unresolved = utf_conditional_t<
			Char,
			static_fn<ham_parse_context_new_unresolved_utf8>,
			static_fn<ham_parse_context_new_unresolved_utf16>,
			static_fn<ham_parse_context_new_unresolved_utf32>
		>{};

		template<typename Char>
		constexpr inline auto parse_context_ctype_new_lit_int = utf_conditional_t<
			Char,
			static_fn<ham_parse_context_new_lit_int_utf8>,
			static_fn<ham_parse_context_new_lit_int_utf16>,
			static_fn<ham_parse_context_new_lit_int_utf32>
		>{};

		template<typename Char>
		constexpr inline auto parse_context_ctype_new_lit_real = utf_conditional_t<
			Char,
			static_fn<ham_parse_context_new_lit_real_utf8>,
			static_fn<ham_parse_context_new_lit_real_utf16>,
			static_fn<ham_parse_context_new_lit_real_utf32>
		>{};

		template<typename Char>
		constexpr inline auto parse_context_ctype_new_lit_str = utf_conditional_t<
			Char,
			static_fn<ham_parse_context_new_lit_str_utf8>,
			static_fn<ham_parse_context_new_lit_str_utf16>,
			static_fn<ham_parse_context_new_lit_str_utf32>
		>{};

		//
		// Scope c api selectors
		//

		template<typename Char>
		constexpr inline auto parse_scope_ctype_create = utf_conditional_t<
			Char,
			static_fn<ham_parse_scope_create_utf8>,
			static_fn<ham_parse_scope_create_utf16>,
			static_fn<ham_parse_scope_create_utf32>
		>{};

		template<typename Char>
		constexpr inline auto parse_scope_ctype_destroy = utf_conditional_t<
			Char,
			static_fn<ham_parse_scope_destroy_utf8>,
			static_fn<ham_parse_scope_destroy_utf16>,
			static_fn<ham_parse_scope_destroy_utf32>
		>{};

		template<typename Char>
		constexpr inline auto parse_scope_ctype_context = utf_conditional_t<
			Char,
			static_fn<ham_parse_scope_context_utf8>,
			static_fn<ham_parse_scope_context_utf16>,
			static_fn<ham_parse_scope_context_utf32>
		>{};

		template<typename Char>
		constexpr inline auto parse_scope_ctype_get_indent = utf_conditional_t<
			Char,
			static_fn<ham_parse_scope_get_indent_utf8>,
			static_fn<ham_parse_scope_get_indent_utf16>,
			static_fn<ham_parse_scope_get_indent_utf32>
		>{};

		template<typename Char>
		constexpr inline auto parse_scope_ctype_set_indent = utf_conditional_t<
			Char,
			static_fn<ham_parse_scope_set_indent_utf8>,
			static_fn<ham_parse_scope_set_indent_utf16>,
			static_fn<ham_parse_scope_set_indent_utf32>
		>{};

		template<typename Char>
		constexpr inline auto parse_scope_ctype_bind = utf_conditional_t<
			Char,
			static_fn<ham_parse_scope_bind_utf8>,
			static_fn<ham_parse_scope_bind_utf16>,
			static_fn<ham_parse_scope_bind_utf32>
		>{};

		template<typename Char>
		constexpr inline auto parse_scope_ctype_resolve = utf_conditional_t<
			Char,
			static_fn<ham_parse_scope_resolve_utf8>,
			static_fn<ham_parse_scope_resolve_utf16>,
			static_fn<ham_parse_scope_resolve_utf32>
		>{};

		template<typename Self, typename Char, expr_kind Kind>
		class basic_expr_ext{};

		template<typename Self, typename Char>
		class basic_expr_ext<Self, Char, expr_kind::binding>{
			public:
				const Self *self() const noexcept{ return static_cast<Self*>(this); }

				basic_str<Char> name() const noexcept{ return self()->handle()->name; }
				basic_expr<Char, expr_kind::base> value() const noexcept{ return self()->handle()->value; }
		};

		template<typename Self, typename Char>
		class basic_expr_ext<Self, Char, expr_kind::ref>{
			public:
				const Self *self() const noexcept{ return static_cast<Self*>(this); }

				basic_expr<Char, expr_kind::binding> refed() const noexcept{ return self()->handle()->refed; }
		};

		template<typename Self, typename Char>
		class basic_expr_ext<Self, Char, expr_kind::unresolved>{
			public:
				const Self *self() const noexcept{ return static_cast<Self*>(this); }

				basic_str<Char> id() const noexcept{ return self()->handle()->id; }
		};
	} // namespace detail
	//! @endcond

	template<typename Char, bool Mutable>
	class basic_parse_context_view{
		public:
			using ctype = detail::parse_context_ctype_t<Char>;

			using str_type = basic_str<Char>;

			using handle_type = std::conditional_t<Mutable, ctype*, const ctype*>;

			using token_range_type = basic_token_range<Char>;

			basic_parse_context_view(handle_type handle_ = nullptr) noexcept
				: m_handle(handle_){}

			operator handle_type() const noexcept{ return m_handle; }

			template<expr_kind Kind>
			auto new_expr(const token_range_type &tokens = {})
				-> std::enable_if_t<Mutable, basic_expr<Char, Kind, true>>
			{
				return (detail::expr_type_from_kind_t<Char, Kind>*)detail::parse_context_ctype_new_expr<Char>(handle(), Kind, tokens);
			}

			template<typename ... Args>
			auto new_error(const token_range_type &tokens, const basic_expr<Char, expr_kind::base> &prev, const char *fmt_str, Args &&... args)
				-> std::enable_if_t<Mutable, basic_expr<Char, expr_kind::error>>
			{
				return detail::parse_context_ctype_new_error<Char>(handle(), tokens, prev, fmt_str, std::forward<Args>(args)...);
			}

			template<typename ... Args>
			auto new_error(const basic_expr<Char, expr_kind::base> &prev, const char *fmt_str, Args &&... args)
				-> std::enable_if_t<Mutable, basic_expr<Char, expr_kind::error>>
			{
				return detail::parse_context_ctype_new_error<Char>(handle(), { nullptr, nullptr }, prev, fmt_str, std::forward<Args>(args)...);
			}

			template<bool Enable = Mutable>
			auto new_binding(const token_range_type &tokens, const str_type &name, const basic_expr<Char, expr_kind::base> &value)
				-> std::enable_if_t<Enable, basic_expr<Char, expr_kind::binding>>
			{
				return detail::parse_context_ctype_new_binding<Char>(handle(), tokens, name, value);
			}

			template<bool Enable = Mutable>
			auto new_ref(const token_range_type &tokens, const basic_expr<Char, expr_kind::binding> &refed)
				-> std::enable_if_t<Enable, basic_expr<Char, expr_kind::ref>>
			{
				return detail::parse_context_ctype_new_ref<Char>(handle(), tokens, refed);
			}

			template<bool Enable = Mutable>
			auto new_unresolved(const token_range_type &tokens, const str_type &id)
				-> std::enable_if_t<Enable, basic_expr<Char, expr_kind::unresolved>>
			{
				return detail::parse_context_ctype_new_unresolved<Char>(handle(), tokens, id);
			}

			template<bool Enable = Mutable>
			auto new_lit_int(const token_range_type &tokens, const str_type &value)
				-> std::enable_if_t<Enable, basic_expr<Char, expr_kind::lit_int>>
			{
				return detail::parse_context_ctype_new_lit_int<Char>(handle(), tokens, value);
			}

			template<bool Enable = Mutable>
			auto new_lit_real(const token_range_type &tokens, const str_type &value)
				-> std::enable_if_t<Enable, basic_expr<Char, expr_kind::lit_real>>
			{
				return detail::parse_context_ctype_new_lit_real<Char>(handle(), tokens, value);
			}

			template<bool Enable = Mutable>
			auto new_lit_str(const token_range_type &tokens, const str_type &value)
				-> std::enable_if_t<Enable, basic_expr<Char, expr_kind::lit_str>>
			{
				return detail::parse_context_ctype_new_lit_str<Char>(handle(), tokens, value);
			}

			handle_type handle() const noexcept{ return m_handle; }

		private:
			handle_type m_handle;
	};

	template<typename Char>
	class basic_parse_context{
		public:
			using ctype = detail::parse_context_ctype_t<Char>;

			using str_type = basic_str<Char>;
			using handle_type = ctype*;
			using const_handle_type = const ctype*;

			using token_range_type = basic_token_range<Char>;

			explicit basic_parse_context(handle_type handle_ = nullptr) noexcept
				: m_handle(handle_){}

			basic_parse_context(basic_parse_context&&) noexcept = default;

			basic_parse_context &operator=(basic_parse_context&&) noexcept = default;

			operator bool() const& noexcept{ return (bool)m_handle; }

			operator bool() const&& = delete;

			operator handle_type() & noexcept{ return m_handle.get(); }
			operator const_handle_type() const& noexcept{ return m_handle.get(); }

			operator const_handle_type() const&& = delete;

			template<expr_kind Kind>
			basic_expr<Char, Kind, true> new_expr(const token_range_type &tokens = {}){
				return (detail::expr_type_from_kind_t<Char, Kind>*)detail::parse_context_ctype_new_expr<Char>(handle(), Kind, tokens);
			}

			template<typename ... Args>
			basic_expr<Char, expr_kind::error> new_error(const token_range_type &tokens, const char *fmt_str, Args &&... args){
				return detail::parse_context_ctype_new_error<Char>(handle(), tokens, fmt_str, std::forward<Args>(args)...);
			}

			template<typename ... Args>
			basic_expr<Char, expr_kind::error> new_error(const char *fmt_str, Args &&... args){
				return detail::parse_context_ctype_new_error<Char>(handle(), { nullptr, nullptr }, fmt_str, std::forward<Args>(args)...);
			}

			basic_expr<Char, expr_kind::binding> new_binding(const token_range_type &tokens, const str_type &name, const basic_expr<Char, expr_kind::base> &value){
				return detail::parse_context_ctype_new_binding<Char>(handle(), tokens, name, value);
			}

			basic_expr<Char, expr_kind::ref> new_ref(const token_range_type &tokens, const basic_expr<Char, expr_kind::binding> &refed){
				return detail::parse_context_ctype_new_ref<Char>(handle(), tokens, refed);
			}

			basic_expr<Char, expr_kind::unresolved> new_unresolved(const token_range_type &tokens, const str_type &id){
				return detail::parse_context_ctype_new_unresolved<Char>(handle(), tokens, id);
			}

			basic_expr<Char, expr_kind::lit_int> new_lit_int(const token_range_type &tokens, const str_type &val){
				return detail::parse_context_ctype_new_lit_int<Char>(handle(), tokens, val);
			}

			basic_expr<Char, expr_kind::lit_real> new_lit_real(const token_range_type &tokens, const str_type &val){
				return detail::parse_context_ctype_new_lit_real<Char>(handle(), tokens, val);
			}

			basic_expr<Char, expr_kind::lit_str> new_lit_str(const token_range_type &tokens, const str_type &val){
				return detail::parse_context_ctype_new_lit_str<Char>(handle(), tokens, val);
			}

			handle_type handle() noexcept{ return m_handle.get(); }
			const_handle_type handle() const noexcept{ return m_handle.get(); }

		private:
			unique_handle<handle_type, detail::parse_context_ctype_destroy<Char>> m_handle;
	};

	template<typename Char, bool Mutable>
	class basic_parse_scope_view{
		public:
			using ctype = detail::parse_scope_ctype_t<Char>;
			using expr_binding_type = basic_expr<Char, expr_kind::binding>;
			using str_type = basic_str<Char>;
			using handle_type = std::conditional_t<Mutable, ctype*, const ctype*>;
			using context_view_type = basic_parse_context_view<Char, Mutable>;

			basic_parse_scope_view(handle_type handle_ = nullptr) noexcept
				: m_handle(handle_){}

			operator bool() const noexcept{ return m_handle != nullptr; }

			operator handle_type() const noexcept{ return m_handle; }

			context_view_type context() const noexcept{
				return detail::parse_scope_ctype_context<Char>(m_handle);
			}

			template<bool Enable = Mutable>
			auto bind(expr_binding_type binding) -> std::enable_if_t<Enable, bool>{
				return detail::parse_scope_ctype_bind<Char>(m_handle, binding);
			}

			str_type get_indent() const noexcept{
				return detail::parse_scope_ctype_get_indent<Char>(m_handle);
			}

			template<bool Enable = Mutable>
			auto set_indent(const str_type &new_indent) noexcept -> std::enable_if_t<Enable, bool>{
				return detail::parse_scope_ctype_set_indent<Char>(m_handle, new_indent);
			}

			expr_binding_type resolve(str_type name) const noexcept{
				return detail::parse_scope_ctype_resolve<Char>(m_handle, name);
			}

			handle_type handle() const noexcept{ return m_handle; }

		private:
			handle_type m_handle;
	};

	template<typename Char>
	class basic_parse_scope{
		public:
			using ctype = detail::parse_scope_ctype_t<Char>;
			using cparse_context = detail::parse_context_ctype_t<Char>;
			using expr_binding = basic_expr<Char, expr_kind::binding>;

			using context_view_type = basic_parse_context_view<Char, true>;

			using str_type = basic_str<Char>;

			using handle_type = ctype*;
			using const_handle_type = const ctype*;

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

			basic_parse_scope_view<Char, true> view() noexcept{ return m_handle.get(); }
			basic_parse_scope_view<Char, false> view() const noexcept{ return m_handle.get(); }

			str_type get_indent() const noexcept{
				return detail::parse_scope_ctype_get_indent<Char>(m_handle.get());
			}

			bool set_indent(const str_type &new_indent) noexcept{
				return detail::parse_scope_ctype_set_indent<Char>(m_handle.get(), new_indent);
			}

			bool bind(expr_binding binding){
				return detail::parse_scope_ctype_bind<Char>(m_handle.get(), binding);
			}

			expr_binding resolve(str_type name) noexcept{
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

	template<typename Char, expr_kind Kind, bool Mutable>
	class basic_expr: public detail::basic_expr_ext<basic_expr<Char, Kind>, Char, Kind>{
		public:
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

			handle_type handle() const noexcept{ return m_expr; }

			expr_kind kind() const noexcept{
				if constexpr(Kind == expr_kind::base){
					return m_expr->kind;
				}
				else{
					return m_expr->super.kind;
				}
			}

		private:
			handle_type m_expr;
	};

	using parse_context_view_utf8  = basic_parse_context_view<char8>;
	using parse_context_view_utf16 = basic_parse_context_view<char16>;
	using parse_context_view_utf32 = basic_parse_context_view<char32>;

	using parse_context_view = basic_parse_context_view<uchar>;

	using parse_context_utf8  = basic_parse_context<char8>;
	using parse_context_utf16 = basic_parse_context<char16>;
	using parse_context_utf32 = basic_parse_context<char32>;

	using parse_context = basic_parse_context<uchar>;

	using parse_scope_view_utf8  = basic_parse_scope_view<char8>;
	using parse_scope_view_utf16 = basic_parse_scope_view<char16>;
	using parse_scope_view_utf32 = basic_parse_scope_view<char32>;

	using parse_scope_view = basic_parse_scope_view<uchar>;

	using parse_scope_utf8  = basic_parse_scope<char8>;
	using parse_scope_utf16 = basic_parse_scope<char16>;
	using parse_scope_utf32 = basic_parse_scope<char32>;

	using parse_scope = basic_parse_scope<uchar>;

	template<expr_kind Kind, bool Mutable = false>
	using expr_utf8 = basic_expr<char8, Kind, Mutable>;

	using expr_base_utf8       = expr_utf8<expr_kind::base>;
	using expr_error_utf8      = expr_utf8<expr_kind::error>;
	using expr_binding_utf8    = expr_utf8<expr_kind::binding>;
	using expr_ref_utf8        = expr_utf8<expr_kind::ref>;
	using expr_unresolved_utf8 = expr_utf8<expr_kind::unresolved>;

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

	using expr_lit_int  = expr<expr_kind::lit_int>;
	using expr_lit_real = expr<expr_kind::lit_real>;
	using expr_lit_str  = expr<expr_kind::lit_str>;
}

#endif // __cplusplus

/**
 * @}
 */

#endif // !HAM_PARSE_H
