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

#ifndef HAM_ASYNC_H
#define HAM_ASYNC_H 1

/**
 * @defgroup HAM_ASYNC Asynchronous utilities
 * @ingroup HAM
 * @{
 */

#include "typedefs.h"

HAM_C_API_BEGIN

/**
 * @defgroup HAM_ASYNC_SEM Semaphores
 * @{
 */

typedef struct ham_sem ham_sem;

ham_api ham_sem *ham_sem_create(ham_u32 initial_val);

ham_api ham_nothrow void ham_sem_destroy(ham_sem *sem);

ham_api ham_nothrow bool ham_sem_post(ham_sem *sem);

ham_api ham_nothrow bool ham_sem_wait(ham_sem *sem);

/**
 * Try to wait for a semaphore to post.
 * @param sem semaphore to try to wait on
 * @returns `1` if the semphore would block, `0` if the semaphore was decremented or `-1` on error
 */
ham_api ham_nothrow ham_i32 ham_sem_try_wait(ham_sem *sem);

/**
 * @}
 */

/**
 * @defgroup HAM_ASYNC_MUTEX Mutually exclusive locks
 * @{
 */

typedef enum ham_mutex_kind{
	HAM_MUTEX_NORMAL,
	HAM_MUTEX_RECURSIVE,
	HAM_MUTEX_ERRORCHECK,

	HAM_MUTEX_KIND_COUNT,
} ham_mutex_kind;

typedef struct ham_mutex ham_mutex;

ham_api ham_mutex *ham_mutex_create(ham_mutex_kind kind);

ham_api ham_nothrow void ham_mutex_destroy(ham_mutex *mut);

ham_api ham_nothrow bool ham_mutex_lock(ham_mutex *mut);

/**
 * Try to lock a mutex.
 * @param mut mutex to try lock
 * @returns `1` if the mutex would block, `0` if the mutex was locked or `-1` on error
 */
ham_api ham_nothrow ham_i32 ham_mutex_try_lock(ham_mutex *mut);

ham_api ham_nothrow bool ham_mutex_unlock(ham_mutex *mut);

/**
 * @}
 */

/**
 * @defgroup HAM_ASYNC_COND Condition variables
 * @{
 */

typedef struct ham_cond ham_cond;

ham_api ham_cond *ham_cond_create();

ham_api ham_nothrow void ham_cond_destroy(ham_cond *cond);

ham_api ham_nothrow bool ham_cond_signal(ham_cond *cond);
ham_api ham_nothrow bool ham_cond_broadcast(ham_cond *cond);

ham_api ham_nothrow bool ham_cond_wait(ham_cond *cond, ham_mutex *mut);

/**
 * @}
 */

/**
 * @defgroup HAM_ASYNC_THREADS Threads
 * @{
 */

typedef struct ham_thread ham_thread;

typedef ham_uptr(*ham_thread_fn)(void *user);

ham_api ham_thread *ham_thread_create(ham_thread_fn fn, void *user);

ham_api ham_nothrow void ham_thread_destroy(ham_thread *thd);

ham_api ham_nothrow bool ham_thread_join(ham_thread *thd, ham_uptr *ret);

ham_api ham_nothrow bool ham_thread_set_name(ham_thread *thd, ham_str8 name);

/**
 * @}
 */

HAM_C_API_END

#ifdef __cplusplus

#include "memory.h"

#include <tuple>
#include <functional>

namespace ham{
	class sem{
		public:
			sem(u32 initial_value = 0)
				: m_handle(ham_sem_create(initial_value)){}

			sem(sem&&) noexcept = default;

			sem &operator=(sem&&) noexcept = default;

			bool post() noexcept{ return ham_sem_post(m_handle.get()); }
			bool wait() noexcept{ return ham_sem_wait(m_handle.get()); }

		private:
			unique_handle<ham_sem*, ham_sem_destroy> m_handle;
	};

	enum class mutex_kind{
		normal = HAM_MUTEX_NORMAL,
		recursive = HAM_MUTEX_RECURSIVE,
		errorcheck = HAM_MUTEX_ERRORCHECK,
	};

	template<mutex_kind Kind>
	class basic_mutex{
		public:
			basic_mutex(): m_handle(ham_mutex_create(static_cast<ham_mutex_kind>(Kind))){}

			basic_mutex(basic_mutex&&) noexcept = default;

			basic_mutex &operator=(basic_mutex&&) noexcept = default;

			bool lock() noexcept{ return ham_mutex_lock(m_handle.get()); }
			bool unlock() noexcept{ return ham_mutex_unlock(m_handle.get()); }
			bool try_lock() noexcept{ return ham_mutex_try_lock(m_handle.get()) == 0; }

		private:
			unique_handle<ham_mutex*, ham_mutex_destroy> m_handle;

			friend class cond;
	};

	using mutex = basic_mutex<mutex_kind::normal>;
	using recursive_mutex = basic_mutex<mutex_kind::recursive>;
	using errorcheck_mutex = basic_mutex<mutex_kind::errorcheck>;

	template<typename Mutex>
	class unique_lock{
		public:
			unique_lock(Mutex &mut, bool lock_now = true) noexcept
				: m_mut(&mut), m_locked(lock_now ? mut.lock() : false)
			{}

			~unique_lock(){
				if(m_locked) m_mut->unlock();
			}

			bool lock() noexcept{
				if(m_locked) return true;
				return (m_locked = m_mut->lock());
			}

			bool unlock() noexcept{
				if(!m_locked) return true;
				return (m_locked = !m_mut->unlock());
			}

		private:
			Mutex *m_mut;
			bool m_locked;

			friend class cond;
	};

	template<typename Mutex>
	unique_lock(Mutex&) -> unique_lock<std::remove_reference_t<Mutex>>;

	template<typename ... Muts>
	class scoped_lock{
		public:
			scoped_lock(Muts &... muts) noexcept
				: m_muts(&muts...)
			{
				(muts.lock(), ...);
			}

			~scoped_lock(){
				unlock_all(meta::make_index_seq<sizeof...(Muts)>());
			}

		private:
			template<usize ... Is>
			void unlock_all(meta::index_seq<Is...>){
				(std::get<Is>(m_muts)->unlock(), ...);
			}

			std::tuple<std::remove_reference_t<Muts>*...> m_muts;
	};

	template<typename ... Muts>
	scoped_lock(Muts&...) -> scoped_lock<std::remove_reference_t<Muts>...>;

	//
	// Condition variables
	//

	class cond{
		public:
			cond(): m_handle(ham_cond_create()){}

			cond(cond&&) noexcept = default;

			cond &operator=(cond&&) noexcept = default;

			bool signal() noexcept{ return ham_cond_signal(m_handle.get()); }
			bool broadcast() noexcept{ return ham_cond_broadcast(m_handle.get()); }

			template<mutex_kind Kind>
			bool wait(unique_lock<basic_mutex<Kind>> &lock) noexcept{
				return ham_cond_wait(m_handle.get(), lock.m_mut->m_handle.get());
			}

			template<mutex_kind Kind, typename CheckFn>
			bool wait(unique_lock<basic_mutex<Kind>> &lock, CheckFn &&check_fn){
				bool wait_res = false;
				while((wait_res = wait(lock)) && !check_fn()){}
				return wait_res;
			}

		private:
			unique_handle<ham_cond*, ham_cond_destroy> m_handle;
	};

	//
	// Threads
	//

	class thread{
		public:
			thread() noexcept
				: m_allocator(nullptr)
				, m_f(nullptr)
				, m_thd(nullptr)
			{}

			template<typename Fn, typename ... Args>
			thread(Fn f, Args ... args)
				: m_allocator(ham_current_allocator())
			{
				auto func = [f_{std::move(f)}, args_{std::make_tuple(std::move(args)...)}]{
					std::apply(f_, args_);
				};

				using func_type = std::remove_cvref_t<decltype(func)>;

				m_f = ham_allocator_new(m_allocator, internal_fn_functor<func_type>, std::move(func));

				m_thd = ham_thread_create(thread_routine, this);
			}

			thread(ham_thread_fn f, void *user)
				: m_allocator(nullptr)
				, m_f(nullptr)
			{
				m_thd = ham_thread_create(f, user);
			}

			thread(thread &&other) noexcept
				: m_allocator(std::exchange(other.m_allocator, nullptr))
				, m_f(std::exchange(other.m_f, nullptr))
				, m_thd(std::exchange(other.m_thd, nullptr))
			{}

			~thread(){
				if(m_thd){
					ham_thread_destroy(m_thd);
				}

				if(m_f){
					ham_allocator_delete(m_allocator, m_f);
				}
			}

			thread &operator=(thread &&other){
				if(this != &other){
					if(m_thd) ham_thread_destroy(m_thd);
					if(m_f) ham_allocator_delete(m_allocator, m_f);

					m_allocator = std::exchange(other.m_allocator, nullptr);
					m_f = std::exchange(other.m_f, nullptr);
					m_thd = std::exchange(other.m_thd, nullptr);
				}

				return *this;
			}

			bool set_name(str8 name) noexcept{ return ham_thread_set_name(m_thd, name); }

			bool join(uptr *ret = nullptr) noexcept{ return ham_thread_join(m_thd, ret); }

		private:
			static uptr thread_routine(void *user){
				const auto self = (thread*)user;
				self->m_f->call();
				return 0;
			}

			struct internal_fn{
				virtual ~internal_fn() = default;
				virtual void call() = 0;
			};

			struct internal_fn_ptr: internal_fn{
				internal_fn_ptr(ham_thread_fn f, void *user) noexcept
					: m_f(f), m_user(user){}

				void call() override{ m_f(m_user); }

				ham_thread_fn m_f;
				void *m_user;
			};

			template<typename Functor>
			struct internal_fn_functor: internal_fn{
				template<typename Fn>
				internal_fn_functor(Fn &&f): m_f(std::forward<Fn>(f)){}

				void call() override{ m_f(); }

				Functor m_f;
			};

			const ham_allocator *m_allocator;
			internal_fn *m_f;
			ham_thread *m_thd;
	};
}

#endif // __cplusplus

/**
 * @}
 */

#endif // !HAM_ASYNC_H
