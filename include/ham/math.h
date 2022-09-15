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

#include <string.h>
#include <uchar.h>

HAM_C_API_BEGIN

/**
 * @defgroup HAM_MATH_AINT Arbitrary precision integers
 * @{
 */

typedef struct ham_aint{
	mpz_t mpz;
} ham_aint;

void ham_aint_init(ham_aint *aint){ mpz_init(aint->mpz); }
void ham_aint_finish(ham_aint *aint){ mpz_clear(aint->mpz); }

void ham_aint_init_iptr(ham_aint *aint, ham_iptr val){ mpz_init_set_si(aint->mpz, val); }
void ham_aint_init_uptr(ham_aint *aint, ham_uptr val){ mpz_init_set_ui(aint->mpz, val); }

bool ham_aint_init_str_utf8(ham_aint *aint, ham_str8 str, ham_u16 base){
	if(str.len > HAM_NAME_BUFFER_SIZE) return false;

	ham_name_buffer_utf8 buf;
	memcpy(buf, str.ptr, str.len);
	buf[str.len] = '\0';

	const int res = mpz_init_set_str(aint->mpz, buf, base);
	if(res != 0){
		(void)res;
		// TODO: signal error
	}

	return true;
}

bool ham_aint_init_str_utf16(ham_aint *aint, ham_str16 str, ham_u16 base){
	ham_name_buffer_utf8 buf;

	auto out_ptr = buf;

	mbstate_t mbstate;
	for(ham_usize i = 0; i < str.len; i++){
		const ham_usize res = c16rtomb(out_ptr, str.ptr[i], &mbstate);
		if(res == -1){
			return false;
		}
		else if(res > 0){
			out_ptr += res;
		}
	}

	*out_ptr = '\0';

	const int res = mpz_init_set_str(aint->mpz, buf, base);
	if(res != 0){
		(void)res;
		// TODO: signal error
	}

	return true;
}

bool ham_aint_init_str_utf32(ham_aint *aint, ham_str32 str, ham_u16 base){
	ham_name_buffer_utf8 buf;

	auto out_ptr = buf;

	mbstate_t mbstate;
	for(ham_usize i = 0; i < str.len; i++){
		const ham_usize res = c32rtomb(out_ptr, str.ptr[i], &mbstate);
		if(res == -1){
			return false;
		}
		else if(res > 0){
			out_ptr += res;
		}
	}

	*out_ptr = '\0';

	const int res = mpz_init_set_str(aint->mpz, buf, base);
	if(res != 0){
		(void)res;
		// TODO: signal error
	}

	return true;
}

#define HAM_AINT_INIT_STR_UTF(n) HAM_CONCAT(ham_aint_init_str_utf, n)

/**
 * @}
 */

/**
 * @defgroup HAM_MATH_AREAL Arbitrary precision reals
 * @{
 */

typedef struct ham_areal{
	mpfr_t mpfr;
} ham_areal;

void ham_areal_init(ham_areal *areal){ mpfr_init(areal->mpfr); }
void ham_areal_finish(ham_areal *areal){ mpfr_clear(areal->mpfr); }

/**
 * @}
 */

HAM_C_API_END

/**
 * @}
 */

#endif // !HAM_MATH_H
