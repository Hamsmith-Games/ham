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

#ifndef HAM_PLUGIN_H
#define HAM_PLUGIN_H 1

/**
 * @defgroup HAM_PLUGIN Plugins
 * @ingroup HAM
 * @{
 */

#include "object.h"
#include "dso.h"

HAM_C_API_BEGIN

typedef struct ham_plugin ham_plugin;
typedef struct ham_plugin_vtable ham_plugin_vtable;

/**
 * Get the default plugin search path.
 * This function will return the environment variable HAM_PLUGIN_PATH if set
 * or the current working directory with "/plugins" appended otherwise.
 * @returns null-terminated path string
 */
ham_api ham_str8 ham_plugin_default_path();

/**
 * Search for a plugin within a directory based on id.
 * @param id an identifier to match with the plugin; this can be a uuid, name or display name
 * @param path path to the directory to search
 * @param[out] plugin_ret where to store the loaded plugin
 * @param[out] dso_ret where to store the opened dso
 * @returns whether the plugin was succefully found
 */
ham_api bool ham_plugin_find(const char *id, ham_str8 path, ham_plugin **plugin_ret, ham_dso_handle *dso_ret);

ham_api ham_plugin *ham_plugin_load(ham_dso_handle dso, const char *plugin_id);

ham_api void ham_plugin_unload(ham_plugin *plugin);

ham_api ham_uuid    ham_plugin_uuid(const ham_plugin *plugin);
ham_api ham_str8    ham_plugin_name(const ham_plugin *plugin);
ham_api ham_version ham_plugin_version(const ham_plugin *plugin);
ham_api ham_str8    ham_plugin_display_name(const ham_plugin *plugin);
ham_api ham_str8    ham_plugin_author(const ham_plugin *plugin);
ham_api ham_str8    ham_plugin_license(const ham_plugin *plugin);
ham_api ham_str8    ham_plugin_category(const ham_plugin *plugin);
ham_api ham_str8    ham_plugin_description(const ham_plugin *plugin);

ham_api bool ham_plugin_init(ham_plugin *plugin);
ham_api bool ham_plugin_fini(ham_plugin *plugin);

ham_api const ham_object_vtable *ham_plugin_object(const ham_plugin *plugin, ham_str8 name);

typedef bool(*ham_plugin_iterate_objects_fn)(const ham_object_vtable *vtable, void *user);

ham_api ham_usize ham_plugin_iterate_objects(const ham_plugin *plugin, ham_plugin_iterate_objects_fn fn, void *user);

typedef ham_uuid(*ham_plugin_uuid_fn)();
typedef ham_str8(*ham_plugin_name_fn)();
typedef ham_version(*ham_plugin_version_fn)();
typedef ham_str8(*ham_plugin_display_name_fn)();
typedef ham_str8(*ham_plugin_author_fn)();
typedef ham_str8(*ham_plugin_license_fn)();
typedef ham_str8(*ham_plugin_category_fn)();
typedef ham_str8(*ham_plugin_description_fn)();

typedef bool(*ham_plugin_init_fn)();
typedef void(*ham_plugin_fini_fn)();

static inline bool ham_plugin_init_pass(){ return true; }
static inline void ham_plugin_fini_pass(){}

typedef struct ham_plugin_vtable{
	ham_plugin_uuid_fn uuid;
	ham_plugin_name_fn name;
	ham_plugin_version_fn version;
	ham_plugin_display_name_fn display_name;
	ham_plugin_author_fn author;
	ham_plugin_license_fn license;
	ham_plugin_category_fn category;
	ham_plugin_description_fn description;

	ham_plugin_init_fn init;
	ham_plugin_fini_fn fini;
} ham_plugin_vtable;

//! @cond ignore
#define HAM_IMPL_PLUGIN_VTABLE_NAME_PREFIX ham_impl_vtable_
#define HAM_IMPL_PLUGIN_VTABLE_NAME(derived) HAM_CONCAT(HAM_IMPL_PLUGIN_VTABLE_NAME_PREFIX, derived)

#define HAM_IMPL_PLUGIN(id_, uuid_str_, name_, version_, display_name_, author_, license_, category_, desc_, init_fn_, fini_fn_) \
	static inline ham_uuid HAM_CONCAT(ham_impl_vtable_uuid_, id_)(){ return ham_str_to_uuid_utf8(HAM_LIT_UTF8(uuid_str_)); } \
	static inline ham_str8 HAM_CONCAT(ham_impl_vtable_name_, id_)(){ return HAM_LIT_UTF8(name_); } \
	static inline ham_version HAM_CONCAT(ham_impl_vtable_version_, id_)(){ return version_; } \
	static inline ham_str8 HAM_CONCAT(ham_impl_vtable_display_name_, id_)(){ return HAM_LIT_UTF8(display_name_); } \
	static inline ham_str8 HAM_CONCAT(ham_impl_vtable_author_, id_)(){ return HAM_LIT_UTF8(author_); } \
	static inline ham_str8 HAM_CONCAT(ham_impl_vtable_license_, id_)(){ return HAM_LIT_UTF8(license_); } \
	static inline ham_str8 HAM_CONCAT(ham_impl_vtable_category_, id_)(){ return HAM_LIT_UTF8(category_); } \
	static inline ham_str8 HAM_CONCAT(ham_impl_vtable_description_, id_)(){ return HAM_LIT_UTF8(desc_); } \
	ham_extern_c ham_public ham_export const ham_plugin_vtable *HAM_IMPL_PLUGIN_VTABLE_NAME(id_)(){ \
		static const ham_plugin_vtable ret = (ham_plugin_vtable){ \
			.uuid         = HAM_CONCAT(ham_impl_vtable_uuid_, id_), \
			.name         = HAM_CONCAT(ham_impl_vtable_name_, id_), \
			.version      = HAM_CONCAT(ham_impl_vtable_version_, id_), \
			.display_name = HAM_CONCAT(ham_impl_vtable_display_name_, id_), \
			.author       = HAM_CONCAT(ham_impl_vtable_author_, id_), \
			.license      = HAM_CONCAT(ham_impl_vtable_license_, id_), \
			.category     = HAM_CONCAT(ham_impl_vtable_category_, id_), \
			.description  = HAM_CONCAT(ham_impl_vtable_description_, id_), \
			.init         = (init_fn_), \
			.fini         = (fini_fn_), \
		}; \
		return &ret; \
	}
//! @endcond

#define HAM_PLUGIN(\
	id, \
	uuid_str, \
	name, \
	version, \
	display_name, \
	author, \
	license, \
	category, \
	desc, \
	init_fn, \
	fini_fn \
) \
	HAM_IMPL_PLUGIN(id, uuid_str, name, version, display_name, author, license, category, desc, init_fn, fini_fn)

HAM_C_API_END

/**
 * @}
 */

#endif // !HAM_PLUGIN_H
