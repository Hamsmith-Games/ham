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

#ifndef HAM_FUNCTIONAL_H
#define HAM_FUNCTIONAL_H 1

/**
 * @defgroup HAM_FUNCTIONAL Functional programming helpers
 * @ingroup HAM
 * @{
 */

#include "memory.h"
#include "buffer.h"
#include "async.h"
#include "log.h"

typedef bool(*ham_ptr_iterate_fn)(void *ptr, void *user);

ham_used
static inline ham_usize ham_ptr_iterate(
	void *beg, void *end,
	ham_usize byte_step,
	ham_ptr_iterate_fn fn, void *user
){
	if(beg == end) return 0;
	else if((char*)end < (char*)beg) return (ham_usize)-1;

	if(fn){
		ham_usize counter = 0;
		for(auto it = (char*)beg; it < (char*)end; it += byte_step){
			if(!fn(it, user)) return counter;
			++counter;
		}

		return counter;
	}
	else{
		return ((ham_uptr)end - (ham_uptr)beg)/byte_step;
	}
}

#define ham_iterate(elem, beg_, end_) \
	for(ham_typeof(beg_) elem = (beg_); elem < (end_); ++elem)

/**
 * @brief A signal connection.
 * @see ham_signal
 */
typedef struct ham_signal_connection{
	ham_uptr id;
	void(*fptr)(void *user, void *data);
	void *user;
} ham_signal_connection;

/**
 * @brief An asynchronous signal.
 */
typedef struct ham_signal{
	//! @brief Buffer of \ref ham_signal_connection
	ham_buffer connections;

	//! @cond ignore
	ham_uptr _impl_counter;
	ham_mutex _impl_mut;
	//! @endcond
} ham_signal;

ham_nonnull_args(1) ham_used
ham_nothrow static inline bool ham_signal_init(ham_signal *sig){
	if(!ham_mutex_init(&sig->_impl_mut, HAM_MUTEX_NORMAL)){
		ham_logapierrorf("Error in ham_mutex_init");
		return false;
	}

	if(!ham_buffer_init(&sig->connections, alignof(ham_signal_connection), 16 * sizeof(ham_signal_connection))){
		ham_logapierrorf("Error in ham_buffer_init");
		ham_mutex_finish(&sig->_impl_mut);
		return false;
	}

	sig->_impl_counter = 0;

	return true;
}

ham_nonnull_args(1) ham_used
ham_nothrow static inline void ham_signal_finish(ham_signal *sig){
	const bool mut_locked = ham_mutex_lock(&sig->_impl_mut);
	if(!mut_locked){
		ham_logapiwarnf("Error in ham_mutex_lock");
	}

	ham_buffer_finish(&sig->connections);

	if(mut_locked && !ham_mutex_unlock(&sig->_impl_mut)){
		ham_logapiwarnf("Error in ham_mutex_unlock");
	}

	ham_mutex_finish(&sig->_impl_mut);

	// TODO: only in debug builds?
	std::memset(sig, 0, sizeof(ham_signal));
}

ham_nonnull_args(1, 2) ham_used
ham_nothrow static inline ham_uptr ham_signal_connect(ham_signal *sig, void(*fptr)(void *user, void *data), void *user){
	if(!ham_mutex_lock(&sig->_impl_mut)){
		ham_logapierrorf("Error in ham_mutex_lock");
		return (ham_uptr)-1;
	}

	const ham_uptr new_off = ham_buffer_size(&sig->connections);

	ham_signal_connection conn;
	conn.id = sig->_impl_counter;
	conn.fptr = fptr;
	conn.user = user;

	if(!ham_buffer_insert(&sig->connections, new_off, &conn, sizeof(conn))){
		if(!ham_mutex_unlock(&sig->_impl_mut)){
			ham_logapiwarnf("Error in ham_mutex_unlock");
		}

		ham_logapierrorf("Error in ham_buffer_insert");

		return (ham_uptr)-1;
	}

	++sig->_impl_counter;

	if(!ham_mutex_unlock(&sig->_impl_mut)){
		ham_logapiwarnf("Error in ham_mutex_unlock");
	}

	return conn.id;
}

ham_nonnull_args(1) ham_used
ham_nothrow static inline void ham_signal_emit(ham_signal *sig, void *data){
	const ham_signal_connection *const conns = (const ham_signal_connection*)ham_buffer_data(&sig->connections);
	const ham_uptr n = ham_buffer_size(&sig->connections) / sizeof(ham_signal_connection);
	for(ham_uptr i = 0; i < n; i++){
		conns[i].fptr(conns[i].user, data);
	}
}

/**
 * @defgroup HAM_FUNCTIONAL_WORK_GROUP Work groups
 * @{
 */

/**
 * @brief A workgroup item.
 * @see ham_workgroup
 */
typedef struct ham_workgroup_item{
	void(*fn)(void*);
	void *user;
} ham_workgroup_item;

/**
 * @brief A FIFO work queue.
 */
typedef struct ham_workgroup{
	//! @brief Buffer of \ref ham_workgroup_item
	ham_buffer items;

	//! @cond ignore
	ham_mutex _impl_mut;
	//! @endcond
} ham_workgroup;

ham_nonnull_args(1)
ham_nothrow static inline bool ham_workgroup_init(ham_workgroup *group){
	if(!ham_mutex_init(&group->_impl_mut, HAM_MUTEX_NORMAL)){
		ham_logapierrorf("Error in ham_mutex_init");
		return false;
	}

	if(!ham_buffer_init(&group->items, alignof(ham_workgroup_item), 16)){
		ham_logapierrorf("Error in ham_buffer_init_allocator");
		ham_mutex_finish(&group->_impl_mut);
		return false;
	}

	return true;
}

ham_nonnull_args(1)
ham_nothrow static inline void ham_workgroup_finish(ham_workgroup *group){
	ham_buffer_finish(&group->items);
	ham_mutex_finish(&group->_impl_mut);
}

ham_nonnull_args(1)
ham_nothrow static inline bool ham_workgroup_push(ham_workgroup *group, void(*fn)(void*), void *user){
	if(!ham_mutex_lock(&group->_impl_mut)){
		ham_logapierrorf("Error in ham_mutex_lock");
		return false;
	}

	const ham_usize new_idx = ham_buffer_size(&group->items);
	const ham_usize new_off = new_idx * sizeof(ham_workgroup_item);

	const ham_workgroup_item new_item = (ham_workgroup_item){ fn, user };

	const bool result = ham_buffer_insert(&group->items, new_off, &new_item, sizeof(ham_workgroup_item));
	if(!result){
		ham_logapierrorf("Error in ham_buffer_insert");
	}

	if(!ham_mutex_unlock(&group->_impl_mut)){
		ham_logapiwarnf("Error in ham_mutex_unlock");
	}

	return result;
}

ham_nonnull_args(1)
static inline bool ham_workgroup_pop_n(ham_workgroup *group, ham_usize max_n){
	if(!ham_mutex_lock(&group->_impl_mut)){
		ham_logapierrorf("Error in ham_mutex_lock");
		return false;
	}

	const ham_usize byte_len = ham_buffer_size(&group->items);
	if(byte_len){
		ham_workgroup_item *const items = (ham_workgroup_item*)ham_buffer_data(&group->items);
		const ham_usize n = ham_min(max_n, byte_len / sizeof(ham_workgroup_item));
		for(ham_usize i = 0; i < n; i++){
			ham_workgroup_item *const item = items + i;
			item->fn(item->user);
		}

		if(!ham_buffer_erase(&group->items, 0, n * sizeof(ham_workgroup_item))){
			ham_logapiwarnf("Error in ham_buffer_erase");
		}
	}

	if(!ham_mutex_unlock(&group->_impl_mut)){
		ham_logapiwarnf("Error in ham_mutex_unlock");
	}

	return true;
}

ham_nonnull_args(1)
static inline bool ham_workgroup_pop_all(ham_workgroup *group){
	return ham_workgroup_pop_n(group, HAM_USIZE_MAX);
}

ham_nonnull_args(1)
static inline bool ham_workgroup_pop(ham_workgroup *group){
	return ham_workgroup_pop_n(group, 1);
}

/**
 * @}
 */

#ifdef __cplusplus

namespace ham{
	class null_function_call: public std::exception{
		public:
			const char *what() const noexcept override{ return "NULL function called"; }
	};

	template<typename Sig>
	class indirect_function;

	template<typename Ret, typename ... Args>
	class indirect_function<Ret(Args...)>{
		public:
			indirect_function() noexcept
				: m_data{ .fptr = nullptr }
				, m_dispatcher(null_dispatcher)
				, m_deleter(null_deleter)
			{}

			indirect_function(Ret(*fptr)(Args...)) noexcept
				: m_data{ .fptr = fptr }
				, m_dispatcher(fptr_dispatcher)
				, m_deleter(null_deleter)
			{}

			template<typename Func>
			indirect_function(Func fn) noexcept{
				const auto allocator = ham_current_allocator();
				m_data.functor = ham_allocator_new(allocator, functor<Func>, allocator, std::move(fn));
				m_dispatcher = functor_dispatcher;
				m_deleter = functor_deleter;
			}

			indirect_function(indirect_function &&other) noexcept
				: m_data(std::exchange(other.m_data, fn_data{ .functor = nullptr }))
				, m_dispatcher(std::exchange(other.m_dispatcher, null_dispatcher))
				, m_deleter(std::exchange(other.m_deleter, null_deleter))
			{}

			~indirect_function(){
				if(m_data.functor){ // this *should* be okay
					m_deleter(m_data);
				}
			}

			indirect_function &operator=(Ret(*fptr)(Args...)) noexcept{
				if(m_data.functor){
					m_deleter(m_data);
				}

				m_data.fptr = fptr;
				m_dispatcher = fptr_dispatcher;
				m_deleter = null_deleter;

				return *this;
			}

			template<typename Func>
			indirect_function &operator=(Func fn) noexcept{
				if(m_data.functor){
					m_deleter(m_data);
				}

				const auto allocator = ham_current_allocator();

				m_data.functor = ham_allocator_new(allocator, functor<Func>, allocator, std::move(fn));
				m_dispatcher = functor_dispatcher;
				m_deleter = functor_deleter;

				return *this;
			}

			indirect_function &operator=(indirect_function &&other) noexcept{
				if(this != &other){
					if(m_data.functor){
						m_deleter(m_data);
					}

					m_data = std::exchange(other.m_data, fn_data{ .functor = nullptr });
					m_dispatcher = std::exchange(other.m_dispatcher, null_dispatcher);
					m_deleter = std::exchange(other.m_deleter, null_deleter);
				}

				return *this;
			}

			template<typename ... UArgs>
			Ret operator()(UArgs &&... args) const{
				return m_dispatcher(m_data, std::forward<UArgs>(args)...);
			}

		private:
			struct functor_base{
				explicit functor_base(const ham_allocator *allocator_) noexcept
					: allocator(allocator_){}

				virtual ~functor_base() = default;
				virtual Ret call(Args...) const = 0;

				const ham_allocator *allocator;
			};

			template<typename Func>
			struct functor: public functor_base{
				template<typename UFunc>
				functor(const ham_allocator *allocator_, UFunc &&f_) noexcept(noexcept(Func(std::forward<UFunc>(f_))))
					: functor_base(allocator_), f(std::forward<UFunc>(f_)){}

				Ret call(Args ... args) const override{ return f(args...); }

				Func f;
			};

			union fn_data{
				functor_base *functor;
				Ret(*fptr)(Args...);
			};

			using deleter_fn_type  = void(*)(fn_data) noexcept;
			using dispatch_fn_type = Ret(*)(fn_data, Args... args);

			[[noreturn]]
			static Ret null_dispatcher(fn_data, Args...){
				throw null_function_call();
			}

			static Ret fptr_dispatcher(fn_data data, Args ... args){
				return data.fptr(args...);
			}

			static Ret functor_dispatcher(fn_data data, Args ... args){
				return data.functor->call(std::move(args)...);
			}

			static void null_deleter(fn_data) noexcept{}

			static void functor_deleter(fn_data data) noexcept{
				const auto allocator = data.functor->allocator;
				std::destroy_at(data.functor);
				ham_allocator_free(allocator, data.functor); // TODO: check this doesn't need to be adjusted for virtualness or somein'
			}

			fn_data m_data;
			dispatch_fn_type m_dispatcher;
			deleter_fn_type m_deleter;
	};

	class workgroup_exception: public exception{};

	class workgroup_init_error: public workgroup_exception{
		public:
			const char *api() const noexcept override{ return "workgroup::workgroup"; }
			const char *what() const noexcept override{ return "error in ham_workgroup_init"; }
	};

	class workgroup{
		public:
			workgroup(){
				if(!ham_workgroup_init(&m_group)){
					throw workgroup_init_error();
				}
			}

			workgroup(workgroup &&other) noexcept
				: m_group(other.m_group)
			{
				memset(&m_group, 0, sizeof(ham_workgroup));
			}

			~workgroup(){
				ham_workgroup_finish(&m_group);
			}

			workgroup &operator=(workgroup &&other) noexcept{
				if(this != &other){
					ham_workgroup_finish(&m_group);

					m_group = other.m_group;
					memset(&other.m_group, 0, sizeof(ham_workgroup));
				}

				return *this;
			}

			bool push(void(*fptr)(void*), void *user) noexcept{
				return ham_workgroup_push(&m_group, fptr, user);
			}

			bool push(indirect_function<void()> fn){
				const auto allocator = ham_current_allocator();

				const auto data = ham_allocator_new(allocator, indirect_data);

				data->allocator = allocator;
				data->fn = std::move(fn);

				if(!ham_workgroup_push(&m_group, indirect_dispatcher, data)){
					ham_logapierrorf("Error in ham_workgroup_push");
					ham_allocator_delete(allocator, data);
					return false;
				}

				return true;
			}

			bool pop(ham_usize max_n){ return ham_workgroup_pop_n(&m_group, max_n); }

			bool pop(){ return ham_workgroup_pop(&m_group); }

			bool pop_all(){ return ham_workgroup_pop_all(&m_group); }

		private:
			struct indirect_data{
				const ham_allocator *allocator;
				indirect_function<void()> fn;
			};

			static void indirect_dispatcher(void *user){
				const auto data = (indirect_data*)user;
				data->fn();
				ham_allocator_delete(data->allocator, data);
			}

			ham_workgroup m_group;
	};
}

#endif // __cplusplus

/**
 * @}
 */

#endif // !HAM_FUNCTIONAL_H
