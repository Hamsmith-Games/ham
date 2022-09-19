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

//
// Initialization and finalization
//

ham_nonnull_args(1)
ham_nothrow static inline void ham_aint_init(ham_aint *aint){ mpz_init(aint->mpz); }

ham_nonnull_args(1)
ham_nothrow static inline void ham_aint_finish(ham_aint *aint){ mpz_clear(aint->mpz); }

//
// Simultaneous init and set
//

ham_nonnull_args(1, 2)
ham_nothrow static inline void ham_aint_init_set(ham_aint *aint, const ham_aint *other){ mpz_init_set(aint->mpz, other->mpz); }

ham_nonnull_args(1)
ham_nothrow static inline void ham_aint_init_iptr(ham_aint *aint, ham_iptr val){ mpz_init_set_si(aint->mpz, val); }

ham_nonnull_args(1)
ham_nothrow static inline void ham_aint_init_uptr(ham_aint *aint, ham_uptr val){ mpz_init_set_ui(aint->mpz, val); }

ham_nonnull_args(1)
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

ham_nonnull_args(1)
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

ham_nonnull_args(1)
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

//
// Assignment
//

ham_nonnull_args(1, 2)
ham_nothrow static inline void ham_aint_set(ham_aint *ret, const ham_aint *other){ mpz_set(ret->mpz, other->mpz); }

ham_nonnull_args(1)
ham_nothrow static inline void ham_aint_set_iptr(ham_aint *ret, ham_iptr val){ mpz_set_si(ret->mpz, val); }

ham_nonnull_args(1)
ham_nothrow static inline void ham_aint_set_uptr(ham_aint *ret, ham_uptr val){ mpz_set_ui(ret->mpz, val); }

//
// Arithmetic
//

ham_nonnull_args(1, 2, 3)
ham_nothrow static inline void ham_aint_add(ham_aint *ret, const ham_aint *lhs, const ham_aint *rhs){ mpz_add(ret->mpz, lhs->mpz, rhs->mpz); }

ham_nonnull_args(1, 2, 3)
ham_nothrow static inline void ham_aint_sub(ham_aint *ret, const ham_aint *lhs, const ham_aint *rhs){ mpz_sub(ret->mpz, lhs->mpz, rhs->mpz); }

ham_nonnull_args(1, 2, 3)
ham_nothrow static inline void ham_aint_mul(ham_aint *ret, const ham_aint *lhs, const ham_aint *rhs){ mpz_mul(ret->mpz, lhs->mpz, rhs->mpz); }

ham_nonnull_args(1, 2, 3)
ham_nothrow static inline void ham_aint_div(ham_aint *ret, const ham_aint *lhs, const ham_aint *rhs){ mpz_div(ret->mpz, lhs->mpz, rhs->mpz); }

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

//
// Initialization
//

ham_nonnull_args(1)
ham_nothrow static inline void ham_arat_init(ham_arat *arat){ mpq_init(arat->mpq); }

ham_nonnull_args(1)
ham_nothrow static inline void ham_arat_finish(ham_arat *arat){ mpq_clear(arat->mpq); }

//
// Simultaneous init and set
//

ham_nonnull_args(1, 2)
ham_nothrow static inline void ham_arat_init_set(ham_arat *arat, const ham_arat *other){
	mpq_init(arat->mpq);
	mpq_set(arat->mpq, other->mpq);
}

ham_nonnull_args(1, 2, 3)
ham_nothrow static inline void ham_arat_init_aint(ham_arat *arat, const ham_aint *numerator, const ham_aint *denominator){
	mpq_init(arat->mpq);
	mpz_set(mpq_numref(arat->mpq), numerator->mpz);
	mpz_set(mpq_denref(arat->mpq), denominator->mpz);
	mpq_canonicalize(arat->mpq);
}

ham_nonnull_args(1)
ham_nothrow static inline void ham_arat_init_iptr(ham_arat *arat, ham_iptr numerator, ham_uptr denominator){
	mpq_init(arat->mpq);
	mpz_set_si(mpq_numref(arat->mpq), numerator);
	mpz_set_ui(mpq_denref(arat->mpq), denominator);
	mpq_canonicalize(arat->mpq);
}

ham_nonnull_args(1)
ham_nothrow static inline void ham_arat_init_uptr(ham_arat *arat, ham_uptr numerator, ham_uptr denominator){
	mpq_init(arat->mpq);
	mpz_set_ui(mpq_numref(arat->mpq), numerator);
	mpz_set_ui(mpq_denref(arat->mpq), denominator);
	mpq_canonicalize(arat->mpq);
}

//
// Assignment
//

ham_nonnull_args(1, 2)
ham_nothrow static inline void ham_arat_set(ham_arat *ret, const ham_arat *other){ mpq_set(ret->mpq, other->mpq); }

ham_nonnull_args(1, 2, 3)
ham_nothrow static inline void ham_arat_set_aint(ham_arat *ret, const ham_aint *num, const ham_aint *den){
	mpz_set(mpq_numref(ret->mpq), num->mpz);
	mpz_set(mpq_denref(ret->mpq), den->mpz);
	mpq_canonicalize(ret->mpq);
}

ham_nonnull_args(1)
ham_nothrow static inline void ham_arat_set_iptr(ham_arat *ret, ham_iptr num, ham_uptr den){
	mpz_set_si(mpq_numref(ret->mpq), num);
	mpz_set_ui(mpq_denref(ret->mpq), den);
	mpq_canonicalize(ret->mpq);
};

ham_nonnull_args(1)
ham_nothrow static inline void ham_arat_set_uptr(ham_arat *ret, ham_uptr num, ham_uptr den){
	mpz_set_ui(mpq_numref(ret->mpq), num);
	mpz_set_ui(mpq_denref(ret->mpq), den);
	mpq_canonicalize(ret->mpq);
};

//
// Arithmetic
//

ham_nonnull_args(1, 2, 3)
ham_nothrow static inline void ham_arat_add(ham_arat *ret, const ham_arat *lhs, const ham_arat *rhs){ mpq_add(ret->mpq, lhs->mpq, rhs->mpq); }

ham_nonnull_args(1, 2, 3)
ham_nothrow static inline void ham_arat_sub(ham_arat *ret, const ham_arat *lhs, const ham_arat *rhs){ mpq_sub(ret->mpq, lhs->mpq, rhs->mpq); }

ham_nonnull_args(1, 2, 3)
ham_nothrow static inline void ham_arat_mul(ham_arat *ret, const ham_arat *lhs, const ham_arat *rhs){ mpq_mul(ret->mpq, lhs->mpq, rhs->mpq); }

ham_nonnull_args(1, 2, 3)
ham_nothrow static inline void ham_arat_div(ham_arat *ret, const ham_arat *lhs, const ham_arat *rhs){ mpq_div(ret->mpq, lhs->mpq, rhs->mpq); }

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

//
// Initialization and finalization
//

ham_nonnull_args(1)
ham_nothrow static inline void ham_areal_init(ham_areal *areal){ mpfr_init(areal->mpfr); }

ham_nonnull_args(1)
ham_nothrow static inline void ham_areal_finish(ham_areal *areal){ mpfr_clear(areal->mpfr); }

//
// Simultaneous init and set
//

ham_nonnull_args(1, 2)
ham_nothrow static inline void ham_areal_init_set(ham_areal *areal, const ham_areal *other){ mpfr_init_set(areal->mpfr, other->mpfr, MPFR_RNDN); }

ham_nonnull_args(1, 2)
ham_nothrow static inline void ham_areal_init_arat(ham_areal *areal, const ham_arat *arat){ mpfr_init_set_q(areal->mpfr, arat->mpq, MPFR_RNDN); }

ham_nonnull_args(1, 2)
ham_nothrow static inline void ham_areal_init_aint(ham_areal *areal, const ham_aint *aint){ mpfr_init_set_z(areal->mpfr, aint->mpz, MPFR_RNDN); }

ham_nonnull_args(1)
ham_nothrow static inline void ham_areal_init_iptr(ham_areal *areal, ham_iptr val){ mpfr_init_set_si(areal->mpfr, val, MPFR_RNDN); }

ham_nonnull_args(1)
ham_nothrow static inline void ham_areal_init_uptr(ham_areal *areal, ham_uptr val){ mpfr_init_set_ui(areal->mpfr, val, MPFR_RNDN); }

ham_nonnull_args(1)
ham_nothrow static inline void ham_areal_init_f32 (ham_areal *areal, ham_f32 val){ mpfr_init_set_d(areal->mpfr, val, MPFR_RNDN); }

ham_nonnull_args(1)
ham_nothrow static inline void ham_areal_init_f64 (ham_areal *areal, ham_f64 val){ mpfr_init_set_d(areal->mpfr, val, MPFR_RNDN); }

ham_nonnull_args(1)
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

ham_nonnull_args(1)
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

ham_nonnull_args(1)
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

#define HAM_AREAL_INIT_STR_UTF(n) HAM_CONCAT(ham_areal_init_str_utf, n)

#define ham_areal_init_str HAM_AREAL_INIT_STR_UTF(HAM_UTF)

//
// Assignment
//

ham_nonnull_args(1, 2)
ham_nothrow static inline void ham_areal_set(ham_areal *ret, const ham_areal *other){ mpfr_set(ret->mpfr, other->mpfr, MPFR_RNDN); }

ham_nonnull_args(1, 2)
ham_nothrow static inline void ham_areal_set_arat(ham_areal *ret, const ham_arat *val){ mpfr_set_q(ret->mpfr, val->mpq, MPFR_RNDN); }

ham_nonnull_args(1, 2)
ham_nothrow static inline void ham_areal_set_aint(ham_areal *ret, const ham_aint *val){ mpfr_set_z(ret->mpfr, val->mpz, MPFR_RNDN); }

ham_nonnull_args(1)
ham_nothrow static inline void ham_areal_set_iptr(ham_areal *ret, ham_iptr val){ mpfr_set_si(ret->mpfr, val, MPFR_RNDN); }

ham_nonnull_args(1)
ham_nothrow static inline void ham_areal_set_uptr(ham_areal *ret, ham_uptr val){ mpfr_set_ui(ret->mpfr, val, MPFR_RNDN); }

ham_nonnull_args(1)
ham_nothrow static inline void ham_areal_set_f32(ham_areal *ret, ham_f32 val){ mpfr_set_flt(ret->mpfr, val, MPFR_RNDN); }

ham_nonnull_args(1)
ham_nothrow static inline void ham_areal_set_f64(ham_areal *ret, ham_f64 val){ mpfr_set_d(ret->mpfr, val, MPFR_RNDN); }

//
// Arithmetic
//

ham_nonnull_args(1, 2, 3)
ham_nothrow static inline void ham_areal_add(ham_areal *ret, const ham_areal *lhs, const ham_areal *rhs){ mpfr_add(ret->mpfr, lhs->mpfr, rhs->mpfr, MPFR_RNDN); }

ham_nonnull_args(1, 2, 3)
ham_nothrow static inline void ham_areal_sub(ham_areal *ret, const ham_areal *lhs, const ham_areal *rhs){ mpfr_sub(ret->mpfr, lhs->mpfr, rhs->mpfr, MPFR_RNDN); }

ham_nonnull_args(1, 2, 3)
ham_nothrow static inline void ham_areal_mul(ham_areal *ret, const ham_areal *lhs, const ham_areal *rhs){ mpfr_mul(ret->mpfr, lhs->mpfr, rhs->mpfr, MPFR_RNDN); }

ham_nonnull_args(1, 2, 3)
ham_nothrow static inline void ham_areal_div(ham_areal *ret, const ham_areal *lhs, const ham_areal *rhs){ mpfr_div(ret->mpfr, lhs->mpfr, rhs->mpfr, MPFR_RNDN); }

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
			meta::static_fn<ham_aint_init_str_utf8>,
			meta::static_fn<ham_aint_init_str_utf16>,
			meta::static_fn<ham_aint_init_str_utf32>
		>{};

		template<typename Char>
		constexpr inline auto areal_ctype_init_str = utf_conditional_t<
			Char,
			meta::static_fn<ham_areal_init_str_utf8>,
			meta::static_fn<ham_areal_init_str_utf16>,
			meta::static_fn<ham_areal_init_str_utf32>
		>{};
	}

	class aint{
		public:
			aint() noexcept{ ham_aint_init(&m_val); }

			~aint(){ ham_aint_finish(&m_val); }

			aint(const aint &other) noexcept{ ham_aint_init_set(&m_val, &other.m_val); }

			aint(const ham_aint &cvalue) noexcept{ ham_aint_init_set(&m_val, &cvalue); }

			// needed for overload resolution
			aint(int val_) noexcept{ ham_aint_init_iptr(&m_val, val_); }

			aint(iptr val_) noexcept{ ham_aint_init_iptr(&m_val, val_); }
			aint(uptr val_) noexcept{ ham_aint_init_uptr(&m_val, val_); }

			template<typename Char>
			explicit aint(const ham::basic_str<Char> &str_, u16 base = 10){
				detail::aint_ctype_init_str<Char>(&m_val, str_, base);
			}

			aint &operator=(const aint &other) noexcept{
				if(this != &other){
					ham_aint_set(&m_val, &other.m_val);
				}

				return *this;
			}

			// needed for overload resolution
			aint &operator=(int val) noexcept{
				ham_aint_set_iptr(&m_val, val);
				return *this;
			}

			aint &operator=(iptr val) noexcept{
				ham_aint_set_iptr(&m_val, val);
				return *this;
			}

			aint &operator=(uptr val) noexcept{
				ham_aint_set_uptr(&m_val, val);
				return *this;
			}

		private:
			ham_aint m_val;

			friend class arat;
			friend class areal;
	};

	class arat{
		public:
			arat() noexcept{ ham_arat_init(&m_val); }

			~arat(){ ham_arat_finish(&m_val); }

			arat(const arat &other) noexcept{ ham_arat_init_set(&m_val, &other.m_val); }
			arat(const aint &num_, const aint &den_ = 1) noexcept{ ham_arat_init_aint(&m_val, &num_.m_val, &den_.m_val); }

			arat(const ham_arat &cvalue) noexcept{ ham_arat_init_set(&m_val, &cvalue); }

			// needed for overload resolution
			arat(int  num_, uptr den_ = 1) noexcept{ ham_arat_init_iptr(&m_val, num_, den_); }

			arat(iptr num_, uptr den_ = 1) noexcept{ ham_arat_init_iptr(&m_val, num_, den_); }
			arat(uptr num_, uptr den_ = 1) noexcept{ ham_arat_init_uptr(&m_val, num_, den_); }

			arat &operator=(const arat &other) noexcept{
				if(this != &other) ham_arat_set(&m_val, &other.m_val);
				return *this;
			}

			arat &operator=(const aint &value) noexcept{
				const aint den_ = 1;
				ham_arat_set_aint(&m_val, &value.m_val, &den_.m_val);
				return *this;
			}

			arat &operator=(int value) noexcept{
				ham_arat_set_iptr(&m_val, value, 1);
				return *this;
			}

			arat &operator=(iptr value) noexcept{
				ham_arat_set_iptr(&m_val, value, 1);
				return *this;
			}

			arat &operator=(uptr value) noexcept{
				ham_arat_set_uptr(&m_val, value, 1);
				return *this;
			}

		private:
			ham_arat m_val;

			friend class areal;
	};

	class areal{
		public:
			areal() noexcept{ ham_areal_init(&m_val); }

			~areal(){ ham_areal_finish(&m_val); }

			areal(const areal &other) noexcept{ ham_areal_init_set (&m_val, &other.m_val); }
			areal(const arat  &arat_) noexcept{ ham_areal_init_arat(&m_val, &arat_.m_val); }
			areal(const aint  &aint_) noexcept{ ham_areal_init_aint(&m_val, &aint_.m_val); }

			areal(const ham_areal &cvalue) noexcept{ ham_areal_init_set (&m_val, &cvalue); }
			areal(const ham_arat  &cvalue) noexcept{ ham_areal_init_arat(&m_val, &cvalue); }
			areal(const ham_aint  &cvalue) noexcept{ ham_areal_init_aint(&m_val, &cvalue); }

			// needed for overload resolution
			areal(int val_) noexcept{ ham_areal_init_iptr(&m_val, val_); }

			areal(iptr val_) noexcept{ ham_areal_init_iptr(&m_val, val_); }
			areal(uptr val_) noexcept{ ham_areal_init_uptr(&m_val, val_); }

			areal(f32 val_) noexcept{ ham_areal_init_f32(&m_val, val_); }
			areal(f64 val_) noexcept{ ham_areal_init_f64(&m_val, val_); }

			template<typename Char>
			explicit areal(const ham::basic_str<Char> &str_, u16 base = 10){
				detail::areal_ctype_init_str<Char>(&m_val, str_, base);
			}

			areal &operator=(const areal &other) noexcept{
				if(this != &other) ham_areal_set(&m_val, &other.m_val);
				return *this;
			}

			areal &operator=(const arat &value) noexcept{
				ham_areal_set_arat(&m_val, &value.m_val);
				return *this;
			}

			areal &operator=(const aint &value) noexcept{
				ham_areal_set_aint(&m_val, &value.m_val);
				return *this;
			}

			// needed for overload resolution
			areal &operator=(int val) noexcept{
				ham_areal_set_iptr(&m_val, val);
				return *this;
			}

			areal &operator=(iptr val) noexcept{
				ham_areal_set_iptr(&m_val, val);
				return *this;
			}

			areal &operator=(uptr val) noexcept{
				ham_areal_set_uptr(&m_val, val);
				return *this;
			}

			areal &operator=(f32 val) noexcept{
				ham_areal_set_f32(&m_val, val);
				return *this;
			}

			areal &operator=(f64 val) noexcept{
				ham_areal_set_f64(&m_val, val);
				return *this;
			}

		private:
			ham_areal m_val;
	};
}

#endif // __cplusplus

/**
 * @}
 */

#endif // !HAM_MATH_H
