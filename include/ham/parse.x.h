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

#ifndef HAM_PARSE_X_H
#define HAM_PARSE_X_H 1

#ifndef HAM_PARSE_X_UTF
#	error "HAM_PARSE_X_UTF not defined before including parse.x.h"
#endif

#define HAM_PARSE_X_STR HAM_STR_UTF(HAM_PARSE_X_UTF)
#define HAM_PARSE_X_TOKEN HAM_TOKEN_UTF(HAM_PARSE_X_UTF)
#define HAM_PARSE_X_TOKEN_RANGE HAM_TOKEN_RANGE_UTF(HAM_PARSE_X_UTF)

#define HAM_PARSE_X_PARSE_CONTEXT HAM_PARSE_CONTEXT_UTF(HAM_PARSE_X_UTF)
#define HAM_PARSE_X_PARSE_SCOPE HAM_PARSE_SCOPE_UTF(HAM_PARSE_X_UTF)

#define HAM_PARSE_X_EXPR_U HAM_EXPR_UTF(HAM_PARSE_X_UTF, u)
#define HAM_PARSE_X_EXPR_BASE HAM_EXPR_UTF(HAM_PARSE_X_UTF, base)
#define HAM_PARSE_X_EXPR_ERROR HAM_EXPR_UTF(HAM_PARSE_X_UTF, error)
#define HAM_PARSE_X_EXPR_BINDING HAM_EXPR_UTF(HAM_PARSE_X_UTF, binding)
#define HAM_PARSE_X_EXPR_REF HAM_EXPR_UTF(HAM_PARSE_X_UTF, ref)
#define HAM_PARSE_X_EXPR_UNRESOLVED HAM_EXPR_UTF(HAM_PARSE_X_UTF, unresolved)

#define HAM_PARSE_X_EXPR_LIT_INT HAM_EXPR_UTF(HAM_PARSE_X_UTF, lit_int)
#define HAM_PARSE_X_EXPR_LIT_REAL HAM_EXPR_UTF(HAM_PARSE_X_UTF, lit_real)
#define HAM_PARSE_X_EXPR_LIT_STR HAM_EXPR_UTF(HAM_PARSE_X_UTF, lit_str)

typedef struct HAM_PARSE_X_PARSE_CONTEXT HAM_PARSE_X_PARSE_CONTEXT;
typedef struct HAM_PARSE_X_PARSE_SCOPE HAM_PARSE_X_PARSE_SCOPE;

typedef struct HAM_PARSE_X_EXPR_BASE{
	ham_expr_kind kind;
	HAM_PARSE_X_TOKEN_RANGE tokens;
} HAM_PARSE_X_EXPR_BASE;

typedef struct HAM_PARSE_X_EXPR_ERROR{
	HAM_PARSE_X_EXPR_BASE super;
	const HAM_PARSE_X_EXPR_BASE *prev;
	ham_str8 message; // all parser error messages in utf-8
} HAM_PARSE_X_EXPR_ERROR;

typedef struct HAM_PARSE_X_EXPR_BINDING{
	HAM_PARSE_X_EXPR_BASE super;
	HAM_PARSE_X_STR name;
	const HAM_PARSE_X_EXPR_BASE *type, *value;
} HAM_PARSE_X_EXPR_BINDING;

typedef struct HAM_PARSE_X_EXPR_REF{
	HAM_PARSE_X_EXPR_BASE super;
	const HAM_PARSE_X_EXPR_BINDING *refed;
} HAM_PARSE_X_EXPR_REF;

typedef struct HAM_PARSE_X_EXPR_UNRESOLVED{
	HAM_PARSE_X_EXPR_BASE super;
	HAM_PARSE_X_STR id;
} HAM_PARSE_X_EXPR_UNRESOLVED;

typedef struct HAM_PARSE_X_EXPR_LIT_INT{
	HAM_PARSE_X_EXPR_BASE super;
	ham_aint value;
} HAM_PARSE_X_EXPR_LIT_INT;

typedef struct HAM_PARSE_X_EXPR_LIT_REAL{
	HAM_PARSE_X_EXPR_BASE super;
	ham_areal value;
} HAM_PARSE_X_EXPR_LIT_REAL;

typedef struct HAM_PARSE_X_EXPR_LIT_STR{
	HAM_PARSE_X_EXPR_BASE super;
	HAM_PARSE_X_STR value;
} HAM_PARSE_X_EXPR_LIT_STR;

typedef union HAM_PARSE_X_EXPR_U{
	ham_expr_kind kind;

	HAM_PARSE_X_EXPR_BASE base;
	HAM_PARSE_X_EXPR_ERROR error;
	HAM_PARSE_X_EXPR_BINDING binding;
	HAM_PARSE_X_EXPR_REF ref;
	HAM_PARSE_X_EXPR_UNRESOLVED unresolved;

	HAM_PARSE_X_EXPR_LIT_INT lit_int;
	HAM_PARSE_X_EXPR_LIT_REAL lit_real;
	HAM_PARSE_X_EXPR_LIT_STR lit_str;
} HAM_PARSE_X_EXPR_U;

#undef HAM_PARSE_X_STR
#undef HAM_PARSE_X_TOKEN
#undef HAM_PARSE_X_TOKEN_RANGE
#undef HAM_PARSE_X_PARSE_CONTEXT
#undef HAM_PARSE_X_PARSE_SCOPE
#undef HAM_PARSE_X_EXPR_U
#undef HAM_PARSE_X_EXPR_BASE
#undef HAM_PARSE_X_EXPR_BINDING
#undef HAM_PARSE_X_EXPR_REF
#undef HAM_PARSE_X_EXPR_UNRESOLVED
#undef HAM_PARSE_X_EXPR_LIT_INT
#undef HAM_PARSE_X_EXPR_LIT_REAL
#undef HAM_PARSE_X_EXPR_LIT_STR
#undef HAM_PARSE_X_UTF
#undef HAM_PARSE_X_H

#endif // !HAM_PARSE_X_H
