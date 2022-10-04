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

ham_constexpr static inline ham_u32 ham_bit_ceil32(ham_u32 x){ return x == 1 ? 1 : 1 << (32UL - ham_lzcnt32(x - 1UL)); }
ham_constexpr static inline ham_u64 ham_bit_ceil64(ham_u64 x){ return x == 1 ? 1 : 1 << (64UL - ham_lzcnt64(x - 1UL)); }
//! @endcond

static inline bool ham_buffer_init(ham_buffer *buf, ham_usize alignment, ham_usize initial_capacity){
	if(!buf || alignment == (ham_usize)-1 || initial_capacity == (ham_usize)-1){
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

	const ham_allocator *cur_allocator = ham_current_allocator();

	void *const mem = ham_allocator_alloc(cur_allocator, alignment, initial_capacity);
	if(ham_unlikely(!mem)) return false;

	buf->allocator = cur_allocator;
	buf->mem = mem;
	buf->alignment = alignment;
	buf->allocated = 0;
	buf->capacity  = initial_capacity;
	return true;
}

static inline void ham_buffer_finish(ham_buffer *buf){
	if(ham_unlikely(!buf)) return;

	if(buf->mem) ham_allocator_free(buf->allocator, buf->mem);

	buf->allocator = ham_null;
	buf->mem       = ham_null;
	buf->alignment = 0;
	buf->allocated = 0;
	buf->capacity  = 0;
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

static inline bool ham_buffer_resize(ham_buffer *buf, ham_usize req_size){
	if(!buf) return false;

	if(req_size > buf->capacity && !ham_buffer_reserve(buf, req_size)){
		// TODO: signal error
		return false;
	}

	buf->allocated = req_size;
	return true;
}

static inline ham_usize ham_buffer_size(const ham_buffer *buf){ return buf ? buf->allocated : 0; }

static inline void *ham_buffer_data(ham_buffer *buf){ return buf ? buf->mem : ham_null; }

HAM_C_API_END

/**
 * @}
 */

#endif // HAM_BUFFER_H 1
