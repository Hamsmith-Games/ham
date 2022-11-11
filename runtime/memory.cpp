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

#include "ham/memory.h"
#include "ham/log.h"

#ifdef _WIN32

#	define VC_EXTRALEAN 1
#	define WIN32_LEAN_AND_MEAN 1

#	include <sysinfoapi.h>
#	include <malloc.h>

#	define ham_aligned_alloc(alignment, size) (_aligned_malloc((size), (alignment)))
#	define ham_aligned_free(mem) (_aligned_free((mem)))

#else

#	include <sys/mman.h>
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

	return 4096; // eh uh eh

#endif
}

ham_api void *ham_map_pages(ham_usize num_pages){
	if(num_pages == 0) return nullptr;

	const auto page_size = ham_get_page_size();
	if(num_pages > (HAM_USIZE_MAX/page_size)){
		ham_logapierrorf("Allocating %zu pages of %zu bytes would overflow ham_usize", num_pages, page_size);
		return nullptr;
	}

	const auto alloc_size = page_size * num_pages;

#ifdef _WIN32

	const auto mem = VirtualAlloc(nullptr, alloc_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	if(mem == nullptr){
		ham_logapierrorf("Error in VirtualAlloc");
		return nullptr;
	}

	return mem;

#elif defined(__unix__)

	const auto mem = mmap(nullptr, alloc_size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	if(mem == MAP_FAILED){
		ham_logapierrorf("Error in mmap: %s", strerror(errno));
		return nullptr;
	}

	return mem;

#else

	ham_logapierrorf("ham_map_pages unimplemented on this platform");
	return nullptr;

#endif
}

ham_api bool ham_unmap_pages(void *mem, ham_usize num_pages){
	if(!mem || num_pages == 0) return false;

	const auto page_size = ham_get_page_size();
	if(num_pages > (HAM_USIZE_MAX/page_size)){
		ham_logapierrorf("Deallocating %zu pages of %zu bytes would overflow ham_usize", num_pages, page_size);
		return false;
	}

	const auto free_size = page_size * num_pages;

#ifdef _WIN32

	if(!VirtualFree(mem, 0, MEM_RELEASE)){
		ham_logapierrorf("Error in VirtualFree");
		return false;
	}

	ham_logapiwarnf("ALL PAGES OF A MAPPING WILL ALWAYS BE FREED WHEN USING WIN32");

	return true;

#elif defined(__unix__)

	const int res = munmap(mem, free_size);
	if(res != 0){
		ham_logapierrorf("Error in munmap: %s", strerror(errno));
		return false;
	}

	return true;

#else

	ham_logapierrorf("ham_unmap_pages unimplemented on this platform");
	return false;

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
