/*
 * Ham World Engine Runtime
 * Copyright (C) 2022 Hamsmith Ltd.
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

#include "engine/world.h" // IWYU pragma: keep

#include "ham/json.h"

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

typedef struct ham_engine_subsys ham_engine_subsys;

//! @cond ignore
ham_engine_api extern ham_engine *ham_impl_gengine;
//! @endcond

ham_nothrow static inline ham_engine *ham_gengine(){
	return ham_impl_gengine;
}

/**
 * @brief Create a new engine.
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

typedef bool(*ham_engine_app_init_fn)(ham_engine *engine, void *user);
typedef void(*ham_engine_app_fini_fn)(ham_engine *engine, void *user);
typedef void(*ham_engine_app_loop_fn)(ham_engine *engine, ham_f64 dt, void *user);

typedef struct ham_engine_app{
	ham_u32 id;
	ham_str8 dir, name, display_name, author, license, description;
	ham_version version;

	ham_engine_app_init_fn init;
	ham_engine_app_fini_fn fini;
	ham_engine_app_loop_fn loop;
	void *user;
} ham_engine_app;

ham_engine_api ham_nothrow bool ham_engine_app_load_json(ham_engine_app *ret, const ham_json_value *json);

/**
 * @brief Create a new engine.
 * @warning You can not create multiple engine instances in the same _process_.
 * @param[in] app engine application information
 * @returns newly created engine or ``NULL`` on error
 */
ham_engine_api ham_engine *ham_engine_create2(const ham_engine_app *app);

/**
 * @brief Destroy an engine.
 * @param engine engine to destroy
 * @see ham_engine_create
 */
ham_engine_api ham_nothrow void ham_engine_destroy(ham_engine *engine);

ham_engine_api ham_nothrow const ham_engine_app *ham_engine_get_app(const ham_engine *engine);

ham_engine_api ham_nothrow ham_usize ham_engine_num_subsystems(const ham_engine *engine);

ham_engine_api ham_nothrow ham_engine_subsys *ham_engine_get_subsystem(ham_engine *engine, ham_usize idx);

/**
 * @brief Request an engine to finish execution.
 * @param engine engine to make the request on
 * @returns whether the request was successful
 */
ham_engine_api ham_nothrow bool ham_engine_request_exit(ham_engine *engine);

/**
 * @brief Execute an engine, destroy it and return an exit code.
 * @param engine engine to execute
 * @return exit status as if returned from ``main``
 * @see ham_engine_main
 */
ham_engine_api int ham_engine_exec(ham_engine *engine);

/**
 * @brief Execute an engine, destroy it and return an exit code.
 * @note This function calls \ref ham_engine_destroy on the \p engine .
 * @param engine engine to execute
 * @return exit status as if returned from ``main``
 */
static inline int ham_engine_main(ham_engine *engine){
	int result = -1;
	if(engine){
		result = ham_engine_exec(engine);
		ham_engine_destroy(engine);
	}
	return result;
}

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
 * @brief Create a new engine subsystem.
 * @param engine engine to create the subsystem in
 * @param name name of the new subsystem; a valid string is required
 * @param init_fn initializer function
 * @param fini_fn finalizer function
 * @param loop_fn loop/tick function
 * @param user data passed in calls to \p init_fn , \p fini_fn and \p loop_fn
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

ham_engine_api ham_nothrow ham_str8 ham_engine_subsys_name(const ham_engine_subsys *subsys);

ham_engine_api ham_nothrow bool ham_engine_subsys_running(const ham_engine_subsys *subsys);

/**
 * @brief Get a subsystems minimum time between loops.
 * @param subsys subsystem to query
 * @returns minimum dt of \p subsys , if \p subsys is ``NULL`` 0.0 is returned
 */
ham_engine_api ham_nothrow ham_f64 ham_engine_subsys_min_dt(ham_engine_subsys *subsys);

/**
 * @brief Set a subsystems minimum time between loops.
 * @param subsys subsystem to query
 * @param min_dt new minimum dt
 * @returns whether the minimum dt was successfully set
 */
ham_engine_api ham_nothrow bool ham_engine_subsys_set_min_dt(ham_engine_subsys *subsys, ham_f64 min_dt);

/**
 * @brief Launch a subsystem explicitly outside the engine.
 * @param subsys subsystem to launch
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
	template<typename App, typename ... Subsystems>
	class basic_engine_view;

	template<typename App, typename ... Subsystems>
	class basic_engine;

	template<typename Derived>
	class app_base{
		public:
			ham_engine_app *ptr() noexcept{ return &m_val; }
			const ham_engine_app *ptr() const noexcept{ return &m_val; }

			str8 dir() const noexcept{ return m_val.dir; }
			str8 name() const noexcept{ return m_val.name; }
			str8 display_name() const noexcept{ return m_val.display_name; }
			str8 author() const noexcept{ return m_val.author; }
			str8 license() const noexcept{ return m_val.license; }
			str8 description() const noexcept{ return m_val.description; }
			class version version() const noexcept{ return m_val.version; }

			void set_dir(const str8 &new_dir) noexcept{ m_val.dir = new_dir; }
			void set_name(const str8 &new_name) noexcept{ m_val.name = new_name; }
			void set_display_name(const str8 &new_display) noexcept{ m_val.display_name = new_display; }
			void set_author(const str8 &new_author) noexcept{ m_val.author = new_author; }
			void set_license(const str8 &new_license) noexcept{ m_val.license = new_license; }
			void set_description(const str8 &new_description) noexcept{ m_val.description = new_description; }
			void set_version(const class version &new_version) noexcept{ m_val.version = new_version; }

		protected:
			app_base(
				const str8 &dir_,
				const str8 &name_,
				const str8 &display_name_,
				const str8 &author_,
				const class version &version_,
				const str8 &license_ = "All rights reserved.",
				const str8 &description_ = HAM_EMPTY_STR8
			){
				m_val.dir = dir_;
				m_val.name = name_;
				m_val.display_name = display_name_;
				m_val.author = author_;
				m_val.description = description_;
				m_val.license = license_;
				m_val.version = version_;

				m_val.init = init_dispatch;
				m_val.fini = fini_dispatch;
				m_val.loop = loop_dispatch;
				m_val.user = static_cast<Derived*>(this);
			}

		private:
			static inline bool init_dispatch(ham_engine *engine, void *user){
				const auto self = reinterpret_cast<Derived*>(user);
				return self->init(engine);
			}

			static inline void fini_dispatch(ham_engine *engine, void *user){
				const auto self = reinterpret_cast<Derived*>(user);
				self->fini(engine);
			}

			static inline void loop_dispatch(ham_engine *engine, f64 dt, void *user){
				const auto self = reinterpret_cast<Derived*>(user);
				self->loop(engine, dt);
			}

			ham_engine_app m_val;
	};

	class subsystem_exception: public exception{};

	class subsystem_ctor_error: public subsystem_exception{
		public:
			const char *api() const noexcept{ return "ham::subsystem_base::subsystem_base"; }
			const char *what() const noexcept{ return "Error in ham_engine_subsys_create"; }
	};

	template<typename Derived>
	class subsystem_base{
		public:
			str8 name() const noexcept{ return ham_engine_subsys_name(m_subsys); }

			bool running() const noexcept{ return ham_engine_subsys_running(m_subsys); }

			f64 min_dt() const noexcept{ return ham_engine_subsys_min_dt(m_subsys); }
			bool set_min_dt(f64 new_min_dt) noexcept{ return ham_engine_subsys_set_min_dt(m_subsys, new_min_dt); }

			bool launch() noexcept{ return ham_engine_subsys_launch(m_subsys); }

		protected:
			explicit subsystem_base(ham_engine *engine)
				: m_subsys(
					ham_engine_subsys_create(
						engine,
						str8(Derived::name()),
						init_impl, fini_impl, loop_impl,
						static_cast<Derived*>(this)
					)
				){
					if(!m_subsys){
						throw subsystem_ctor_error();
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

	namespace detail{
		template<typename T>
		class subsystem_interface{};
	}

	template<bool Mutable = true, class Interface = detail::subsystem_interface<void>>
	class basic_subsystem_view: public Interface{
		public:
			using pointer = std::conditional_t<Mutable, ham_engine_subsys*, const ham_engine_subsys*>;

			basic_subsystem_view(pointer ptr_) noexcept
				: m_ptr(ptr_){}

			operator pointer() const noexcept{ return m_ptr; }

			str8 name() const noexcept{ return ham_engine_subsys_name(m_ptr); }
			bool running() const noexcept{ return ham_engine_subsys_running(m_ptr); }

			f64 min_dt() const noexcept{ return ham_engine_subsys_min_dt(m_ptr); }

			template<
				bool IsMutable = Mutable,
				std::enable_if_t<IsMutable, int> = 0
			>
			bool set_min_dt(f64 new_min_dt) const noexcept{ return ham_engine_subsys_set_min_dt(m_ptr, new_min_dt); }

			template<
				bool IsMutable = Mutable,
				std::enable_if_t<IsMutable, int> = 0
			>
			bool launch() const noexcept{ return ham_engine_subsys_launch(m_ptr); }

		private:
			pointer m_ptr;
	};

	namespace detail{
		template<typename T, typename...>
		struct id_map{ using type = T; };

		template<typename T, typename ... Us>
		using id_map_t = typename id_map<T, Us...>::type;

		template<typename ... Us, typename T>
		constexpr inline decltype(auto) id_map_v(const T &val) noexcept{ return (val); }
	}

	class engine_exception: public exception{};

	class engine_init_error: public engine_exception{
		public:
			const char *api() const noexcept override{ return "ham::basic_engine::basic_engine"; }
			const char *what() const noexcept override{ return "Error in ham_engine_create"; }
	};

	template<typename App, typename ... Subsystems>
	class basic_engine{
		public:
			using subsystem_tuple = std::tuple<Subsystems...>;

			basic_engine()
				: m_app()
				, m_engine(ham_engine_create2(m_app.ptr()))
				, m_subsystems(detail::id_map_v<Subsystems>(m_engine)...)
			{}

			~basic_engine(){
				ham_engine_destroy(m_engine);
			}

			ham_engine *ptr() noexcept{ return m_engine; }
			const ham_engine *ptr() const noexcept{ return m_engine; }

			bool request_exit(){ return ham_engine_request_exit(m_engine); }

			int exec(){ return ham_engine_exec(m_engine); }

			int main(){
				const int result = ham_engine_main(m_engine);
				m_engine = nullptr;
				return result;
			}

			template<usize Idx>
			auto subsystem() noexcept
				-> basic_subsystem_view<true, detail::subsystem_interface<std::tuple_element_t<Idx, subsystem_tuple>>>
			{
				return std::get<Idx>(m_subsystems).ptr();
			}

			template<usize Idx>
			auto subsystem() const noexcept
				-> basic_subsystem_view<false, detail::subsystem_interface<std::tuple_element_t<Idx, subsystem_tuple>>>
			{
				return std::get<Idx>(m_subsystems).ptr();
			}

			template<typename Subsys>
			auto subsystem() noexcept
				-> basic_subsystem_view<true, detail::subsystem_interface<Subsys>>
			{
				return std::get<Subsys>(m_subsystems).ptr();
			}

			template<typename Subsys>
			auto subsystem() const noexcept
				-> basic_subsystem_view<false, detail::subsystem_interface<Subsys>>
			{
				return std::get<Subsys>(m_subsystems).ptr();
			}

		private:
			struct initial_tag{};

			template<typename F>
			void for_subsystems(F &&f){
				for_subsystems_impl(std::forward<F>(f), meta::make_index_seq<sizeof...(Subsystems)>());
			}

			template<typename F, usize ... Is>
			void for_subsystems_impl(F &&f, meta::index_seq<Is...>){
				(std::forward<F>(f)(std::get<Is>(m_subsystems)), ...);
			}

			App m_app;
			ham_engine *m_engine;
			subsystem_tuple m_subsystems;
	};

	template<typename App, typename ... Subsystems>
	class basic_engine_view{
		public:
			using subsystem_tuple = std::tuple<Subsystems...>;

			basic_engine_view(basic_engine<App, Subsystems...> &engine) noexcept
				: m_engine(engine.ptr()){}

			basic_engine_view(ham_engine *engine) noexcept
				: m_engine(engine){}

			bool request_exit() const{ return ham_engine_request_exit(m_engine); }

			int exec() const{ return ham_engine_exec(m_engine); }

			int main(){
				const int result = ham_engine_main(m_engine);
				m_engine = nullptr;
				return result;
			}

			template<usize Idx>
			auto subsystem() noexcept
				-> basic_subsystem_view<true, detail::subsystem_interface<std::tuple_element_t<Idx, subsystem_tuple>>>
			{
				return ham_engine_get_subsystem(m_engine, Idx);
			}

			template<usize Idx>
			auto subsystem() const noexcept
				-> basic_subsystem_view<false, detail::subsystem_interface<std::tuple_element_t<Idx, subsystem_tuple>>>
			{
				return ham_engine_get_subsystem(m_engine, Idx);
			}

		private:
			ham_engine *m_engine;
	};
}

#endif // __cplusplus

/**
 * @}
 */

#endif // !HAM_ENGINE_H
