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

typedef bool(*ham_ptr_iterate_fn)(void *ptr, void *user);

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

#ifdef __GNUC__

#define ham_iterate(elem, beg_, end_) \
	for(ham_typeof(beg_) elem = (beg_); elem < (end_); ++elem)

#endif // __GNUC__

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
				: m_data{ .ptr = nullptr }
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
}

#endif // __cplusplus

/**
 * @}
 */

#endif // !HAM_FUNCTIONAL_H
