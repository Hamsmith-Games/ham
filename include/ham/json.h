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

#ifndef HAM_JSON_H
#define HAM_JSON_H 1

/**
 * @defgroup HAM_JSON JSON Parsing
 * @ingroup HAM
 * @{
 */

#include "ham/typedefs.h"

HAM_C_API_BEGIN

typedef struct ham_json_document ham_json_document;

typedef struct ham_json_value ham_json_value;

ham_api ham_json_document *ham_json_document_create(ham_str8 json);

ham_api void ham_json_document_destroy(ham_json_document *doc);

ham_api const ham_json_value *ham_json_document_root(const ham_json_document *doc);

typedef enum ham_json_type{
	HAM_JSON_NULL,
	HAM_JSON_OBJECT,
	HAM_JSON_ARRAY,
	HAM_JSON_BOOL,
	HAM_JSON_NAT,
	HAM_JSON_INT,
	HAM_JSON_REAL,
	HAM_JSON_STRING,
	HAM_JSON_RAW,

	HAM_JSON_TYPE_COUNT
} ham_json_type;

ham_api ham_json_type ham_json_get_type(const ham_json_value *json);

// Objects

ham_api const ham_json_value *ham_json_object_get(const ham_json_value *json, const char *key);

typedef bool(*ham_json_object_iterate_fn)(ham_str8 key, const ham_json_value *value, void *user);

ham_api ham_usize ham_json_object_iterate(const ham_json_value *json, ham_json_object_iterate_fn fn, void *user);

// Arrays

ham_api const ham_json_value *ham_json_array_get(const ham_json_value *json, ham_usize idx);

typedef bool(*ham_json_array_iterate_fn)(ham_usize idx, const ham_json_value *value, void *user);

ham_api ham_usize ham_json_array_iterate(const ham_json_value *json, ham_json_array_iterate_fn fn, void *user);

// Numeric

ham_api ham_uptr ham_json_get_nat(const ham_json_value *json);

ham_api ham_iptr ham_json_get_int(const ham_json_value *json);

ham_api ham_f64 ham_json_get_real(const ham_json_value *json);

// Strings

ham_api ham_str8 ham_json_get_str(const ham_json_value *json);

HAM_C_API_END

/**
 * @}
 */

#endif // !HAM_JSON_H
