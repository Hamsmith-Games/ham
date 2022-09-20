/**
 * The Ham World Engine
 * Copyright (C) 2022  Hamsmith Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef HAM_ENGINE_VTABLE_H
#define HAM_ENGINE_VTABLE_H 1

#include "engine.h"

#include "ham/memory.h"
#include "ham/plugin.h"
#include "ham/async.h"
#include "ham/thread.h"

#include <atomic>

HAM_C_API_BEGIN

typedef struct ham_engine_vtable ham_engine_vtable;

struct ham_engine_context{
	const ham_allocator *allocator = nullptr;
	ham_dll_handle dll_handle = nullptr;
	const ham_engine_vtable *vtable = nullptr;
	int status = 0;

	ham_mutex *mut = nullptr;
	ham_sem *sem = nullptr;

	std::atomic_bool running = false;
	std::atomic<ham_f64> min_dt = 0.0;

	ham_thread *thd = nullptr;

	ham_path_buffer_utf8 game_dir = { 0 };
};

typedef ham_engine_context*(*ham_engine_context_alloc_fn)(const ham_allocator *allocator);
typedef void(*ham_engine_context_free_fn)(ham_engine_context *ctx);

typedef bool(*ham_engine_context_init_fn)(ham_engine_context *ctx);
typedef void(*ham_engine_context_finish_fn)(ham_engine_context *ctx);
typedef void(*ham_engine_context_loop_fn)(ham_engine_context *ctx, ham_f64 dt);

typedef struct ham_engine_vtable{
	ham_derive(ham_plugin_vtable)

	ham_engine_context_alloc_fn context_alloc;
	ham_engine_context_free_fn context_free;

	ham_engine_context_init_fn context_init;
	ham_engine_context_finish_fn context_finish;
	ham_engine_context_loop_fn context_loop;
} ham_engine_vtable;

//! @cond ignore
#define HAM_IMPL_ENGINE_VTABLE_NAME_PREFIX ham_impl_engine_vtable_

#define HAM_IMPL_ENGINE_VTABLE_METHOD_NAME_(self_method_id) HAM_CONCAT(HAM_IMPL_ENGINE_VTABLE_NAME_PREFIX, self_method_id)
#define HAM_IMPL_ENGINE_VTABLE_METHOD_NAME(id, method_id) HAM_IMPL_ENGINE_VTABLE_METHOD_NAME_(HAM_CONCAT(HAM_CONCAT(id, _), method_id))

#define HAM_IMPL_ENGINE_VTABLE_NAME(id) HAM_CONCAT(HAM_IMPL_ENGINE_VTABLE_NAME_PREFIX, id)

#define HAM_IMPL_ENGINE_VTABLE( \
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
static_assert(ham_is_same(ham_typeof(ham_super((derived_ctx*)ham_null)), ham_engine_context*), #derived_ctx " is not derived from ham_engine_context"); \
static inline ham_engine_context *derived_alloc_name(const ham_allocator *allocator){ \
	void *const mem = ham_allocator_new(allocator, derived_ctx); \
	if(!mem) return ham_null; \
	derived_ctx *const ptr = (derived_ctx*)mem; \
	return ham_super(ptr); \
} \
static inline void derived_free_name(ham_engine_context *ctx_super){ \
	derived_ctx *const ctx = (derived_ctx*)ctx_super; \
	ham_allocator_delete(ctx_super->allocator, ctx); \
} \
static inline bool derived_init_fn_name(ham_engine_context *ctx_super){ return (init_fn)((derived_ctx*)ctx_super); } \
static inline void derived_finish_fn_name(ham_engine_context *ctx_super){ (finish_fn)((derived_ctx*)ctx_super); } \
static inline void derived_loop_fn_name(ham_engine_context *ctx_super, ham_f64 dt){ (loop_fn)((derived_ctx*)ctx_super, dt); } \
HAM_PLUGIN_VTABLE(\
	ham_engine_vtable, \
	uuid_str, \
	name_str, \
	version, \
	display_name_str, \
	author_str, \
	license_str, \
	HAM_ENGINE_PLUGIN_CATEGORY, \
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

#define HAM_ENGINE_VTABLE( \
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
	HAM_IMPL_ENGINE_VTABLE(\
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
		HAM_IMPL_ENGINE_VTABLE_METHOD_NAME(derived_ctx, alloc), \
		HAM_IMPL_ENGINE_VTABLE_METHOD_NAME(derived_ctx, free), \
		HAM_IMPL_ENGINE_VTABLE_METHOD_NAME(derived_ctx, init), init_fn, \
		HAM_IMPL_ENGINE_VTABLE_METHOD_NAME(derived_ctx, finish), finish_fn, \
		HAM_IMPL_ENGINE_VTABLE_METHOD_NAME(derived_ctx, loop), loop_fn \
	)

HAM_C_API_END

#endif // !HAM_ENGINE_VTABLE_H
