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

#ifndef HAM_ENGINE_H
#define HAM_ENGINE_H 1

/**
 * @defgroup HAM_ENGINE Ham World Engine
 * @{
 */

#include "engine/world.h"

#define HAM_ENGINE_VERSION ((ham_version){HAM_ENGINE_VERSION_MAJOR,HAM_ENGINE_VERSION_MINOR,HAM_ENGINE_VERSION_PATCH})

HAM_C_API_BEGIN

ham_engine_api ham_nothrow ham_version ham_engine_version();

ham_engine_api ham_nothrow const char *ham_engine_version_line();

typedef struct ham_screen{
	ham_u32 w, h, refresh;
} ham_screen;

/**
 * @defgroup HAM_ENGINE_CTX Engine management
 * @{
 */

/**
 * Base engine object.
 */
typedef struct ham_engine ham_engine;

/**
 * Create a new engine.
 * @warning It is not advised to create multiple engines in the same process.
 * @param plugin_id id of the engine plugin
 * @param obj_id id of the engine object within \p plugin_id
 * @param argc ``argc`` passed from ``main``
 * @param argv ``argv`` passed from ``main``
 * @returns newly created engine or ``NULL`` on error
 * @see ham_engine_subsystem_create
 * @see ham_engine_exec
 * @see ham_engine_destroy
 */
ham_engine_api ham_engine *ham_engine_create(
	const char *plugin_id,
	const char *obj_id,
	int argc, char **argv
);

/**
 * Destroy an unexecuted engine.
 * @note Do not use this function after \ref ham_engine_exec on the same \p engine .
 * @param engine engine to destroy
 * @see ham_engine_create
 */
ham_engine_api ham_nothrow void ham_engine_destroy(ham_engine *engine);

/**
 * Request an engine to finish execution.
 * @param engine engine to make the request on
 * @returns whether the request was successful
 */
ham_engine_api ham_nothrow bool ham_engine_request_exit(ham_engine *engine);

ham_engine_api ham_nothrow ham_world *ham_engine_world(ham_engine *engine);

/**
 * Execute an engine, destroy it and return an exit code.
 * @note This function calls \ref ham_engine_destroy on the \p engine .
 * @param engine engine to execute
 * @return exit status as if returned from ``main``
 */
ham_engine_api int ham_engine_exec(ham_engine *engine);

/**
 * @}
 */

/**
 * @defgroup HAM_ENGINE_SYS Sub-systems
 * @{
 */

typedef struct ham_engine_subsys ham_engine_subsys;

typedef bool(*ham_engine_subsys_init_fn)(ham_engine *engine, void *user);
typedef void(*ham_engine_subsys_fini_fn)(ham_engine *engine, void *user);
typedef void(*ham_engine_subsys_loop_fn)(ham_engine *engine, ham_f64 dt, void *user);

//! @cond ignore
ham_engine_api ham_nothrow void ham_impl_engine_subsys_destroy(ham_engine_subsys *subsys);
ham_engine_api ham_nothrow bool ham_impl_engine_subsys_request_exit(ham_engine_subsys *subsys);
//! @endcond

/**
 * Create a new engine subsystem.
 * @param engine engine to create the subsystem in
 * @param name name of the new subsystem; a valid string is required
 * @param init_fn initializer function
 * @param fini_fn finalizer function
 * @param loop_fn loop/tick function
 * @param user passed in ever call to \p init_fn , \p fini_fn and \p loop_fn
 * @returns newly created subsystem or ``NULL`` on error
 * @see ham_engine_subsys_launch
 */
ham_engine_api ham_engine_subsys *ham_engine_subsys_create(
	ham_engine *engine,
	ham_str8 name,
	ham_engine_subsys_init_fn init_fn,
	ham_engine_subsys_fini_fn fini_fn,
	ham_engine_subsys_loop_fn loop_fn,
	void *user
);

ham_engine_api ham_nothrow ham_engine *ham_engine_subsys_owner(ham_engine_subsys *subsys);

ham_engine_api ham_nothrow bool ham_engine_subsys_running(const ham_engine_subsys *subsys);

/**
 * Get a subsystems minimum time between loops.
 * @param subsys subsystem to query
 * @returns minimum dt of \p subsys , if \p subsys is ``NULL`` 0.0 is returned
 */
ham_engine_api ham_nothrow ham_f64 ham_engine_subsys_min_dt(ham_engine_subsys *subsys);

/**
 * Set a subsystems minimum time between loops.
 * @param subsys subsystem to query
 * @param min_dt new minimum dt
 * @returns whether the minimum dt was successfully set
 */
ham_engine_api ham_nothrow bool ham_engine_subsys_set_min_dt(ham_engine_subsys *subsys, ham_f64 min_dt);

/**
 * Launch a subsystem before its owning engine is executed.
 * @param subsys
 * @returns whether the subsystem was successfully launched
 */
ham_engine_api ham_nothrow bool ham_engine_subsys_launch(ham_engine_subsys *subsys);

/**
 * @}
 */

HAM_C_API_END

#ifdef __cplusplus

#include "ham/str_buffer.h"

namespace ham::engine{
	class subsystem_error: public exception{
		public:
			subsystem_error(str_buffer_utf8 msg): m_msg(std::move(msg)){}

			const char *api() const noexcept override{ return "ham::engine::subsys"; }
			const char *what() const noexcept override{ return m_msg.c_str(); }

		private:
			str_buffer_utf8 m_msg;
			const char *m_what;
	};

	template<typename Derived>
	class subsys_base{
		public:
			ham_engine *owner() const noexcept{ return ham_engine_subsys_owner(m_subsys); }

			bool running() const noexcept{ return ham_engine_subsys_running(m_subsys); }

			bool launch() noexcept{ return ham_engine_subsys_launch(m_subsys); }

			bool set_min_dt(f64 new_min_dt){ return ham_engine_subsys_set_min_dt(m_subsys, new_min_dt); }

		protected:
			explicit subsys_base(ham_engine *engine, const ham::str8 &name)
				: m_subsys{ham_engine_subsys_create(engine, name, init_impl, fini_impl, loop_impl, this)}
			{
				if(!m_subsys){
					throw subsystem_error(format("error creating subsystem \"{}\"", name));
				}
			}

		private:
			static bool init_impl(ham_engine *engine, void *user){
				const auto self = reinterpret_cast<Derived*>(user);
				return self->init(engine);
			}

			static void fini_impl(ham_engine *engine, void *user) noexcept{
				const auto self = reinterpret_cast<Derived*>(user);
				self->fini(engine);
			}

			static void loop_impl(ham_engine *engine, ham_f64 dt, void *user){
				const auto self = reinterpret_cast<Derived*>(user);
				self->loop(engine, dt);
			}

			ham_engine_subsys *m_subsys;
	};
}

#endif // __cplusplus

/**
 * @}
 */

#endif // !HAM_ENGINE_H
