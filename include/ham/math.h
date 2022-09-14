#ifndef HAM_MATH_H
#define HAM_MATH_H 1

/**
 * @defgroup HAM_MATH Math
 * @ingroup HAM
 * #{
 */

#include "config.h"

#include "gmp.h"
#include "mpfr.h"

HAM_C_API_BEGIN

typedef struct ham_aint{
	mpz_t mpz;
} ham_aint;

typedef struct ham_areal{
	mpfr_t mpfr;
} ham_areal;

HAM_C_API_END

/**
 * @}
 */

#endif // !HAM_MATH_H
