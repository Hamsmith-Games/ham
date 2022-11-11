/*
 * Ham Runtime
 * Copyright (C) 2022 Keith Hammond
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

#ifndef HAM_MEMORY_H
#define HAM_MEMORY_H 1

/**
 * @defgroup HAM_MEMORY Memory management
 * @ingroup HAM
 * @{
 */

#include "typedefs.h"

#ifdef __cplusplus
#	include <new>
#endif

HAM_C_API_BEGIN

ham_api ham_usize ham_get_page_size();

ham_api void *ham_map_pages(ham_usize num_pages);
ham_api bool ham_unmap_pages(void *mem, ham_usize num_pages);

typedef void*(*ham_alloc_fn)(ham_usize alignment, ham_usize size, void *user);
typedef void(*ham_free_fn)(void *mem, void *user);

typedef struct ham_allocator{
	ham_alloc_fn alloc;
	ham_free_fn free;
	void *user;
} ham_allocator;

static inline void *ham_allocator_alloc(const ham_allocator *allocator, ham_usize alignment, ham_usize size){
	return allocator->alloc(alignment, size, allocator->user);
}

static inline void ham_allocator_free(const ham_allocator *allocator, void *mem){
	allocator->free(mem, allocator->user);
}



#ifdef __cplusplus
#	define ham_allocator_new(allocator, t, ...) (new(ham_allocator_alloc((allocator), alignof(t), sizeof(t))) t(__VA_ARGS__))
#	define ham_allocator_delete(allocator, ptr) (std::destroy_at((ptr)), ham_allocator_free((allocator), (ptr)))
#	define ham_allocator_new_array(allocator, t, n) (new(ham_allocator_alloc((allocator), alignof(t), sizeof(t) * n)) t[n])
#	define ham_allocator_delete_array(allocator, ptr, n) \
		([](const auto allocator_, const auto ptr_, const auto n_){ \
			for(ham_usize i = 0; i < n_; i++) std::destroy_at(ptr_ + i); \
			ham_allocator_free(allocator_, ptr_); \
		}((allocator), (ptr), (n)))
#else
#	define ham_allocator_new(allocator t) ((t*)ham_allocator_alloc((allocator), alignof(t), sizeof(t)))
#	define ham_allocator_free(allocator, ptr) (ham_allocator_free((allocator), (ptr)))
#endif

//! @cond ignore
ham_api extern const ham_allocator ham_impl_default_allocator;
ham_api extern const ham_allocator *ham_impl_global_allocator;
ham_api extern ham_thread_local const ham_allocator *ham_impl_thread_allocator;
ham_api extern ham_thread_local const ham_allocator *const *ham_impl_current_allocator_ptr;
//! @endcond

ham_nothrow static inline const ham_allocator *ham_current_allocator(){ return *ham_impl_current_allocator_ptr; }

ham_nothrow static inline void ham_set_global_allocator(const ham_allocator *allocator){
	ham_impl_global_allocator = allocator ? allocator : &ham_impl_default_allocator;
}

ham_nothrow static inline void ham_set_current_allocator(const ham_allocator *allocator){
	ham_impl_thread_allocator = allocator;
	ham_impl_current_allocator_ptr = allocator ? &ham_impl_thread_allocator : &ham_impl_global_allocator;
}

static inline void *ham_alloc(ham_usize alignment, ham_usize size){
	return ham_allocator_alloc(ham_current_allocator(), alignment, size);
}

static inline void ham_free(void *mem){
	return ham_allocator_free(ham_current_allocator(), mem);
}

HAM_C_API_END

#ifdef __cplusplus

#include <atomic>
#include <utility>

namespace ham{
	template<typename T>
	class allocator{
		public:
			using pointer = T*;
			using const_pointer = const T*;

			using value_type = T;
			using size_type = usize;

			using difference_type = iptr;

			allocator(const ham_allocator *handle_ = nullptr) noexcept
				: m_handle(handle_ ? handle_ : ham_current_allocator()){}

			template<typename U>
			allocator(const allocator<U> &other) noexcept
				: m_handle(other.m_handle){}

			template<typename U>
			allocator &operator=(const allocator<U> &other) noexcept{
				m_handle = other.m_handle;
				return *this;
			}

			operator const ham_allocator*() const noexcept{ return m_handle; }

			const ham_allocator *handle() const noexcept{ return m_handle; }

			template<typename U>
			bool operator==(const allocator<U> &other) const noexcept{
				return m_handle == other.m_handle;
			}

			template<typename U>
			bool operator!=(const allocator<U> &other) const noexcept{
				return m_handle != other.m_handle;
			}

			template<typename U, typename ... Args>
			U *construct(U *mem, Args &&... args) const noexcept(noexcept(U(std::forward<Args>(args)...))){
				return new(mem) U(std::forward<Args>(args)...);
			}

			template<typename U>
			void destroy(U *ptr) const noexcept{
				ptr->~U();
			}

			template<typename U = T>
			U *allocate(size_type n = 1) const{
				return (U*)ham_allocator_alloc(m_handle, alignof(U), sizeof(U) * n);
			}

			template<typename U = T>
			void deallocate(U *p, usize n = 1) const{
				(void)n;
				ham_allocator_free(m_handle, p);
			}

		private:
			const ham_allocator *m_handle;

			template<typename U>
			friend class allocator;
	};

	class scoped_allocator{
		public:
			scoped_allocator(const ham_allocator *allocator_) noexcept
				: m_old_allocator_ptr(ham_impl_current_allocator_ptr)
				, m_old_allocator(*ham_impl_current_allocator_ptr)
			{
				ham_set_current_allocator(allocator_);
			}

			~scoped_allocator(){
				if(m_old_allocator_ptr == &ham_impl_global_allocator){
					ham_set_current_allocator(nullptr);
				}
				else{
					ham_set_current_allocator(m_old_allocator);
				}
			}

		private:
			const ham_allocator *const *m_old_allocator_ptr;
			const ham_allocator *m_old_allocator;
	};

	template<typename T>
	class owned_ptr{
		public:
			owned_ptr(std::nullptr_t = nullptr) noexcept
				: m_allocator(nullptr)
				, m_ptr(nullptr)
			{}

			template<
				typename U,
				std::enable_if_t<std::is_base_of_v<T, U>, int> = 0
			>
			explicit owned_ptr(U *ptr_, const allocator<T> &allocator_) noexcept
				: m_allocator(allocator_)
				, m_ptr(ptr_)
			{}

			template<
				typename U,
				std::enable_if_t<std::is_base_of_v<T, U>, int> = 0
			>
			owned_ptr(owned_ptr<U> &&other) noexcept
				: m_allocator(other.m_allocator)
				, m_ptr(other.m_ptr.exchange(nullptr))
			{}

			~owned_ptr(){
				const auto old_ptr = m_ptr.exchange(nullptr);
				if(old_ptr){
					m_allocator.destroy(old_ptr);
					m_allocator.deallocate(old_ptr, 1);
				}
			}

			template<
				typename U,
				std::enable_if_t<std::is_base_of_v<T, U>, int> = 0
			>
			owned_ptr &operator=(owned_ptr<U> &&other) noexcept{
				if constexpr(std::is_same_v<T, U>){
					if(this == &other) return *this;
				}

				const auto new_ptr = other.m_ptr.exchange(nullptr);
				const auto old_ptr = m_ptr.exchange(new_ptr);
				if(old_ptr){
					m_allocator.destroy(old_ptr);
					m_allocator.deallocate(old_ptr, 1);
				}

				m_allocator = other.m_allocator;

				return *this;
			}

			template<
				typename U,
				std::enable_if_t<std::is_base_of_v<U, T>, int> = 0
			>
			operator owned_ptr<U>() &&noexcept{ return std::move(*this); }

			T *operator->() noexcept{ return get(); }
			const T *operator->() const noexcept{ return get(); }

			operator bool() const noexcept{ return m_ptr.load(std::memory_order_relaxed) != nullptr; }

			const allocator<T> &get_allocator() const noexcept{ return m_allocator; }

			T *get() noexcept{ return m_ptr.load(std::memory_order_relaxed); }
			const T *get() const noexcept{ return m_ptr.load(std::memory_order_relaxed); }

			T *release() noexcept{ return m_ptr.exchange(nullptr); }

			void destroy() noexcept{
				const auto old_ptr = m_ptr.exchange(nullptr);
				if(old_ptr){
					m_allocator.destroy(old_ptr);
					m_allocator.deallocate(old_ptr, 1);
				}
			}

		private:
			allocator<T> m_allocator;
			std::atomic<T*> m_ptr;

			template<typename U>
			friend class owned_ptr;
	};

	template<typename T, typename ... Args>
	owned_ptr<T> make_owned(Args &&... args){
		const auto allocator = ham_current_allocator();

		const auto mem = ham_allocator_alloc(allocator, alignof(T), sizeof(T));
		if(!mem) return nullptr;

		const auto ptr = new(mem) T(std::forward<Args>(args)...);
		if(!ptr){
			ham_allocator_free(allocator, mem);
			return nullptr;
		}

		return owned_ptr<T>(ptr, allocator);
	}
}

#endif // __cplusplus

/**
 * @}
 */

#endif // !HAM_MEMORY_H
