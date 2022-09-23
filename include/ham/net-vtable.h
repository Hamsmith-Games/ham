#ifndef HAM_NET_VTABLE_H
#define HAM_NET_VTABLE_H 1

/**
 * @defgroup HAM_NET_VTABLE Networking plugin vtables
 * @ingroup HAM_NET
 * @{
 */

#include "net.h"

#include "ham/memory.h"
#include "ham/plugin.h"

HAM_C_API_BEGIN

typedef struct ham_net_vtable ham_net_vtable;

struct ham_net_context{
	const ham_allocator *allocator;
	ham_dll_handle dll;
	const ham_net_vtable *vtable;
};

typedef ham_net_context*(*ham_net_context_alloc_fn)(const ham_allocator *allocator);
typedef void(*ham_net_context_free_fn)(ham_net_context *ctx);

typedef bool(*ham_net_context_init_fn)(ham_net_context *ctx);
typedef void(*ham_net_context_finish_fn)(ham_net_context *ctx);
typedef void(*ham_net_context_loop_fn)(ham_net_context *ctx, ham_f64 dt);

struct ham_net_vtable{
	ham_derive(ham_plugin_vtable)

	ham_net_context_alloc_fn context_alloc;
	ham_net_context_free_fn context_free;

	ham_net_context_init_fn context_init;
	ham_net_context_finish_fn context_finish;
	ham_net_context_loop_fn context_loop;
};

//! @cond ignore
#define HAM_IMPL_NET_VTABLE_NAME_PREFIX ham_impl_engine_vtable_

#define HAM_IMPL_NET_VTABLE_METHOD_NAME_(self_method_id) HAM_CONCAT(HAM_IMPL_NET_VTABLE_NAME_PREFIX, self_method_id)
#define HAM_IMPL_NET_VTABLE_METHOD_NAME(id, method_id) HAM_IMPL_NET_VTABLE_METHOD_NAME_(HAM_CONCAT(HAM_CONCAT(id, _), method_id))

#define HAM_IMPL_NET_VTABLE_NAME(id) HAM_CONCAT(HAM_IMPL_NET_VTABLE_NAME_PREFIX, id)

#define HAM_IMPL_NET_VTABLE(\
	derived_ctx, \
	uuid_str, \
	name_str, \
	version, \
	display_name_str, \
	author_str, \
	license_str, \
	desc_str, \
	on_load_fn, \
	on_unload_fn, \
	derived_alloc_name, \
	derived_free_name, \
	derived_init_fn_name, init_fn, \
	derived_finish_fn_name, finish_fn, \
	derived_loop_fn_name, loop_fn \
) \
static_assert(ham_is_same(ham_typeof(ham_super((derived_ctx*)ham_null)), ham_net_context*), #derived_ctx " is not derived from ham_net_context"); \
static inline ham_net_context *derived_alloc_name(const ham_allocator *allocator){ \
	void *const mem = ham_allocator_new(allocator, derived_ctx); \
	if(!mem) return ham_null; \
	derived_ctx *const ptr = (derived_ctx*)mem; \
	return ham_super(ptr); \
} \
static inline void derived_free_name(ham_net_context *ctx_super){ \
	derived_ctx *const ctx = (derived_ctx*)ctx_super; \
	ham_allocator_delete(ctx_super->allocator, ctx); \
} \
static inline bool derived_init_fn_name(ham_net_context *ctx_super){ return (init_fn)((derived_ctx*)ctx_super); } \
static inline void derived_finish_fn_name(ham_net_context *ctx_super){ (finish_fn)((derived_ctx*)ctx_super); } \
static inline void derived_loop_fn_name(ham_net_context *ctx_super, ham_f64 dt){ (loop_fn)((derived_ctx*)ctx_super, dt); } \
HAM_PLUGIN_VTABLE(\
	ham_net_vtable, \
	uuid_str, \
	name_str, \
	version, \
	display_name_str, \
	author_str, \
	license_str, \
	"net", \
	desc_str, \
	on_load_fn, \
	on_unload_fn, \
	( \
		.context_alloc = derived_alloc_name, \
		.context_free = derived_free_name, \
		.context_init = derived_init_fn_name, \
		.context_finish = derived_finish_fn_name, \
		.context_loop = derived_loop_fn_name, \
	) \
)
//! @endcond

#define HAM_NET_VTABLE( \
	derived_ctx, \
	uuid_str, \
	name_str, \
	version, \
	display_name_str, \
	author_str, \
	license_str, \
	desc_str, \
	 \
	on_load_fn, \
	on_unload_fn, \
	init_fn, \
	finish_fn, \
	loop_fn \
) \
	HAM_IMPL_NET_VTABLE(\
		derived_ctx, \
		uuid_str, \
		name_str, \
		version, \
		display_name_str, \
		author_str, \
		license_str, \
		desc_str, \
		on_load_fn, \
		on_unload_fn, \
		HAM_IMPL_NET_VTABLE_METHOD_NAME(derived_ctx, alloc), \
		HAM_IMPL_NET_VTABLE_METHOD_NAME(derived_ctx, free), \
		HAM_IMPL_NET_VTABLE_METHOD_NAME(derived_ctx, init), init_fn, \
		HAM_IMPL_NET_VTABLE_METHOD_NAME(derived_ctx, finish), finish_fn, \
		HAM_IMPL_NET_VTABLE_METHOD_NAME(derived_ctx, loop), loop_fn \
	)

HAM_C_API_END

/**
 * @}
 */

#endif // !HAM_NET_VTABLE_H
