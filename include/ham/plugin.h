#ifndef HAM_PLUGIN_H
#define HAM_PLUGIN_H 1

/**
 * @defgroup HAM_PLUGIN Plugins
 * @ingroup HAM
 * @{
 */

#include "typedefs.h"

HAM_C_API_BEGIN

typedef struct ham_plugin ham_plugin;
typedef struct ham_plugin_vtable ham_plugin_vtable;

ham_api ham_plugin *ham_plugin_load(ham_str8 path);

ham_api void ham_plugin_unload(ham_plugin *plugin);

ham_api void *ham_plugin_symbol(const ham_plugin *plugin, ham_str8 name);

typedef ham_str8(*ham_plugin_name_fn)();
typedef ham_str8(*ham_plugin_author_fn)();
typedef ham_str8(*ham_plugin_license_fn)();
typedef ham_str8(*ham_plugin_category_fn)();
typedef ham_str8(*ham_plugin_description_fn)();

typedef void(*ham_plugin_on_load_fn)();
typedef void(*ham_plugin_on_unload_fn)();

static inline void ham_plugin_on_load_pass(){}
static inline void ham_plugin_on_unload_pass(){}

typedef struct ham_plugin_vtable{
	ham_plugin_name_fn name;
	ham_plugin_author_fn author;
	ham_plugin_license_fn license;
	ham_plugin_category_fn category;
	ham_plugin_description_fn description;

	ham_plugin_on_load_fn on_load;
	ham_plugin_on_unload_fn on_unload;
} ham_plugin_vtable;

//! @cond ignore
#define HAM_IMPL_PLUGIN_VTABLE_NAME(derived) HAM_CONCAT(ham_impl_vtable_, derived)

#define HAM_IMPL_PLUGIN_VTABLE(derived_, name_, author_, license_, category_, desc_, on_load_fn_, on_unload_fn_, derived_body_) \
static inline ham_str8 HAM_CONCAT(ham_impl_vtable_name_, derived_)(){ return HAM_LIT_UTF8(name_); }\
static inline ham_str8 HAM_CONCAT(ham_impl_vtable_author_, derived_)(){ return HAM_LIT_UTF8(author_); }\
static inline ham_str8 HAM_CONCAT(ham_impl_vtable_license_, derived_)(){ return HAM_LIT_UTF8(license_); }\
static inline ham_str8 HAM_CONCAT(ham_impl_vtable_category_, derived_)(){ return HAM_LIT_UTF8(category_); }\
static inline ham_str8 HAM_CONCAT(ham_impl_vtable_description_, derived_)(){ return HAM_LIT_UTF8(desc_); }\
ham_public_export const derived_ *HAM_IMPL_PLUGIN_VTABLE_NAME(derived_)(){ \
	static_assert(ham_type_is_compatible(ham_typeof(((derived_*)ham_null)->HAM_SUPER_NAME), ham_plugin_vtable), "Plugin is not derived from ham_plugin_vtable"); \
	static const derived_ ret = { \
		.HAM_SUPER_NAME = { \
			.name        = HAM_CONCAT(ham_impl_vtable_name_, derived_), \
			.author      = HAM_CONCAT(ham_impl_vtable_author_, derived_), \
			.license     = HAM_CONCAT(ham_impl_vtable_license_, derived_), \
			.category    = HAM_CONCAT(ham_impl_vtable_category_, derived_), \
			.description = HAM_CONCAT(ham_impl_vtable_description_, derived_), \
			.on_load     = (on_load_fn_), \
			.on_unload   = (on_unload_fn_), \
		}, \
		HAM_EAT derived_body \
	}; \
	return &ret; \
}
//! @endcond

#define HAM_PLUGIN_VTABLE(\
	derived, \
	name, \
	author, \
	license, \
	category, \
	desc, \
	on_load_fn, \
	on_unload_fn, \
	derived_body \
) \
	HAM_IMPL_PLUGIN_VTABLE(derived, name, author, category, desc, on_load_fn, on_unload_fn, derived_body)

HAM_C_API_END

/**
 * @}
 */

#endif // !HAM_PLUGIN_H
