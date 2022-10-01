/*
 * Ham World Engine Runtime
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

#include "ham/engine-object.h"

#include "ham/async.h"
#include "ham/check.h"

#include <atomic>

HAM_C_API_BEGIN

struct ham_engine_subsys{
	ham_engine *engine;

	ham_engine_subsys_init_fn init_fn;
	ham_engine_subsys_fini_fn fini_fn;
	ham_engine_subsys_loop_fn loop_fn;
	void *user;

	volatile bool running = false;
	volatile ham_f64 min_dt = 0.0;

	ham::thread thread;
	ham::mutex mut;
	ham::cond cond;
};

static inline ham_uptr ham_engine_subsys_thread_routine(void *data){
	const auto subsys = (ham_engine_subsys*)data;

	{
		ham::unique_lock lock(subsys->mut);
		if(!subsys->cond.wait(lock, [subsys]{ return subsys->running; })){
			ham_logapierrorf("Error waiting on subsystem mutex");
			return 1;
		}
	}

	if(!subsys->init_fn(subsys->engine, subsys->user)){
		ham_logapierrorf("Error initializing subsystem");
		return 1;
	}

	ham_ticker ticker;
	ham_ticker_reset(&ticker);

	//ham_f64 excess_dt = 0.0;

	while(subsys->running){
		//const auto min_dt = subsys->min_dt - excess_dt;
		const auto dt = ham_ticker_tick(&ticker, subsys->min_dt);
		//excess_dt = ham_max(dt - min_dt, 0.0);
		subsys->loop_fn(subsys->engine, dt, subsys->user);
	}

	subsys->fini_fn(subsys->engine, subsys->user);

	return 0;
}

ham_nothrow void ham_impl_engine_subsys_destroy(ham_engine_subsys *subsys){
	if(ham_unlikely(subsys == NULL)) return;

	const auto allocator = subsys->engine->allocator;

	ham_allocator_delete(allocator, subsys);
}

ham_nothrow bool ham_impl_engine_subsys_request_exit(ham_engine_subsys *subsys){
	if(!ham_check(subsys != NULL)) return false;
	subsys->running = false;
	return true;
}

ham_engine_subsys *ham_engine_subsys_create(
	ham_engine *engine,
	ham_str8 name,
	ham_engine_subsys_init_fn init_fn,
	ham_engine_subsys_fini_fn fini_fn,
	ham_engine_subsys_loop_fn loop_fn,
	void *user
){
	if(
	   !ham_check(engine != NULL) ||
	   !ham_check(init_fn != NULL) ||
	   !ham_check(fini_fn != NULL) ||
	   !ham_check(loop_fn != NULL) ||
	   !ham_check(name.len && name.ptr)
	){
		return nullptr;
	}

	ham::scoped_lock lock(engine->subsys_mut);

	const auto subsys_idx = engine->num_subsystems;
	if(subsys_idx >= HAM_ENGINE_MAX_SUBSYSTEMS){
		ham_logapierrorf("Max subsystems already created (%d)", HAM_ENGINE_MAX_SUBSYSTEMS);
		return nullptr;
	}

	const auto allocator = engine->allocator;

	const auto subsys = ham_allocator_new(allocator, ham_engine_subsys);
	if(!subsys){
		ham_logapierrorf("Error allocating memory for ham_engine_subsys");
		return nullptr;
	}

	subsys->engine = engine;
	subsys->init_fn = init_fn;
	subsys->fini_fn = fini_fn;
	subsys->loop_fn = loop_fn;
	subsys->user = user;
	subsys->min_dt = engine->min_dt;

	subsys->thread = ham::thread((ham_thread_fn)ham_engine_subsys_thread_routine, (void*)subsys);
	subsys->thread.set_name(name);

	if(engine->running){
		{
			ham::scoped_lock lock(subsys->mut);
			subsys->running = true;
		}

		if(!subsys->cond.signal()){
			ham_logapierrorf("Error signaling subsystem to start");
			ham_allocator_delete(allocator, subsys);
			return nullptr;
		}
	}

	engine->subsystems[subsys_idx] = subsys;
	++engine->num_subsystems;

	return subsys;
}

ham_nothrow ham_engine *ham_engine_subsys_owner(ham_engine_subsys *subsys){
	if(!ham_check(subsys != NULL)) return nullptr;
	return subsys->engine;
}

ham_nothrow bool ham_engine_subsys_running(const ham_engine_subsys *subsys){
	if(!ham_check(subsys != NULL)) return false;
	return subsys->running;
}

ham_nothrow bool ham_engine_subsys_set_min_dt(ham_engine_subsys *subsys, ham_f64 min_dt){
	if(!ham_check(subsys != NULL)) return false;
	subsys->min_dt = ham_max(0.0, min_dt);
	return true;
}

ham_nothrow bool ham_engine_subsys_launch(ham_engine_subsys *subsys){
	if(!ham_check(subsys != NULL)) return false;

	{
		ham::scoped_lock lock(subsys->mut);
		if(std::exchange(subsys->running, true)) return true;
	}

	return subsys->cond.signal();
}

HAM_C_API_END
