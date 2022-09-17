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
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef HAM_MATH_H
#define HAM_MATH_H 1

/**
 * @defgroup HAM_MATH Math
 * @ingroup HAM
 * #{
 */

#include "typedefs.h"

#include "gmp.h"
#include "mpfr.h"

#include <string.h>

HAM_C_API_BEGIN

/**
 * @defgroup HAM_MATH_AINT Arbitrary precision integers
 * @{
 */

typedef struct ham_aint{
	mpz_t mpz;
} ham_aint;

ham_nothrow static inline void ham_aint_init(ham_aint *aint){ mpz_init(aint->mpz); }
ham_nothrow static inline void ham_aint_finish(ham_aint *aint){ mpz_clear(aint->mpz); }

ham_nothrow static inline void ham_aint_init_iptr(ham_aint *aint, ham_iptr val){ mpz_init_set_si(aint->mpz, val); }
ham_nothrow static inline void ham_aint_init_uptr(ham_aint *aint, ham_uptr val){ mpz_init_set_ui(aint->mpz, val); }

ham_nothrow static inline bool ham_aint_init_str_utf8(ham_aint *aint, ham_str8 str, ham_u16 base){
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

ham_nothrow static inline bool ham_aint_init_str_utf16(ham_aint *aint, ham_str16 str, ham_u16 base){
	ham_name_buffer_utf8 buf;
	if(ham_str_conv_utf16_utf8(str, buf, sizeof(buf)) == (ham_usize)-1){
		return false;
	}

	const int res = mpz_init_set_str(aint->mpz, buf, base);
	if(res != 0){
		(void)res;
		// TODO: signal error
	}

	return true;
}

ham_nothrow static inline bool ham_aint_init_str_utf32(ham_aint *aint, ham_str32 str, ham_u16 base){
	ham_name_buffer_utf8 buf;
	if(ham_str_conv_utf32_utf8(str, buf, sizeof(buf)) == (ham_usize)-1){
		return false;
	}

	const int res = mpz_init_set_str(aint->mpz, buf, base);
	if(res != 0){
		mpz_clear(aint->mpz);
		return false;
	}
	else{
		return true;
	}
}

#define HAM_AINT_INIT_STR_UTF(n) HAM_CONCAT(ham_aint_init_str_utf, n)

/**
 * @}
 */

/**
 * @defgroup HAM_MATH_ARAT Arbitrary precision rationals
 * @{
 */

typedef struct ham_arat{
	mpq_t mpq;
} ham_arat;

ham_nothrow static inline void ham_arat_init(ham_arat *arat){ mpq_init(arat->mpq); }
ham_nothrow static inline void ham_arat_clear(ham_arat *arat){ mpq_clear(arat->mpq); }

ham_nothrow static inline void ham_arat_init_aint(ham_arat *arat, const ham_aint *numerator, const ham_aint *denominator){
	mpq_init(arat->mpq);
	mpz_set(mpq_numref(arat->mpq), numerator->mpz);
	mpz_set(mpq_denref(arat->mpq), denominator->mpz);
	mpq_canonicalize(arat->mpq);
}

ham_nothrow static inline void ham_arat_init_iptr(ham_arat *arat, ham_iptr numerator, ham_uptr denominator){
	mpq_init(arat->mpq);
	mpz_set_si(mpq_numref(arat->mpq), numerator);
	mpz_set_ui(mpq_denref(arat->mpq), denominator);
	mpq_canonicalize(arat->mpq);
}

ham_nothrow static inline void ham_arat_init_uptr(ham_arat *arat, ham_uptr numerator, ham_uptr denominator){
	mpq_init(arat->mpq);
	mpz_set_ui(mpq_numref(arat->mpq), numerator);
	mpz_set_ui(mpq_denref(arat->mpq), denominator);
	mpq_canonicalize(arat->mpq);
}

/**
 *
 */

/**
 * @defgroup HAM_MATH_AREAL Arbitrary precision reals
 * @{
 */

typedef struct ham_areal{
	mpfr_t mpfr;
} ham_areal;

ham_nothrow static inline void ham_areal_init(ham_areal *areal){ mpfr_init(areal->mpfr); }
ham_nothrow static inline void ham_areal_finish(ham_areal *areal){ mpfr_clear(areal->mpfr); }

ham_nothrow static inline void ham_areal_init_arat(ham_areal *areal, const ham_arat *arat){ mpfr_init_set_q(areal->mpfr, arat->mpq, MPFR_RNDN); }
ham_nothrow static inline void ham_areal_init_aint(ham_areal *areal, const ham_aint *aint){ mpfr_init_set_z(areal->mpfr, aint->mpz, MPFR_RNDN); }
ham_nothrow static inline void ham_areal_init_iptr(ham_areal *areal, ham_iptr val){ mpfr_init_set_si(areal->mpfr, val, MPFR_RNDN); }
ham_nothrow static inline void ham_areal_init_uptr(ham_areal *areal, ham_uptr val){ mpfr_init_set_ui(areal->mpfr, val, MPFR_RNDN); }
ham_nothrow static inline void ham_areal_init_f32 (ham_areal *areal, ham_f32 val){ mpfr_init_set_d(areal->mpfr, val, MPFR_RNDN); }
ham_nothrow static inline void ham_areal_init_f64 (ham_areal *areal, ham_f64 val){ mpfr_init_set_d(areal->mpfr, val, MPFR_RNDN); }

ham_nothrow static inline bool ham_areal_init_str_utf8(ham_areal *areal, ham_str8 str, ham_u16 base){
	if(str.len >= HAM_NAME_BUFFER_SIZE) return false;

	ham_name_buffer_utf8 buf;
	memcpy(buf, str.ptr, str.len);
	buf[str.len] = '\0';

	const int res = mpfr_init_set_str(areal->mpfr, buf, base, MPFR_RNDN);
	if(res != 0){
		mpfr_clear(areal->mpfr);
		return false;
	}
	else{
		return true;
	}
}

ham_nothrow static inline bool ham_areal_init_str_utf16(ham_areal *areal, ham_str16 str, ham_u16 base){
	if(str.len >= HAM_NAME_BUFFER_SIZE) return false;

	ham_name_buffer_utf8 buf;
	if(ham_str_conv_utf16_utf8(str, buf, sizeof(buf)) == (ham_usize)-1){
		return false;
	}

	const int res = mpfr_init_set_str(areal->mpfr, buf, base, MPFR_RNDN);
	if(res != 0){
		mpfr_clear(areal->mpfr);
		return false;
	}
	else{
		return true;
	}
}

ham_nothrow static inline bool ham_areal_init_str_utf32(ham_areal *areal, ham_str32 str, ham_u16 base){
	if(str.len >= HAM_NAME_BUFFER_SIZE) return false;

	ham_name_buffer_utf8 buf;
	if(ham_str_conv_utf32_utf8(str, buf, sizeof(buf)) == (ham_usize)-1){
		return false;
	}

	const int res = mpfr_init_set_str(areal->mpfr, buf, base, MPFR_RNDN);
	if(res != 0){
		mpfr_clear(areal->mpfr);
		return false;
	}
	else{
		return true;
	}
}

/**
 * @}
 */

HAM_C_API_END

#ifdef __cplusplus

namespace ham{
	namespace detail{
		template<typename Char>
		constexpr inline auto aint_ctype_init_str = utf_conditional_t<
			Char,
			static_fn<ham_aint_init_str_utf8>,
			static_fn<ham_aint_init_str_utf16>,
			static_fn<ham_aint_init_str_utf32>
		>{};

		template<typename Char>
		constexpr inline auto areal_ctype_init_str = utf_conditional_t<
			Char,
			static_fn<ham_areal_init_str_utf8>,
			static_fn<ham_areal_init_str_utf16>,
			static_fn<ham_areal_init_str_utf32>
		>{};
	}
}

#endif // __cplusplus

/**
 * @}
 */

#endif // !HAM_MATH_H
