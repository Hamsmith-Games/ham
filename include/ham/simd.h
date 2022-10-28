#ifndef HAM_SIMD_H
#define HAM_SIMD_H 1

/**
 * @defgroup HAM_SIMD SIMD
 * @ingroup HAM
 * @{
 */

#include "typedefs.h"

HAM_C_API_BEGIN

#ifdef __GNUC__

#	define HAM_SIMD 1

#	if defined(__x86_64__)
#		include <xmmintrin.h>
#	elif defined(__aarch64__) || defined(__neon__)
#		include <arm_neon.h>

		typedef float32x2_t ham_v2f32;
#	endif

	// 128-bit SIMD types

	typedef ham_u32 ham_v4u32 __attribute__((vector_size(16)));
	typedef ham_i32 ham_v4i32 __attribute__((vector_size(16)));
	typedef ham_f32 ham_v4f32 __attribute__((vector_size(16)));

	typedef ham_u64 ham_v2u64 __attribute__((vector_size(16)));
	typedef ham_i64 ham_v2i64 __attribute__((vector_size(16)));
	typedef ham_f64 ham_v2f64 __attribute__((vector_size(16)));

	// 256-bit SIMD types

	typedef ham_u32 ham_v8u32 __attribute__((vector_size(32)));
	typedef ham_i32 ham_v8i32 __attribute__((vector_size(32)));
	typedef ham_f32 ham_v8f32 __attribute__((vector_size(32)));

	typedef ham_u64 ham_v4u64 __attribute__((vector_size(32)));
	typedef ham_i64 ham_v4i64 __attribute__((vector_size(32)));
	typedef ham_f64 ham_v4f64 __attribute__((vector_size(32)));

#	define ham_simd_shuffle(vec0, vec2, ...) \
		__builtin_shufflevector(vec0, vec2 __VA_OPT__(,) __VA_ARGS__)

#else
#	warning "SIMD currently only supported with GNU C vector extensions"
#endif

HAM_C_API_END

/**
 * @}
 */

#endif // !HAM_SIMD_H
