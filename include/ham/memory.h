#ifndef HAM_MEMORY_H
#define HAM_MEMORY_H 1

/**
 * @defgroup HAM_MEMORY Memory management
 * @ingroup HAM
 * @{
 */

#include "typedefs.h"

HAM_C_API_BEGIN

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

//! @cond ignore
ham_api extern const ham_allocator ham_impl_default_allocator;
ham_api extern const ham_allocator *ham_impl_global_allocator;
ham_api extern ham_thread_local const ham_allocator *ham_impl_thread_allocator;
ham_api extern ham_thread_local const ham_allocator *const *ham_impl_current_allocator_ptr;
//! @endcond

static inline const ham_allocator *ham_current_allocator(){ return *ham_impl_current_allocator_ptr; }

static inline void ham_set_global_allocator(const ham_allocator *allocator){
	ham_impl_global_allocator = allocator ? allocator : &ham_impl_default_allocator;
}

static inline void ham_set_current_allocator(const ham_allocator *allocator){
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

			pointer allocate(size_type n = 1) const{
				return (pointer)ham_allocator_alloc(m_handle, alignof(T), sizeof(T) * n);
			}

			void deallocate(pointer p, usize n = 1) const{
				(void)n;
				ham_allocator_free(m_handle, p);
			}

		private:
			const ham_allocator *m_handle;

			template<typename U>
			friend class allocator;
	};

	template<typename T>
	class owned_ptr{
		public:
			explicit owned_ptr(T *ptr_, const allocator<T> &allocator_) noexcept
				: m_allocator(allocator_)
				, m_ptr(ptr_)
			{}

			owned_ptr(owned_ptr &&other) noexcept
				: m_allocator(other.m_allocator)
				, m_ptr(other.m_ptr.exchange(nullptr))
			{}

			owned_ptr &operator=(owned_ptr &&other){
				if(this != &other){
					const auto new_ptr = other.m_ptr.exchange(nullptr);
					const auto old_ptr = m_ptr.exchange(new_ptr);
					if(old_ptr){
						m_allocator.destroy(old_ptr);
						m_allocator.deallocate(old_ptr, 1);
					}

					m_allocator = other.m_allocator;
				}
			}

			T *operator->() noexcept{ return get(); }
			const T *operator->() const noexcept{ return get(); }

			const allocator<T> &get_allocator() const noexcept{ return m_allocator; }

			T *get() noexcept{ return m_ptr.load(std::memory_order_relaxed); }
			const T *get() const noexcept{ return m_ptr.load(std::memory_order_relaxed); }

			T *release() noexcept{ return m_ptr.exchange(nullptr); }

		private:
			allocator<T> m_allocator;
			std::atomic<T*> m_ptr;
	};
}

#endif // __cplusplus

/**
 * @}
 */

#endif // !HAM_MEMORY_H
