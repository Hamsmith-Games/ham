/*
 * Ham World Engine Runtime
 * Copyright (C) 2022 Keith Hammond
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

#ifndef HAM_ENGINE_OBJECT_H
#define HAM_ENGINE_OBJECT_H 1

#include "engine.h"

#include "ham/object.h"
#include "ham/memory.h"
#include "ham/async.h"
#include "ham/plugin.h"

#include <atomic>

HAM_C_API_BEGIN

struct ham_app_info{
	ham_u32 appid;
	ham_version version;
	ham_name_buffer_utf8 name, display_name, author, license;
	ham_message_buffer_utf8 description;
};

struct ham_engine{
	ham_derive(ham_object)

	const ham_allocator *allocator;
	ham_plugin *plugin;
	ham_dso_handle dso_handle;
	int status;

	ham_app_info app_info;

	ham_mutex *mut;
	ham_sem *sem;
	ham_cond *cond;

	volatile bool running;
	volatile ham_f64 min_dt;

	ham_thread *thd;

	ham_path_buffer_utf8 game_dir;

	ham_mutex *subsys_mut;
	ham_u8 num_subsystems;
	ham_engine_subsys *subsystems[HAM_ENGINE_MAX_SUBSYSTEMS];
};

struct ham_engine_vtable{
	ham_derive(ham_object_vtable)

	bool(*init)(ham_engine *engine);
	void(*fini)(ham_engine *engine);
	void(*loop)(ham_engine *engine, ham_f64 dt);
};

HAM_C_API_END

#endif // !HAM_ENGINE_OBJECT_H
