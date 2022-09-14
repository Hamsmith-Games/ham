#include "ham/memory.h"

#ifdef _WIN32

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

const ham_allocator ham_impl_default_allocator = {
	.alloc = [](usize alignment, usize size, void*){ return ham_aligned_alloc(alignment, size); },
	.free  = [](void *mem, void*){ return ham_aligned_free(mem); },
	.user  = nullptr
};

const ham_allocator *ham_impl_global_allocator = &ham_impl_default_allocator;
ham_thread_local const ham_allocator *ham_impl_thread_allocator = nullptr;

ham_thread_local const ham_allocator *const *ham_impl_current_allocator_ptr = &ham_impl_global_allocator;

HAM_C_API_END
