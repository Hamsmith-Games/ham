/**
 * The Ham Programming Language
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

#include "ham/memory.h"

#ifdef _WIN32

#	define VC_EXTRALEAN 1
#	define WIN32_LEAN_AND_MEAN 1

#	include <sysinfoapi.h>
#	include <malloc.h>

#	define ham_aligned_alloc(alignment, size) (_aligned_malloc((size), (alignment)))
#	define ham_aligned_free(mem) (_aligned_free((mem)))

#else

#	include <stdlib.h>

#	define ham_aligned_alloc(alignment, size) (aligned_alloc((alignment), (size)))
#	define ham_aligned_free(mem) (free((mem)))

#endif

using namespace ham::typedefs;

HAM_C_API_BEGIN

ham_usize ham_get_page_size(){
#ifdef _WIN32
	SYSTEM_INFO info;
	GetSystemInfo(&info);
	return (ham_usize)info.dwPageSize;
#elif defined(__unix__)
	long res = sysconf(_SC_PAGESIZE);
	if(res <= 0){
		// wtf?
		return 4096; // safe bet
	}
	return (ham_usize)res;
#else
	return 4096;
#endif
}

const ham_allocator ham_impl_default_allocator = {
	.alloc = [](usize alignment, usize size, void*){ return ham_aligned_alloc(alignment, size); },
	.free  = [](void *mem, void*){ return ham_aligned_free(mem); },
	.user  = nullptr
};

const ham_allocator *ham_impl_global_allocator = &ham_impl_default_allocator;
ham_thread_local const ham_allocator *ham_impl_thread_allocator = nullptr;

ham_thread_local const ham_allocator *const *ham_impl_current_allocator_ptr = &ham_impl_global_allocator;

HAM_C_API_END
