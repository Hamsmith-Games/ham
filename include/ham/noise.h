#ifndef HAM_NOISE_H
#define HAM_NOISE_H 1

/**
 * @defgroup HAM_NOISE Noise
 * @ingroup HAM
 * @{
 */

#include "typedefs.h"

HAM_C_API_BEGIN

/**
 * @defgroup HAM_RANDOM Random numbers
 * @{
 */

typedef struct ham_random32_state{
	ham_u64 state, inc;
} ham_random32_state;

ham_nonnull_args(1)
ham_nothrow ham_constexpr static inline void ham_srand_step(ham_random32_state *state){
	state->state = state->state * 6364136223846793005ULL + state->inc;
}

ham_nonnull_args(1)
ham_nothrow ham_constexpr static inline void ham_srand(ham_random32_state *state, ham_u64 init_state, ham_u64 init_seq){
	state->state = 0;
	state->inc = (init_seq << 1u) | 1u;

	ham_srand_step(state);
	state->state += init_state;
	ham_srand_step(state);
}

ham_nonnull_args(1)
ham_nothrow ham_constexpr static inline ham_u32 ham_rand(ham_random32_state *state){
	const ham_u64 old_state = state->state;

	state->state = old_state * 6364136223846793005ULL + (state->inc | 1u);

	const ham_u32 shifted = ((old_state >> 18u) ^ old_state) >> 27u;
	const ham_u32 rot = old_state >> 59u;

	return (shifted >> rot) | (shifted << ((-rot) & 31));
}

ham_nonnull_args(1)
ham_nothrow ham_constexpr static inline ham_f64 ham_randf(ham_random32_state *state){
	ham_constexpr const ham_f64 coef = 1.0 / HAM_U32_MAX;
	return coef * (ham_f64)ham_rand(state);
}

/**
 * @}
 */

HAM_C_API_END

/**
 * @}
 */

#endif // !HAM_NOISE_H
