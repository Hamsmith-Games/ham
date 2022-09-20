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

#ifndef HAM_PLUGIN_H
#define HAM_PLUGIN_H 1

/**
 * @defgroup HAM_PLUGIN Plugins
 * @ingroup HAM
 * @{
 */

#include "dll.h"

HAM_C_API_BEGIN

typedef struct ham_plugin ham_plugin;
typedef struct ham_plugin_vtable ham_plugin_vtable;

ham_api ham_plugin *ham_plugin_load(ham_dll_handle dll);

ham_api void ham_plugin_unload(ham_plugin *plugin);

typedef bool(*ham_plugin_iterate_vtables_fn)(const ham_plugin_vtable *vtable, void *user);

ham_api ham_usize ham_plugin_iterate_vtables(const ham_plugin *plugin, ham_plugin_iterate_vtables_fn fn, void *user);

typedef ham_uuid(*ham_plugin_uuid_fn)();
typedef ham_str8(*ham_plugin_name_fn)();
typedef ham_version(*ham_plugin_version_fn)();
typedef ham_str8(*ham_plugin_display_name_fn)();
typedef ham_str8(*ham_plugin_author_fn)();
typedef ham_str8(*ham_plugin_license_fn)();
typedef ham_str8(*ham_plugin_category_fn)();
typedef ham_str8(*ham_plugin_description_fn)();

typedef bool(*ham_plugin_on_load_fn)();
typedef void(*ham_plugin_on_unload_fn)();

static inline bool ham_plugin_on_load_pass(){ return true; }
static inline void ham_plugin_on_unload_pass(){}

typedef struct ham_plugin_vtable{
	ham_plugin_uuid_fn uuid;
	ham_plugin_name_fn name;
	ham_plugin_version_fn version;
	ham_plugin_display_name_fn display_name;
	ham_plugin_author_fn author;
	ham_plugin_license_fn license;
	ham_plugin_category_fn category;
	ham_plugin_description_fn description;

	ham_plugin_on_load_fn on_load;
	ham_plugin_on_unload_fn on_unload;
} ham_plugin_vtable;

//! @cond ignore
#define HAM_IMPL_PLUGIN_VTABLE_NAME_PREFIX ham_impl_vtable_
#define HAM_IMPL_PLUGIN_VTABLE_NAME(derived) HAM_CONCAT(HAM_IMPL_PLUGIN_VTABLE_NAME_PREFIX, derived)

#define HAM_IMPL_PLUGIN_VTABLE(derived_, uuid_str_, name_, version_, display_name_, author_, license_, category_, desc_, on_load_fn_, on_unload_fn_, derived_body_) \
static inline ham_uuid HAM_CONCAT(ham_impl_vtable_uuid_, derived_)(){ return ham_str_to_uuid_utf8(HAM_LIT_UTF8(uuid_str_)); } \
static inline ham_str8 HAM_CONCAT(ham_impl_vtable_name_, derived_)(){ return HAM_LIT_UTF8(name_); } \
static inline ham_version HAM_CONCAT(ham_impl_vtable_version_, derived_)(){ return version_; } \
static inline ham_str8 HAM_CONCAT(ham_impl_vtable_display_name_, derived_)(){ return HAM_LIT_UTF8(display_name_); } \
static inline ham_str8 HAM_CONCAT(ham_impl_vtable_author_, derived_)(){ return HAM_LIT_UTF8(author_); } \
static inline ham_str8 HAM_CONCAT(ham_impl_vtable_license_, derived_)(){ return HAM_LIT_UTF8(license_); } \
static inline ham_str8 HAM_CONCAT(ham_impl_vtable_category_, derived_)(){ return HAM_LIT_UTF8(category_); } \
static inline ham_str8 HAM_CONCAT(ham_impl_vtable_description_, derived_)(){ return HAM_LIT_UTF8(desc_); } \
ham_extern_c ham_public ham_export const derived_ *HAM_IMPL_PLUGIN_VTABLE_NAME(derived_)(){ \
	static_assert((ham_is_same(ham_typeof(ham_super((derived_*)ham_null)), ham_plugin_vtable*)), "Plugin is not derived from ham_plugin_vtable"); \
	static const derived_ ret = (derived_){ \
		.HAM_SUPER_NAME = { \
			.uuid         = HAM_CONCAT(ham_impl_vtable_uuid_, derived_), \
			.name         = HAM_CONCAT(ham_impl_vtable_name_, derived_), \
			.version      = HAM_CONCAT(ham_impl_vtable_version_, derived_), \
			.display_name = HAM_CONCAT(ham_impl_vtable_display_name_, derived_), \
			.author       = HAM_CONCAT(ham_impl_vtable_author_, derived_), \
			.license      = HAM_CONCAT(ham_impl_vtable_license_, derived_), \
			.category     = HAM_CONCAT(ham_impl_vtable_category_, derived_), \
			.description  = HAM_CONCAT(ham_impl_vtable_description_, derived_), \
			.on_load      = (on_load_fn_), \
			.on_unload    = (on_unload_fn_), \
		}, \
		HAM_EAT derived_body_ \
	}; \
	return &ret; \
}
//! @endcond

#define HAM_PLUGIN_VTABLE(\
	derived, \
	uuid_str, \
	name, \
	version, \
	display_name, \
	author, \
	license, \
	category, \
	desc, \
	on_load_fn, \
	on_unload_fn, \
	derived_body \
) \
	HAM_IMPL_PLUGIN_VTABLE(derived, uuid_str, name, version, display_name, author, license, category, desc, on_load_fn, on_unload_fn, derived_body)

HAM_C_API_END

/**
 * @}
 */

#endif // !HAM_PLUGIN_H
