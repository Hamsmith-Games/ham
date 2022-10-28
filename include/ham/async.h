/*
 * Ham Runtime
 * Copyright (C) 2022 Hamsmith Ltd.
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

#include "log.h"

// TODO: check for win32 and use *their* primitives
#include <semaphore.h>

HAM_C_API_BEGIN

/**
 * @defgroup HAM_ASYNC_SEM Semaphores
 * @{
 */

typedef struct ham_sem ham_sem;

typedef struct ham_sem{
	//! @cond ignore
	sem_t _impl_sem;
	//! @endcond
} ham_sem;

ham_nonnull_args(1)
ham_nothrow static inline bool ham_sem_init(ham_sem *sem, ham_u32 initial_val){
	const int res = sem_init(&sem->_impl_sem, 0, initial_val);
	if(res != 0){
		ham_logapierrorf("Error in sem_init: %s", strerror(errno));
		return false;
	}

	return true;
}

ham_nonnull_args(1)
ham_nothrow static inline void ham_sem_finish(ham_sem *sem){
	const int res = sem_destroy(&sem->_impl_sem);
	if(res != 0){
		ham_logapierrorf("Error in sem_close: %s", strerror(errno));
	}
}

ham_nonnull_args(1)
ham_nothrow static inline bool ham_sem_post(ham_sem *sem){
	const int res = sem_post(&sem->_impl_sem);
	if(res != 0){
		ham_logapierrorf("Error in sem_post: %s", strerror(errno));
		return false;
	}

	return true;
}

ham_nonnull_args(1)
ham_nothrow static inline bool ham_sem_wait(ham_sem *sem){
	const int res = sem_wait(&sem->_impl_sem);
	if(res != 0){
		ham_logapierrorf("Error in sem_wait: %s", strerror(errno));
		return false;
	}

	return true;
}

/**
 * Try to wait for a semaphore to post.
 * @param sem semaphore to try to wait on
 * @returns `1` if the semphore would block, `0` if the semaphore was decremented or `-1` on error
 */
ham_nonnull_args(1)
ham_nothrow static inline ham_i32 ham_sem_try_wait(ham_sem *sem){
	const int res = sem_trywait(&sem->_impl_sem);
	if(res != 0){
		if(errno == EAGAIN) return 1;

		ham_logapierrorf("Error in sem_trywait: %s", strerror(errno));
		return -1;
	}

	return 0;
}

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

typedef struct ham_mutex{
	//! @cond ignore
	pthread_mutex_t _impl_mut;
	//! @endcond
} ham_mutex;

ham_nonnull_args(1)
ham_nothrow static inline bool ham_mutex_init(ham_mutex *mut, ham_mutex_kind kind){
	pthread_mutexattr_t mut_attr;
	int res = pthread_mutexattr_init(&mut_attr);
	if(res != 0){
		ham_logapierrorf("Error in pthread_mutexattr_init: %s", strerror(res));
		return false;
	}

	switch (kind) {
		case HAM_MUTEX_RECURSIVE:{
			mut->_impl_mut = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
			pthread_mutexattr_settype(&mut_attr, PTHREAD_MUTEX_RECURSIVE_NP);
			break;
		}

		case HAM_MUTEX_ERRORCHECK:{
			mut->_impl_mut = PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP;
			pthread_mutexattr_settype(&mut_attr, PTHREAD_MUTEX_ERRORCHECK_NP);
			break;
		}

		default:{
			ham_logapiwarnf("Unrecognized ham_mutex_kind (0x%x) using HAM_MUTEX_NORMAL", kind);
		}

		case HAM_MUTEX_NORMAL:{
			mut->_impl_mut = PTHREAD_MUTEX_INITIALIZER;
			pthread_mutexattr_settype(&mut_attr, PTHREAD_MUTEX_NORMAL);
			break;
		}
	}

	res = pthread_mutex_init(&mut->_impl_mut, &mut_attr);
	if(res != 0){
		ham_logapierrorf("Error in pthread_mutex_init: %s", strerror(res));

		res = pthread_mutexattr_destroy(&mut_attr);
		if(res != 0){
			ham_logapiwarnf("Error in pthread_mutexattr_destroy: %s", strerror(res));
		}

		return false;
	}

	res = pthread_mutexattr_destroy(&mut_attr);
	if(res != 0){
		ham_logapiwarnf("Error in pthread_mutexattr_destroy: %s", strerror(res));
	}

	return true;
}

ham_nothrow static inline void ham_mutex_finish(ham_mutex *mut){
	if(ham_unlikely(!mut)) return;

	const int res = pthread_mutex_destroy(&mut->_impl_mut);
	if(res != 0){
		ham_logapiwarnf("Error in pthread_mutex_destroy: %s", strerror(res));
	}
}

ham_nonnull_args(1)
ham_nothrow static inline bool ham_mutex_lock(ham_mutex *mut){
	const int res = pthread_mutex_lock(&mut->_impl_mut);
	if(res != 0){
		ham_logapierrorf("Error in pthread_mutex_lock: %s", strerror(res));
		return false;
	}

	return true;
}

/**
 * Try to lock a mutex.
 * @param mut mutex to try lock
 * @returns `1` if the mutex would block, `0` if the mutex was locked or `-1` on error
 */
ham_nonnull_args(1)
ham_nothrow static inline ham_i32 ham_mutex_try_lock(ham_mutex *mut){
	const int res = pthread_mutex_trylock(&mut->_impl_mut);

	switch(res){
		case 0: return 0;
		case EBUSY: return 1;

		default:{
			ham_logapierrorf("Error in pthread_mutex_trylock: %s", strerror(res));
			return -1;
		}
	}
}

ham_nonnull_args(1)
ham_nothrow static inline bool ham_mutex_unlock(ham_mutex *mut){
	const int res = pthread_mutex_unlock(&mut->_impl_mut);
	if(res != 0){
		ham_logapierrorf("Error in pthread_mutex_unlock: %s", strerror(res));
		return false;
	}

	return true;
}

/**
 * @}
 */

/**
 * @defgroup HAM_ASYNC_COND Condition variables
 * @{
 */

typedef struct ham_cond{
	//! @cond ignore
	pthread_cond_t _impl_cond;
	//! @endcond
} ham_cond;

ham_nonnull_args(1)
ham_nothrow static inline bool ham_cond_init(ham_cond *cond){
	cond->_impl_cond = PTHREAD_COND_INITIALIZER;

	const int res = pthread_cond_init(&cond->_impl_cond, nullptr);
	if(res != 0){
		ham_logapierrorf("Error in pthread_cond_init: %s", strerror(res));
		return false;
	}

	return true;
}

ham_nonnull_args(1)
ham_nothrow static inline void ham_cond_finish(ham_cond *cond){
	const int res = pthread_cond_destroy(&cond->_impl_cond);
	if(res != 0){
		ham_logapierrorf("Error in pthread_cond_destroy: %s", strerror(res));
	}
}

ham_nonnull_args(1)
ham_nothrow static inline bool ham_cond_signal(ham_cond *cond){
	const int res = pthread_cond_signal(&cond->_impl_cond);
	if(res != 0){
		ham_logapierrorf("Error in pthread_cond_signal: %s", strerror(res));
		return false;
	}

	return true;
}

ham_nonnull_args(1)
ham_nothrow static inline bool ham_cond_broadcast(ham_cond *cond){
	const int res = pthread_cond_broadcast(&cond->_impl_cond);
	if(res != 0){
		ham_logapierrorf("Error in pthread_cond_broadcast: %s", strerror(res));
		return false;
	}

	return true;
}

ham_nonnull_args(1)
ham_nothrow static inline bool ham_cond_wait(ham_cond *cond, ham_mutex *mut){
	const int res = pthread_cond_wait(&cond->_impl_cond, &mut->_impl_mut);
	if(res != 0){
		ham_logapierrorf("Error in pthread_cond_wait: %s", strerror(res));
		return false;
	}

	return true;
}

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
	class sem_exception: public exception{};

	class sem_init_error: public sem_exception{
		public:
			const char *api() const noexcept override{ return "sem::sem"; }
			const char *what() const noexcept override{ return "error in ham_sem_init"; }
	};

	class sem{
		public:
			sem(u32 initial_value = 0){
				if(!ham_sem_init(&m_sem, initial_value)){
					throw sem_init_error();
				}
			}

			sem(sem &&other) noexcept
				: m_sem(other.m_sem)
			{
				std::memset(&other.m_sem, 0, sizeof(ham_sem));
			}

			~sem(){
				ham_sem_finish(&m_sem);
			}

			sem &operator=(sem &&other) noexcept{
				if(this != &other){
					ham_sem_finish(&m_sem);

					m_sem = other.m_sem;
					std::memset(&other.m_sem, 0, sizeof(ham_sem));
				}

				return *this;
			}

			bool post() noexcept{ return ham_sem_post(&m_sem); }
			bool wait() noexcept{ return ham_sem_wait(&m_sem); }

		private:
			ham_sem m_sem;
	};

	class mutex_exception: public exception{};

	class mutex_init_error: public mutex_exception{
		public:
			const char *api() const noexcept override{ return "basic_mutex::basic_mutex"; }
			const char *what() const noexcept override{ return "error in ham_mutex_init"; }
	};

	enum class mutex_kind{
		normal = HAM_MUTEX_NORMAL,
		recursive = HAM_MUTEX_RECURSIVE,
		errorcheck = HAM_MUTEX_ERRORCHECK,
	};

	template<mutex_kind Kind>
	class basic_mutex{
		public:
			basic_mutex(){
				if(!ham_mutex_init(&m_mut, static_cast<ham_mutex_kind>(Kind))){
					throw mutex_init_error();
				}
			}

			basic_mutex(basic_mutex &&other) noexcept
				: m_mut(other.m_mut)
			{
				std::memset(&other.m_mut, 0, sizeof(ham_mutex));
			}

			~basic_mutex(){
				ham_mutex_finish(&m_mut);
			}

			basic_mutex &operator=(basic_mutex &&other) noexcept{
				if(this != &other){
					ham_mutex_finish(&m_mut);

					m_mut = other.m_mut;
					std::memset(&other.m_mut, 0, sizeof(ham_mutex));
				}

				return *this;
			}

			bool lock() noexcept{ return ham_mutex_lock(&m_mut); }
			bool unlock() noexcept{ return ham_mutex_unlock(&m_mut); }
			bool try_lock() noexcept{ return ham_mutex_try_lock(&m_mut) == 0; }

		private:
			ham_mutex m_mut;

			friend class cond;
	};

	using mutex = basic_mutex<mutex_kind::normal>;
	using recursive_mutex = basic_mutex<mutex_kind::recursive>;
	using errorcheck_mutex = basic_mutex<mutex_kind::errorcheck>;

	class mutex_lock_error: public exception{
		public:
			const char *api() const noexcept override{ return "ham::basic_mutex"; }
			const char *what() const noexcept override{ return "Error locking mutex"; }
	};

	template<typename Mutex>
	class unique_lock{
		public:
			explicit unique_lock(Mutex &mut, bool lock_now = true)
				: m_mut(&mut), m_locked(lock_now ? mut.lock() : false)
			{
				if(lock_now && !m_locked){
					throw mutex_lock_error();
				}
			}

			~unique_lock(){
				if(m_locked) m_mut->unlock();
			}

			bool lock() noexcept{
				if(!m_mut) return false;
				else if(m_locked) return true;
				else return (m_locked = m_mut->lock());
			}

			bool unlock() noexcept{
				if(!m_mut) return false;
				else if(!m_locked) return true;
				else return (m_locked = !m_mut->unlock());
			}

		private:
			Mutex *m_mut;
			bool m_locked;

			friend class cond;
	};

	template<>
	class unique_lock<ham_mutex*>{
		public:
			explicit unique_lock(ham_mutex *mut, bool lock_now = true)
				: m_mut(mut), m_locked(lock_now ? ham_mutex_lock(mut) : false)
			{
				if(lock_now && !m_locked){
					throw mutex_lock_error();
				}
			}

			~unique_lock(){
				if(m_locked) ham_mutex_unlock(m_mut);
			}

			bool lock() noexcept{
				if(!m_mut) return false;
				else if(m_locked) return true;
				else return (m_locked = ham_mutex_lock(m_mut));
			}

			bool unlock() noexcept{
				if(!m_mut) return false;
				else if(!m_locked) return true;
				else return (m_locked = !ham_mutex_unlock(m_mut));
			}

		private:
			ham_mutex *m_mut;
			bool m_locked;

			friend class cond;
	};

	template<typename Mutex>
	unique_lock(Mutex&) -> unique_lock<std::remove_reference_t<Mutex>>;

	unique_lock(ham_mutex*) -> unique_lock<ham_mutex*>;

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

	template<>
	class scoped_lock<>{};

	template<typename ... Muts>
	class scoped_lock<ham_mutex*, Muts...>{
		public:
			scoped_lock(ham_mutex *cmut, Muts &... muts)
				: m_cmut(ham_mutex_lock(cmut) ? cmut : nullptr)
				, m_inner(muts...)
			{
				if(!m_cmut){
					throw mutex_lock_error();
				}
			}

			~scoped_lock(){
				if(m_cmut) ham_mutex_unlock(m_cmut);
			}

		private:
			ham_mutex *m_cmut;
			scoped_lock<Muts...> m_inner;
	};

	template<typename ... Muts>
	scoped_lock(Muts&...) -> scoped_lock<std::remove_reference_t<Muts>...>;

	template<typename ... Muts>
	scoped_lock(ham_mutex*, Muts&...) -> scoped_lock<ham_mutex*, std::remove_reference_t<Muts>...>;

	//
	// Condition variables
	//

	class cond_exception: public exception{};

	class cond_init_error: public cond_exception{
		public:
			const char *api() const noexcept override{ return "cond::cond"; }
			const char *what() const noexcept override{ return "Error in ham_cond_init"; }
	};

	class cond{
		public:
			cond(){
				if(!ham_cond_init(&m_cond)){
					throw cond_init_error();
				}
			}

			cond(cond &&other) noexcept
				: m_cond(other.m_cond)
			{
				std::memset(&other.m_cond, 0, sizeof(ham_cond));
			}

			~cond(){
				ham_cond_finish(&m_cond);
			}

			cond &operator=(cond &&other) noexcept{
				if(this != &other){
					ham_cond_finish(&m_cond);

					m_cond = other.m_cond;
					std::memset(&other.m_cond, 0, sizeof(ham_cond));
				}

				return *this;
			}

			bool signal() noexcept{ return ham_cond_signal(&m_cond); }
			bool broadcast() noexcept{ return ham_cond_broadcast(&m_cond); }

			template<mutex_kind Kind>
			bool wait(unique_lock<basic_mutex<Kind>> &lock) noexcept{
				return ham_cond_wait(&m_cond, &lock.m_mut->m_mut);
			}

			template<mutex_kind Kind, typename CheckFn>
			bool wait(unique_lock<basic_mutex<Kind>> &lock, CheckFn &&check_fn){
				bool wait_res = false;
				while((wait_res = wait(lock)) && !check_fn()){}
				return wait_res;
			}

		private:
			ham_cond m_cond;
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
