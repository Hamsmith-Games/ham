/*
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

#include "ham/object.h"
#include "ham/memory.h"
#include "ham/async.h"
#include "ham/thread.h"
#include "ham/plugin.h"

#include <atomic>

HAM_C_API_BEGIN

struct ham_engine{
	ham_derive(ham_object)

	const ham_allocator *allocator = nullptr;
	ham_plugin *plugin = nullptr;
	ham_dso_handle dso_handle = nullptr;
	int status = 0;

	ham_mutex *mut = nullptr;
	ham_sem *sem = nullptr;

	std::atomic_bool running = false;
	std::atomic<ham_f64> min_dt = 0.0;

	ham_thread *thd = nullptr;

	ham_path_buffer_utf8 game_dir = { 0 };
};

struct ham_engine_vtable{
	ham_derive(ham_object_vtable)

	bool(*init)(ham_engine *engine);
	void(*fini)(ham_engine *engine);
	void(*loop)(ham_engine *engine, ham_f64 dt);
};

HAM_C_API_END

#endif // !HAM_ENGINE_VTABLE_H
