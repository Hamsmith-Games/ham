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

#ifndef HAM_BUFFER_H
#define HAM_BUFFER_H 1

/**
 * @defgroup HAM_BUFFER Buffers
 * @ingroup HAM
 * @{
 */

#include "typedefs.h"
#include "memory.h"

HAM_C_API_BEGIN

typedef struct ham_buffer ham_buffer;

//! @cond ignore
struct ham_buffer{
	const ham_allocator *allocator;
	void *mem;
	ham_usize alignment, allocated, capacity;
};

ham_used ham_constexpr static inline ham_u32 ham_bit_ceil32(ham_u32 x){ return x == 1 ? 1 : 1 << (32UL - ham_lzcnt32(x - 1UL)); }
ham_used ham_constexpr static inline ham_u64 ham_bit_ceil64(ham_u64 x){ return x == 1 ? 1 : 1 << (64UL - ham_lzcnt64(x - 1UL)); }
//! @endcond

static inline bool ham_buffer_init_allocator(ham_buffer *buf, const ham_allocator *allocator, ham_usize alignment, ham_usize initial_capacity){
	if(!buf || !allocator || alignment == (ham_usize)-1 || initial_capacity == (ham_usize)-1){
		return false;
	}

	if(initial_capacity == 0){
		// TODO: maybe something less arbitrary (using ham_get_page_size?)
		initial_capacity = 64;
	}
	else{
		initial_capacity = ham_popcnt64(initial_capacity) == 1 ? initial_capacity : ham_bit_ceil64(initial_capacity);
	}

	if(alignment == 0){
		alignment = alignof(max_align_t);
	}
	else{
		alignment = ham_popcnt64(alignment) == 1 ? alignment : ham_bit_ceil64(alignment);
	}

	void *const mem = ham_allocator_alloc(allocator, alignment, initial_capacity);
	if(ham_unlikely(!mem)) return false;

	buf->allocator = allocator;
	buf->mem = mem;
	buf->alignment = alignment;
	buf->allocated = 0;
	buf->capacity  = initial_capacity;
	return true;
}

ham_used
static inline bool ham_buffer_init(ham_buffer *buf, ham_usize alignment, ham_usize initial_capacity){
	return ham_buffer_init_allocator(buf, ham_current_allocator(), alignment, initial_capacity);
}

ham_used
ham_nothrow static inline void ham_buffer_finish(ham_buffer *buf){
	if(ham_unlikely(!buf) || ham_unlikely(!buf->allocator)) return;

	if(buf->mem) ham_allocator_free(buf->allocator, buf->mem);

	buf->allocator = ham_null;
	buf->mem       = ham_null;
	buf->alignment = 0;
	buf->allocated = 0;
	buf->capacity  = 0;
}

ham_nonnull_args(1) ham_used
ham_nothrow static inline const ham_allocator *ham_buffer_allocator(const ham_buffer *buf){
	return buf->allocator;
}

static inline bool ham_buffer_reserve(ham_buffer *buf, ham_usize req_size){
	if(!buf || !req_size || req_size == (ham_usize)-1){
		return false;
	}

	if(req_size <= buf->capacity){
		return true;
	}

	if(ham_popcnt64(req_size) != 1){
		req_size = ham_bit_ceil64(req_size);
	}

	void *const new_mem = ham_allocator_alloc(buf->allocator, buf->alignment, req_size);
	if(!new_mem){
		// TODO: signal error
		return false;
	}

	if(buf->allocated){
		memcpy(new_mem, buf->mem, buf->allocated);
	}

	ham_allocator_free(buf->allocator, buf->mem);

	buf->mem = new_mem;
	return true;
}

ham_used
static inline bool ham_buffer_resize(ham_buffer *buf, ham_usize req_size){
	if(!buf) return false;

	if(req_size > buf->capacity && !ham_buffer_reserve(buf, req_size)){
		// TODO: signal error
		return false;
	}

	buf->allocated = req_size;
	return true;
}

ham_used
static inline ham_usize ham_buffer_size(const ham_buffer *buf){ return buf ? buf->allocated : 0; }

ham_used
static inline void *ham_buffer_data(ham_buffer *buf){ return buf ? buf->mem : ham_null; }

ham_used
static inline bool ham_buffer_erase(ham_buffer *buf, ham_usize from, ham_usize len){
	if(!buf || !buf->mem || from >= buf->allocated) return false;

	const ham_usize actual_len = ham_min(len, buf->allocated - from);
	const ham_usize after_len  = buf->allocated - (from + actual_len);

	char *const it = (char*)buf->mem + from;

	char *const after_beg = it + actual_len;
	char *const after_end = after_beg + after_len;

	if(after_beg != after_end){
		memmove(it, after_beg, after_len);
	}

	buf->allocated -= actual_len;
	return true;
}

ham_used
static inline bool ham_buffer_insert(ham_buffer *buf, ham_usize offset, const void *data, ham_usize len){
	if(!buf || !buf->mem) return false;

	offset = ham_min(offset, buf->allocated);

	if(!ham_buffer_resize(buf, buf->allocated + len)) return false;

	const ham_usize rem = buf->allocated - (offset + len);

	char *const it = (char*)buf->mem + offset;
	char *const it_end = it + len;

	if(rem > 0){
		memcpy(it_end, it, rem);
	}

	memcpy(it, data, len);

	return true;
}

#define ham_buffer_foreach(buf, t, it) \
	for(t *it = (t*)ham_buffer_data(buf); it < ((t*)ham_buffer_data(buf) + (ham_buffer_size(buf)/sizeof(t))); ++it)

HAM_C_API_END

#ifdef __cplusplus

#include "str_buffer.h"

namespace ham{
	class basic_buffer_exception: public exception{};

	class basic_buffer_ctor_error: public basic_buffer_exception{
		public:
			const char *api() const noexcept override{ return "ham::basic_buffer::basic_buffer"; }
			const char *what() const noexcept override{ return "Error in ham_buffer_init"; }
	};

	class basic_buffer_emplace_error: public basic_buffer_exception{
		public:
			const char *api() const noexcept override{ return "ham::basic_buffer::emplace_back"; }
			const char *what() const noexcept override{ return "Error in ham_buffer_resize"; }
	};

	class basic_buffer_at_error: public basic_buffer_exception{
		public:
			basic_buffer_at_error(usize idx)
				: m_msg(format("No element at index {}", idx)){}

			const char *api() const noexcept override{ return "ham::basic_buffer::at"; }
			const char *what() const noexcept override{ return m_msg.c_str(); }

		private:
			str_buffer_utf8 m_msg;
	};

	class basic_buffer_copy_error: public basic_buffer_exception{
		public:
			const char *api() const noexcept override{ return "ham::basic_buffer"; }
			const char *what() const noexcept override{ return "Error in ham_buffer_resize"; }
	};

	template<typename T>
		requires std::is_same_v<T, void> || (std::is_trivially_copyable_v<T> && std::is_trivially_destructible_v<T>)
	class basic_buffer{
		public:
			static_assert(std::is_same_v<T, std::remove_cvref_t<T>>);

			using value_type = std::conditional_t<std::is_same_v<T, void>, char, T>;

			using pointer       = value_type*;
			using const_pointer = const value_type*;

			using reference       = value_type&;
			using const_reference = const value_type&;

			static constexpr usize value_alignment = std::conditional_t<std::is_same_v<T, void>, meta::constant_usize<alignof(void*)>, meta::constant_usize<alignof(T)>>::value;
			static constexpr usize value_size      = std::conditional_t<std::is_same_v<T, void>, meta::constant_usize<1>,              meta::constant_usize<sizeof(T)>>::value;

			template<
				bool HasDefaultCtor = !std::is_same_v<T, void>,
				std::enable_if_t<HasDefaultCtor, int> = 0
			>
			explicit basic_buffer(usize initial_size = 0){
				const auto initial_byte_size = initial_size * value_size;

				if(!ham_buffer_init(&m_buf, value_alignment, initial_byte_size)){
					throw basic_buffer_ctor_error();
				}

				ham_buffer_resize(&m_buf, initial_byte_size);
				std::uninitialized_default_construct_n((pointer)m_buf.mem, size());
			}

			template<
				bool HasSizedCtor = std::is_same_v<T, void>,
				std::enable_if_t<HasSizedCtor, int> = 0
			>
			explicit basic_buffer(usize alignment, usize initial_size){
				if(!ham_buffer_init(&m_buf, alignment, initial_size)){
					throw basic_buffer_ctor_error();
				}

				ham_buffer_resize(&m_buf, initial_size);
			}

			basic_buffer(const std::initializer_list<value_type> &values){
				if(!ham_buffer_init(&m_buf, value_alignment, values.size() * value_size)){
					throw basic_buffer_ctor_error();
				}

				if(!ham_buffer_resize(&m_buf, values.size() * value_size)){
					throw basic_buffer_copy_error();
				}

				std::uninitialized_copy_n(values.begin(), values.size(), (pointer)m_buf.mem);
			}

			basic_buffer(basic_buffer &&other) noexcept
				: m_buf(other.m_buf)
			{
				memset(&other.m_buf, 0, sizeof(ham_buffer));
			}

			basic_buffer(const basic_buffer &other){
				if(!ham_buffer_init(&m_buf, value_alignment, other.m_buf.allocated)){
					throw basic_buffer_ctor_error();
				}

				if(!ham_buffer_resize(&m_buf, other.m_buf.allocated)){
					throw basic_buffer_copy_error();
				}

				std::uninitialized_copy_n((const_pointer)other.m_buf.mem, other.size(), (pointer)m_buf.mem);
			}

			~basic_buffer(){
				if(m_buf.mem){
					const auto len = size();
					const auto data_ = data();
					std::destroy_n(data_, len);
					ham_buffer_finish(&m_buf);
				}
			}

			basic_buffer &operator=(basic_buffer &&other) noexcept{
				if(this != &other){
					const auto len = size();
					const auto data_ = data();
					std::destroy_n(data_, len);

					ham_buffer_finish(&m_buf);

					memcpy(&m_buf, &other.m_buf, sizeof(ham_buffer));
					memset(&other.m_buf, 0, sizeof(ham_buffer));
				}

				return *this;
			}

			basic_buffer &operator=(const basic_buffer &other){
				if(this != &other){
					const auto len = size();
					const auto data_ = data();
					std::destroy_n(data_, len);

					if(!ham_buffer_resize(&m_buf, other.m_buf.allocated)){
						throw basic_buffer_copy_error();
					}

					std::uninitialized_copy_n((const_pointer)other.m_buf.mem, other.size(), (pointer)m_buf.mem);
				}

				return *this;
			}

			bool empty() const noexcept{ return m_buf.allocated == 0; }

			pointer data() noexcept{ return (pointer)m_buf.mem; }
			const_pointer data() const noexcept{ return (const_pointer)m_buf.mem; }

			usize size() const noexcept{
				return m_buf.allocated / value_size;
			}

			reference operator[](usize idx) noexcept{ return data()[idx]; }
			const_reference operator[](usize idx) const noexcept{ return data()[idx]; }

			const ham_allocator *allocator() const noexcept{ return ham_buffer_allocator(&m_buf); }

			bool resize(usize new_size) noexcept(noexcept(value_type())){
				if(new_size == size()){
					return true;
				}
				else if(new_size < size()){
					std::destroy_n(data() + new_size, size() - new_size);
					return ham_buffer_resize(&m_buf, new_size * value_size);
				}
				else{
					const auto old_size = size();
					if(!ham_buffer_resize(&m_buf, new_size * value_size)){
						return false;
					}

					std::uninitialized_default_construct_n(data() + old_size, size() - old_size);
					return true;
				}
			}

			reference at(usize idx){
				if(size() <= idx) throw basic_buffer_at_error(idx);
				return data()[idx];
			}

			const_reference at(usize idx) const{
				if(size() <= idx) throw basic_buffer_at_error(idx);
				return data()[idx];
			}

			reference front(){ return at(0); }
			const_reference front() const{ return at(0); }

			reference back(){ return at(size()-1); }
			const_reference back() const{ return at(size()-1); }

			template<
				bool InsertEnabled = !std::is_same_v<T, void>,
				std::enable_if_t<InsertEnabled, int> = 0
			>
			pointer insert(usize idx, const_reference value){
				if(!ham_buffer_insert(&m_buf, idx * value_size, &value, value_size)){
					return nullptr;
				}
				else{
					return data() + idx;
				}
			}

			bool erase(usize idx) noexcept{
				if(idx >= size()) return false;
				else return ham_buffer_erase(&m_buf, idx * value_size, value_size);
			}

			template<typename ... Args>
			auto emplace_back(Args &&... args) -> std::enable_if_t<!std::is_same_v<T, void>, reference>{
				const auto new_idx = size();

				if(!ham_buffer_resize(&m_buf, m_buf.allocated + value_size)){
					throw basic_buffer_emplace_error();
				}

				const auto mem = data() + new_idx;
				return *(new(mem) T(std::forward<Args>(args)...));
			}

			pointer begin() noexcept{ return data(); }
			pointer end()   noexcept{ return data() + size(); }

			const_pointer begin() const noexcept{ return data(); }
			const_pointer end()   const noexcept{ return data() + size(); }

		private:
			ham_buffer m_buf;
	};
}

#endif // __cplusplus

/**
 * @}
 */

#endif // HAM_BUFFER_H 1
