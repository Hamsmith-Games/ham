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

#ifndef HAM_MATH_H
#define HAM_MATH_H 1

/**
 * @defgroup HAM_MATH Math
 * @ingroup HAM
 * @{
 */

#include "simd.h"

#include "gmp.h"
#include "mpfr.h"

#include <string.h>

#define ham_math_api ham_nothrow ham_used static inline

HAM_C_API_BEGIN

//! @cond ignore

// unused unless proven necessary
//ham_constexpr ham_math_api ham_f32 ham_impl_rsqrtf(ham_f32 x){
//	// evil floating point bit level "what the fuck?"
//	const ham_i32 i = 0x5f375a86 - (ham_bit_cast(ham_i32, x) >> 1);

//	const ham_f32 x_2 = x * 0.5f;

//	ham_f32 y = ham_bit_cast(ham_f32, i);
//	y *= 1.5f - x_2 * y * y;
//	//y *= 1.5f - x_2 * y * y; // second iteration gives better accuracy

//	return y;
//}

ham_math_api ham_f32 ham_impl_intrin_rsqrtf(ham_f32 x){
#ifdef __x86_64__
	__m128 tmp = _mm_set_ss(x);
	tmp = _mm_rsqrt_ss(tmp);
	return _mm_cvtss_f32(tmp);
#elif defined(__aarch64__) || defined(__neon__)
	float32x2_t tmp = vdup_n_f32(x);
	tmp = vrsqrte_f32(tmp);
	return vget_lane_f32(tmp, 0);
#else
	return 1.f / sqrtf(x);
#endif
}

ham_math_api ham_f32 ham_impl_instrin_rcpf(ham_f32 x){
#ifdef __x86_64__
	__m128 tmp = _mm_set_ss(x);
	tmp = _mm_rcp_ss(tmp);
	return _mm_cvtss_f32(tmp);
#elif defined(__aarch64__) || defined(__neon__)
	float32x2_t tmp = vdup_n_f32(x);
	tmp = vrecpe_f32(tmp);
	return vget_lane_f32(tmp, 0);
#else
	return 1.f / x;
#endif
}

//! @endcond

/**
 * @brief Inverse square root.
 * Calls either a native rsqrt instruction or returns exactly ``1.f / sqrtf(x)``.
 * For more info see https://en.wikipedia.org/wiki/Fast_inverse_square_root .
 * @param x number to aproximate inverse square root
 * @returns approximately ``1.f / sqrtf(x)``
 */
ham_constexpr ham_math_api ham_f32 ham_rsqrtf(ham_f32 x){
#	ifdef __cplusplus
	if(std::is_constant_evaluated()) return 1.f / sqrtf(x);
#	endif

	return ham_impl_intrin_rsqrtf(x);
}

/**
 * @brief Reciprocal.
 * Calls either a native reciprocal instruction or returns exactly ``1.f / x``.
 * @param x number to approximate reciprocal
 * @returns approximately ``1.f / x``
 */
ham_constexpr ham_math_api ham_f32 ham_rcpf(ham_f32 x){
#	ifdef __cplusplus
	if(std::is_constant_evaluated()) return 1.f / x;
#	endif

	return ham_impl_instrin_rcpf(x);
}


/**
 * @defgroup HAM_VECS Vectors
 * @{
 */

typedef union alignas(8) ham_vec2{
	struct { ham_f32 x, y; };
	ham_f32 data[2];
} ham_vec2;

typedef union ham_vec3{
	struct { ham_f32 x, y, z; };
	ham_f32 data[3];
} ham_vec3;

typedef union alignas(16) ham_vec4{
	struct { ham_f32 x, y, z, w; };
	ham_f32 data[4];
#ifdef HAM_SIMD
	ham_v4f32 v4f32;
#endif
} ham_vec4;

typedef union alignas(8) ham_vec2i{
	struct { ham_i32 x, y; };
	ham_i32 data[2];
} ham_vec2i;

typedef union ham_vec3i{
	struct { ham_i32 x, y, z; };
	ham_i32 data[3];
} ham_vec3i;

typedef union alignas(16) ham_vec4i{
	struct { ham_i32 x, y, z, w; };
	ham_i32 data[4];
#ifdef HAM_SIMD
	ham_v4i32 v4i32;
#endif
} ham_vec4i;

ham_constexpr ham_math_api ham_vec2 ham_make_vec2(ham_f32 x, ham_f32 y){ return (ham_vec2){ .data = { x, y } }; }
ham_constexpr ham_math_api ham_vec3 ham_make_vec3(ham_f32 x, ham_f32 y, ham_f32 z){ return (ham_vec3){ .data = { x, y, z } }; }

ham_constexpr ham_math_api ham_vec4 ham_make_vec4(ham_f32 x, ham_f32 y, ham_f32 z, ham_f32 w){
#if defined(__cplusplus) && defined(HAM_SIMD)
	if(std::is_constant_evaluated()) return (ham_vec4){ .data = { x, y, z, w } };
#endif

	return (ham_vec4){
#ifdef HAM_SIMD
		.v4f32 = (ham_v4f32){ x, y, z, w }
#else
		.data = { x, y, z, w }
#endif
	};
}

ham_constexpr ham_math_api ham_vec2i ham_make_vec2i(ham_i32 x, ham_i32 y){ return (ham_vec2i){ .data = { x, y } }; }
ham_constexpr ham_math_api ham_vec3i ham_make_vec3i(ham_i32 x, ham_i32 y, ham_i32 z){ return (ham_vec3i){ .data = { x, y, z } }; }

ham_constexpr ham_math_api ham_vec4i ham_make_vec4i(ham_i32 x, ham_i32 y, ham_i32 z, ham_i32 w){
#if defined(__cplusplus) && defined(HAM_SIMD)
	if(std::is_constant_evaluated()) return (ham_vec4i){ .data = { x, y, z, w } };
#endif

	return (ham_vec4i){
#ifdef HAM_SIMD
		.v4i32 = (ham_v4i32){ x, y, z, w }
#else
		.data = { x, y, z, w }
#endif
	};
}

ham_constexpr ham_math_api ham_vec2 ham_make_vec2_scalar(ham_f32 all){ return (ham_vec2){ .data = { all, all } }; }
ham_constexpr ham_math_api ham_vec3 ham_make_vec3_scalar(ham_f32 all){ return (ham_vec3){ .data = { all, all, all } }; }

ham_constexpr ham_math_api ham_vec4 ham_make_vec4_scalar(ham_f32 all){
#if defined(__cplusplus) && defined(HAM_SIMD)
	if(std::is_constant_evaluated()) return (ham_vec4){ .data = { all, all, all, all } };
#endif

	return (ham_vec4){
#ifdef HAM_SIMD
		.v4f32 =
#	ifdef __x86_64__
				_mm_set1_ps(all)
#	else
				(ham_v4f32){ all, all, all, all }
#	endif
#else
		.data = { all, all, all, all }
#endif
	};
}

ham_constexpr ham_math_api ham_vec2i ham_make_vec2i_scalar(ham_i32 all){ return (ham_vec2i){ .data = { all, all } }; }
ham_constexpr ham_math_api ham_vec3i ham_make_vec3i_scalar(ham_i32 all){ return (ham_vec3i){ .data = { all, all, all } }; }

ham_constexpr ham_math_api ham_vec4i ham_make_vec4i_scalar(ham_i32 all){
#if defined(__cplusplus) && defined(HAM_SIMD)
	if(std::is_constant_evaluated()) return (ham_vec4i){ .data = { all, all, all, all } };
#endif

	return (ham_vec4i){
#ifdef HAM_SIMD
		.v4i32 =
#	ifdef __x86_64__
			(ham_v4i32)_mm_set1_epi32(all)
#	else
			(ham_v4f32){ all, all, all, all }
#	endif
#else
		.data = { all, all, all, all }
#endif
	};
}

ham_constexpr ham_math_api ham_vec2 ham_vec2_shuffle(ham_vec2 v, ham_u32 i, ham_u32 j){
	return (ham_vec2){ .data = { v.data[i], v.data[j] } };
}

ham_constexpr ham_math_api ham_vec3 ham_vec3_shuffle(ham_vec3 v, ham_u32 i, ham_u32 j, ham_u32 k){
	return (ham_vec3){ .data = { v.data[i], v.data[j], v.data[k] } };
}

ham_constexpr ham_math_api ham_vec4 ham_vec4_shuffle(ham_vec4 v, ham_u32 i, ham_u32 j, ham_u32 k, ham_u32 l){
	return (ham_vec4){ .data = { v.data[i], v.data[j], v.data[k], v.data[l] } };
}

//! @cond ignore

#define HAM_IMPL_VEC_OP_2(a_, b_, op_) ((ham_vec2){ .data = { a_.x op_ b_.x, a_.y op_ b_.y } })
#define HAM_IMPL_VEC_OP_3(a_, b_, op_) ((ham_vec3){ .data = { a_.x op_ b_.x, a_.y op_ b_.y, a_.z op_ b_.z } })

#ifdef HAM_SIMD
#	define HAM_IMPL_VEC_OP_4(a_, b_, op_) ((ham_vec4){ .v4f32 = a_.v4f32 op_ b_.v4f32 })
#else
#	define HAM_IMPL_VEC_OP_4(a_, b_, op_) ((ham_vec4){ .data = { a_.x op_ b_.x, a_.y op_ b_.y, a_.z op_ b_.z, a_.w op_ b_.w } })
#endif

#define HAM_IMPL_VEC_OP_N_(impl_, a_, b_, op_) impl_(a_, b_, op_)
#define HAM_IMPL_VEC_OP_N(n_, a_, b_, op_) HAM_IMPL_VEC_OP_N_(HAM_IMPL_VEC_OP_##n_, a_, b_, op_)

//! @endcond

ham_constexpr ham_math_api ham_vec2 ham_vec2_add(ham_vec2 a, ham_vec2 b){ return HAM_IMPL_VEC_OP_N(2, a, b, +); }
ham_constexpr ham_math_api ham_vec2 ham_vec2_sub(ham_vec2 a, ham_vec2 b){ return HAM_IMPL_VEC_OP_N(2, a, b, -); }
ham_constexpr ham_math_api ham_vec2 ham_vec2_mul(ham_vec2 a, ham_vec2 b){ return HAM_IMPL_VEC_OP_N(2, a, b, *); }
ham_constexpr ham_math_api ham_vec2 ham_vec2_div(ham_vec2 a, ham_vec2 b){ return HAM_IMPL_VEC_OP_N(2, a, b, /); }

ham_constexpr ham_math_api ham_vec3 ham_vec3_neg(ham_vec3 v){ return (ham_vec3){ .data = { -v.x, -v.y, -v.z } }; }
ham_constexpr ham_math_api ham_vec3 ham_vec3_add(ham_vec3 a, ham_vec3 b){ return HAM_IMPL_VEC_OP_N(3, a, b, +); }
ham_constexpr ham_math_api ham_vec3 ham_vec3_sub(ham_vec3 a, ham_vec3 b){ return HAM_IMPL_VEC_OP_N(3, a, b, -); }
ham_constexpr ham_math_api ham_vec3 ham_vec3_mul(ham_vec3 a, ham_vec3 b){ return HAM_IMPL_VEC_OP_N(3, a, b, *); }
ham_constexpr ham_math_api ham_vec3 ham_vec3_div(ham_vec3 a, ham_vec3 b){ return HAM_IMPL_VEC_OP_N(3, a, b, /); }

ham_constexpr ham_math_api ham_vec4 ham_vec4_add(ham_vec4 a, ham_vec4 b){ return HAM_IMPL_VEC_OP_N(4, a, b, +); }
ham_constexpr ham_math_api ham_vec4 ham_vec4_sub(ham_vec4 a, ham_vec4 b){ return HAM_IMPL_VEC_OP_N(4, a, b, -); }
ham_constexpr ham_math_api ham_vec4 ham_vec4_mul(ham_vec4 a, ham_vec4 b){ return HAM_IMPL_VEC_OP_N(4, a, b, *); }
ham_constexpr ham_math_api ham_vec4 ham_vec4_div(ham_vec4 a, ham_vec4 b){ return HAM_IMPL_VEC_OP_N(4, a, b, /); }

ham_constexpr ham_math_api ham_f32 ham_vec2_dot(ham_vec2 a, ham_vec2 b){
	const ham_vec2 c = ham_vec2_mul(a, b);
	return c.x + c.y;
}

ham_constexpr ham_math_api ham_f32 ham_vec3_dot(ham_vec3 a, ham_vec3 b){
	const ham_vec3 c = ham_vec3_mul(a, b);
	return c.x + c.y + c.z;
}

ham_constexpr ham_math_api ham_f32 ham_vec4_dot(ham_vec4 a, ham_vec4 b){
	const ham_vec4 c = ham_vec4_mul(a, b);
	return c.x + c.y + c.z + c.w;
}

ham_constexpr ham_math_api ham_vec3 ham_vec3_cross(ham_vec3 a, ham_vec3 b){
	return (ham_vec3){
		.data = {
			a.y * b.z - a.z * b.y,
			a.z * b.x - a.x * b.z,
			a.x * b.y - a.y * b.x,
		}
	};
}

ham_constexpr ham_math_api ham_f32 ham_vec3_length(ham_vec3 v){
	return sqrtf(ham_vec3_dot(v, v));
}

ham_constexpr ham_math_api ham_f32 ham_vec4_length(ham_vec4 v){
	return sqrtf(ham_vec4_dot(v, v));
}

ham_constexpr ham_math_api ham_vec3 ham_vec3_normalize(ham_vec3 v){
	const ham_f32 inv_len = 1.f / ham_vec3_length(v);
	return (ham_vec3){ .data = { v.x * inv_len, v.y * inv_len, v.z * inv_len } };
}

ham_constexpr ham_math_api ham_vec4 ham_vec4_normalize(ham_vec4 v){
	const ham_f32 vdot = ham_vec4_dot(v, v);
	const ham_f32 inv_len = ham_rsqrtf(vdot);

#ifdef __cplusplus
	if(std::is_constant_evaluated()) return (ham_vec4){ .data = { v.x * inv_len, v.y * inv_len, v.z * inv_len, v.w * inv_len } };
#endif

	return (ham_vec4){
#ifdef HAM_SIMD
		.v4f32 = v.v4f32 * inv_len
#else
		.data = { v.x * inv_len, v.y * inv_len, v.z * inv_len, v.w * inv_len }
#endif
	};
}

/**
 * @}
 */

/**
 * @defgroup HAM_QUAT Quaternions
 * @{
 */

typedef union alignas(16) ham_quat{
	struct { ham_f32 x, y, z, w; };
	ham_f32 data[4];
#ifdef HAM_SIMD
	ham_v4f32 v4f32;
#endif
} ham_quat;

ham_constexpr ham_math_api ham_quat ham_make_quat(ham_f32 x, ham_f32 y, ham_f32 z, ham_f32 w){
	return (ham_quat){ .data = { x, y, z, w } };
}

ham_constexpr ham_math_api ham_quat ham_quat_identity(){
	return (ham_quat){ .data = { 0.f, 0.f, 0.f, 1.f } };
}

ham_constexpr ham_math_api ham_quat ham_quat_axis(ham_f32 angle, ham_vec3 axis){
	const ham_f32 angle_2 = angle * 0.5f;
	const ham_f32 sin_angle_2 = sinf(angle_2);
	return (ham_quat){
		.data = { axis.x * sin_angle_2, axis.y * sin_angle_2, axis.z * sin_angle_2, cosf(angle_2) }
	};
}

ham_constexpr ham_math_api ham_quat ham_quat_pitch(ham_f32 pitch){
	const ham_f32 pitch_2 = pitch * 0.5f;
	return (ham_quat){
		.data = { sinf(pitch_2), 0.f, 0.f, cosf(pitch_2) }
	};
}

ham_constexpr ham_math_api ham_quat ham_quat_yaw(ham_f32 yaw){
	const ham_f32 yaw_2 = yaw * 0.5f;
	return (ham_quat){
		.data = { 0.f, sinf(yaw_2), 0.f, cosf(yaw_2) }
	};
}

ham_constexpr ham_math_api ham_quat ham_quat_roll(ham_f32 roll){
	const ham_f32 roll_2 = roll * 0.5f;
	return (ham_quat){
		.data = { 0.f, 0.f, sinf(roll_2), cosf(roll_2) }
	};
}

ham_constexpr ham_math_api ham_f32 ham_quat_dot(ham_quat a, ham_quat b){
#ifdef HAM_SIMD
	a.v4f32 *= b.v4f32;
#else
	a.x *= b.x;
	a.y *= b.y;
	a.z *= b.z;
	a.w *= b.w;
#endif

	return a.x + a.y + a.z + a.w;
}

ham_constexpr ham_math_api ham_quat ham_quat_conjugate(ham_quat q){
	return (ham_quat){ .data = { -q.x, -q.y, -q.z, q.w } };
}

ham_constexpr ham_math_api ham_quat ham_quat_inverse(ham_quat q){
	const ham_quat conj = ham_quat_conjugate(q);
	const ham_f32  coef = ham_rcpf(ham_quat_dot(q, q));
	return (ham_quat){ .data = { coef * conj.x, coef * conj.y, coef * conj.z, coef * conj.w } };
}

ham_constexpr ham_math_api ham_f32 ham_quat_length(ham_quat q){
	return sqrtf(ham_quat_dot(q, q));
}

ham_constexpr ham_math_api ham_quat ham_quat_normalize(ham_quat q){
	const ham_f32 inv_len = 1.f / ham_quat_length(q);
	return (ham_quat){ .data = { inv_len * q.x, inv_len * q.y, inv_len * q.z, inv_len * q.w } };
}

ham_constexpr ham_math_api ham_quat ham_quat_add(ham_quat a, ham_quat b){
#ifdef HAM_SIMD
	return (ham_quat){ .v4f32 = a.v4f32 + b.v4f32 };
#else
	return (ham_quat){ .data = { a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w } };
#endif
}

ham_constexpr ham_math_api ham_quat ham_quat_mul(ham_quat a, ham_quat b){
	const ham_vec3 a_xyz = ham_make_vec3(a.x, a.y, a.z);
	const ham_vec3 b_xyz = ham_make_vec3(b.x, b.y, b.z);
	const ham_vec3 xyz_cross = ham_vec3_cross(a_xyz, b_xyz);

	return (ham_quat){
#ifdef HAM_SIMD
		.v4f32
#else
		.data
#endif
			= {
				(a.w * b.x) + (b.w * a.x) + xyz_cross.x,
				(a.w * b.y) + (b.w * a.y) + xyz_cross.y,
				(a.w * b.z) + (b.w * a.z) + xyz_cross.z,
				(a.w * b.w) - (a.x * b.x) - (a.y * b.y) - (a.z * b.z),
			}
	};
}

//ham_constexpr ham_math_api ham_quat ham_quat_hamilton(ham_quat a, ham_quat b){
//#ifdef HAM_SIMD
//	return (ham_quat){ .v4f32 = a.v4f32 * (b.x + b.y + b.z + b.w) };
//#else
//	return (ham_quat){
//		.data = {
//			(a.x * b.x) + (a.x * b.y) + (a.x * b.z) + (a.x * b.w),
//			(a.y * b.x) + (a.y * b.y) + (a.y * b.z) + (a.y * b.w),
//			(a.z * b.x) + (a.z * b.y) + (a.z * b.z) + (a.z * b.w),
//			(a.w * b.x) + (a.w * b.y) + (a.w * b.z) + (a.w * b.w),
//		}
//	};
//#endif
//}

ham_constexpr ham_math_api ham_vec3 ham_quat_mul_vec3(ham_quat q, ham_vec3 v){
	const ham_vec3 quat_vec = ham_make_vec3(q.x, q.y, q.z);
	const ham_vec3 uv  = ham_vec3_cross(quat_vec, v);
	const ham_vec3 uuv = ham_vec3_cross(quat_vec, uv);

	ham_vec3 res;
	res = ham_vec3_mul(uv, ham_make_vec3(q.w, q.w, q.w)); // (uv * q.w)
	res = ham_vec3_add(res, uuv); // + uuv
	res = ham_vec3_mul(res, ham_make_vec3(2.f, 2.f, 2.f)); // * 2.f

	return ham_vec3_add(v, res);
}

/**
 * @}
 */

/**
 * @defgroup HAM_MATRICES Matrices
 * @{
 */

typedef union alignas(16) ham_mat2{
	ham_vec2 cols[2];
	ham_f32 data[2*2];
} ham_mat2;

typedef union ham_mat3{
	ham_vec3 cols[3];
	ham_f32 data[3*3];
} ham_mat3;

typedef union alignas(16) ham_mat4{
	ham_vec4 cols[4];
	ham_f32 data[4*4];
} ham_mat4;

ham_constexpr ham_math_api ham_mat2 ham_mat2_identity(){
	return (ham_mat2){
		.data = {
			1.f, 0.f,
			0.f, 1.f
		}
	};
}

ham_constexpr ham_math_api ham_mat3 ham_mat3_identity(){
	return (ham_mat3){
		.data = {
			1.f, 0.f, 0.f,
			0.f, 1.f, 0.f,
			0.f, 0.f, 1.f
		}
	};
}

ham_constexpr ham_math_api ham_mat4 ham_mat4_identity(){
	const ham_vec4 col0 = ham_make_vec4(1.f, 0.f, 0.f, 0.f);
	const ham_vec4 col1 = ham_make_vec4(0.f, 1.f, 0.f, 0.f);
	const ham_vec4 col2 = ham_make_vec4(0.f, 0.f, 1.f, 0.f);
	const ham_vec4 col3 = ham_make_vec4(0.f, 0.f, 0.f, 1.f);

	return (ham_mat4){ .cols = { col0, col1, col2, col3 } };
}

ham_constexpr ham_math_api ham_mat2 ham_mat2_from_f32(ham_f32 f){
	return (ham_mat2){
		.data = {
			f, 0.f,
			0.f, f,
		}
	};
}

ham_constexpr ham_math_api ham_mat3 ham_mat3_from_f32(ham_f32 f){
	return (ham_mat3){
		.data = {
			f, 0.f, 0.f,
			0.f, f, 0.f,
			0.f, 0.f, f,
		}
	};
}

ham_constexpr ham_math_api ham_mat4 ham_mat4_from_f32(ham_f32 f){
	const ham_vec4 col0 = ham_make_vec4(f, 0.f, 0.f, 0.f);
	const ham_vec4 col1 = ham_make_vec4(0.f, f, 0.f, 0.f);
	const ham_vec4 col2 = ham_make_vec4(0.f, 0.f, f, 0.f);
	const ham_vec4 col3 = ham_make_vec4(0.f, 0.f, 0.f, f);

	return (ham_mat4){ .cols = { col0, col1, col2, col3 } };
}

ham_constexpr ham_math_api ham_mat3 ham_mat3_from_quat(ham_quat q){
	const ham_f32 xx = q.x * q.x;
	const ham_f32 yy = q.y * q.y;
	const ham_f32 zz = q.z * q.z;

	return (ham_mat3){
		.data = {
			// column 0
			1.f - 2.f * (yy + zz),
			2.f * (q.x * q.y + q.z * q.w),
			2.f * (q.x * q.z - q.y * q.w),

			// column 1
			2.f * (q.x * q.y - q.z * q.w),
			1.f - 2.f * (xx + zz),
			2.f * (q.y * q.z + q.x * q.w),

			// column 2
			2.f * (q.x * q.z + q.y * q.w),
			2.f * (q.y * q.z - q.x * q.w),
			1.f - 2.f * (xx + yy),
		}
	};
}

ham_constexpr ham_math_api ham_mat4 ham_make_mat4_cols(ham_vec4 c0, ham_vec4 c1, ham_vec4 c2, ham_vec4 c3){
	return (ham_mat4){ .cols = { c0, c1, c2, c3 } };
}

ham_constexpr ham_math_api ham_mat4 ham_mat4_from_quat(ham_quat q){
	const ham_f32 xx = q.x * q.x;
	const ham_f32 yy = q.y * q.y;
	const ham_f32 zz = q.z * q.z;

	const ham_vec4 col0 = ham_make_vec4(1.f - 2.f * (yy + zz), 2.f * (q.x * q.y + q.z * q.w), 2.f * (q.x * q.z - q.y * q.w), 0.f);
	const ham_vec4 col1 = ham_make_vec4(2.f * (q.x * q.y - q.z * q.w), 1.f - 2.f * (xx + zz), 2.f * (q.y * q.z + q.x * q.w), 0.f);
	const ham_vec4 col2 = ham_make_vec4(2.f * (q.x * q.z + q.y * q.w), 2.f * (q.y * q.z - q.x * q.w), 1.f - 2.f * (xx + yy), 0.f);
	const ham_vec4 col3 = ham_make_vec4(0.f, 0.f, 0.f, 1.f);

	return (ham_mat4){ .cols = { col0, col1, col2, col3 } };
}

ham_constexpr ham_math_api ham_mat4 ham_look_at(ham_vec3 pos, ham_vec3 focus, ham_vec3 world_up){
	const ham_vec3 f = ham_vec3_normalize(ham_vec3_sub(focus, pos));
	const ham_vec3 s = ham_vec3_normalize(ham_vec3_cross(world_up, f));
	const ham_vec3 u = ham_vec3_cross(f, s);

	ham_mat4 ret;

	ret.cols[0] = ham_make_vec4(s.x, u.x, f.x, 0.f);
	ret.cols[1] = ham_make_vec4(s.y, u.y, f.y, 0.f);
	ret.cols[2] = ham_make_vec4(s.z, u.z, f.z, 0.f);

	ret.cols[3] = ham_make_vec4(
		-ham_vec3_dot(s, pos),
		-ham_vec3_dot(u, pos),
		-ham_vec3_dot(f, pos),
		1.f
	);

	return ret;
}

//! @cond ignore

#ifdef __GNUC__

#	define HAM_IMPL_MAT_MUL(n, a, b) \
		({	const ham_auto n_ = (n); \
			ham_mat##n ret_; \
			for(int i_ = 0; i_ < n_; i_++){ \
				for(int j_ = 0; j_ < n_; j_++){ \
					ham_f32 tmp_ = 0.f; \
					for(int k_ = 0; k_ < n_; k_++){ \
						tmp_ += (a).data[(i_ * n_) + k_] * (b).data[(k_ * n_) + j_]; \
					} \
					ret_.data[(i_ * n_) + j_] = tmp_; \
				} \
			} \
			ret_; })

#else // __cplusplus

#	define HAM_IMPL_MAT_MUL(n, a, b) \
		([](const auto n_, const auto &a_, const auto &b_){ \
			ham_mat##n ret_; \
			for(int i_ = 0; i_ < n_; i_++){ \
				for(int j_ = 0; j_ < n_; j++){ \
					ham_f32 tmp_ = 0.f; \
					for(int k_ = 0; k_ < n_; k_++){ \
						tmp_ += (a).data[(i_ * n_) + k_] * (b).data[(k_ * n_) + j_]; \
					} \
					ret_.data[(i_ * n_) + j_] = tmp_; \
				} \
			} \
			return ret_; \
		}((n), (a), (b)))

#endif

//! @endcond

ham_constexpr ham_math_api ham_mat2 ham_mat2_mul(ham_mat2 a, ham_mat2 b){ return HAM_IMPL_MAT_MUL(2, a, b); }
ham_constexpr ham_math_api ham_mat3 ham_mat3_mul(ham_mat3 a, ham_mat3 b){ return HAM_IMPL_MAT_MUL(3, a, b); }

ham_constexpr ham_math_api ham_mat4 ham_mat4_mul(ham_mat4 a, ham_mat4 b){
	/*
		mat<4, 4, T, Q> Result;
		Result[0] = SrcA0 * SrcB0[0] + SrcA1 * SrcB0[1] + SrcA2 * SrcB0[2] + SrcA3 * SrcB0[3];
		Result[1] = SrcA0 * SrcB1[0] + SrcA1 * SrcB1[1] + SrcA2 * SrcB1[2] + SrcA3 * SrcB1[3];
		Result[2] = SrcA0 * SrcB2[0] + SrcA1 * SrcB2[1] + SrcA2 * SrcB2[2] + SrcA3 * SrcB2[3];
		Result[3] = SrcA0 * SrcB3[0] + SrcA1 * SrcB3[1] + SrcA2 * SrcB3[2] + SrcA3 * SrcB3[3];
		return Result;
	*/

	ham_mat4 result;

#ifdef HAM_SIMD
#	ifdef __cplusplus
	if(!std::is_constant_evaluated()){
#	endif

	for(int i = 0; i < 4; i++){
		const ham_v4f32 e0 = ham_simd_shuffle(b.cols[i].v4f32, b.cols[i].v4f32, 0, 0, 0, 0);
		const ham_v4f32 e1 = ham_simd_shuffle(b.cols[i].v4f32, b.cols[i].v4f32, 1, 1, 1, 1);
		const ham_v4f32 e2 = ham_simd_shuffle(b.cols[i].v4f32, b.cols[i].v4f32, 2, 2, 2, 2);
		const ham_v4f32 e3 = ham_simd_shuffle(b.cols[i].v4f32, b.cols[i].v4f32, 3, 3, 3, 3);

		const ham_v4f32 m0 = a.cols[0].v4f32 * e0;
		const ham_v4f32 m1 = a.cols[1].v4f32 * e1;
		const ham_v4f32 m2 = a.cols[2].v4f32 * e2;
		const ham_v4f32 m3 = a.cols[3].v4f32 * e3;

		const ham_v4f32 a0 = m0 + m1;
		const ham_v4f32 a1 = m2 + m3;

		result.cols[i].v4f32 = a0 + a1;
	}

	return result;

#	ifdef __cplusplus
	}
#	endif
#endif

	for(int i = 0; i < 4; i++){
		const ham_vec4 b_col = b.cols[i];

		result.cols[i] = ham_make_vec4_scalar(0.f);

		for(int j = 0; j < 4; j++){
			result.cols[i] = ham_vec4_add(result.cols[i], ham_vec4_mul(a.cols[j], ham_make_vec4_scalar(b_col.data[j])));
		}
	}

	return result;
}

ham_constexpr ham_math_api ham_mat4 ham_mat4_translate(ham_mat4 m, ham_vec3 xyz){
	m.cols[3] = ham_vec4_add(m.cols[3], ham_make_vec4(xyz.x, xyz.y, xyz.z, 0.f));
	return m;
}

ham_constexpr ham_math_api ham_mat4 ham_mat4_scale(ham_mat4 m, ham_vec3 scale){
	m.data[0] *= scale.x;
	m.data[5] *= scale.y;
	m.data[10] *= scale.z;
	return m;
}

ham_constexpr ham_math_api ham_mat4 ham_mat4_rotate(ham_mat4 m, ham_f32 angle, ham_vec3 axis){
	const ham_f32 c = cosf(angle);
	const ham_f32 s = sinf(angle);

	axis = ham_vec3_normalize(axis);
	ham_vec3 temp = ham_vec3_mul(ham_make_vec3_scalar(1.f - c), axis);

	ham_mat4 rot;
	rot.cols[0].data[0] = c + temp.data[0] * axis.data[0];
	rot.cols[0].data[1] = temp.data[0] * axis.data[1] + s * axis.data[2];
	rot.cols[0].data[2] = temp.data[0] * axis.data[2] - s * axis.data[1];

	rot.cols[1].data[0] = temp.data[1] * axis.data[0] - s * axis.data[2];
	rot.cols[1].data[1] = c + temp.data[1] * axis.data[1];
	rot.cols[1].data[2] = temp.data[1] * axis.data[2] + s * axis.data[0];

	rot.cols[2].data[0] = temp.data[2] * axis.data[0] + s * axis.data[1];
	rot.cols[2].data[1] = temp.data[2] * axis.data[1] - s * axis.data[0];
	rot.cols[2].data[2] = c + temp.data[2] * axis.data[2];

	ham_mat4 ret;

	ret.cols[0] = ham_vec4_add(
		ham_vec4_add(
			ham_vec4_mul(m.cols[0], ham_make_vec4_scalar(rot.cols[0].data[0])),
			ham_vec4_mul(m.cols[1], ham_make_vec4_scalar(rot.cols[0].data[1]))
		),
		ham_vec4_mul(m.cols[2], ham_make_vec4_scalar(rot.cols[0].data[2]))
	);

	ret.cols[1] = ham_vec4_add(
		ham_vec4_add(
			ham_vec4_mul(m.cols[0], ham_make_vec4_scalar(rot.cols[1].data[0])),
			ham_vec4_mul(m.cols[1], ham_make_vec4_scalar(rot.cols[1].data[1]))
		),
		ham_vec4_mul(m.cols[2], ham_make_vec4_scalar(rot.cols[1].data[2]))
	);

	ret.cols[2] = ham_vec4_add(
		ham_vec4_add(
			ham_vec4_mul(m.cols[0], ham_make_vec4_scalar(rot.cols[2].data[0])),
			ham_vec4_mul(m.cols[1], ham_make_vec4_scalar(rot.cols[2].data[1]))
		),
		ham_vec4_mul(m.cols[2], ham_make_vec4_scalar(rot.cols[2].data[2]))
	);

	ret.cols[3] = m.cols[3];

	return ret;
}

ham_constexpr ham_math_api ham_mat4 ham_mat4_transpose(ham_mat4 m){
	const ham_vec4 col0 = ham_make_vec4(m.cols[0].x, m.cols[1].x, m.cols[2].x, m.cols[3].x);
	const ham_vec4 col1 = ham_make_vec4(m.cols[0].y, m.cols[1].y, m.cols[2].y, m.cols[3].y);
	const ham_vec4 col2 = ham_make_vec4(m.cols[0].z, m.cols[1].z, m.cols[2].z, m.cols[3].z);
	const ham_vec4 col3 = ham_make_vec4(m.cols[0].w, m.cols[1].w, m.cols[2].w, m.cols[3].w);
	return (ham_mat4){ .cols = { col0, col1, col2, col3 } };
}

ham_constexpr ham_math_api ham_mat4 ham_mat4_inverse(ham_mat4 m){
	const ham_f32 coef00 = m.cols[2].data[2] * m.cols[3].data[3] - m.cols[3].data[2] * m.cols[2].data[3];
	const ham_f32 coef02 = m.cols[1].data[2] * m.cols[3].data[3] - m.cols[3].data[2] * m.cols[1].data[3];
	const ham_f32 coef03 = m.cols[1].data[2] * m.cols[2].data[3] - m.cols[2].data[2] * m.cols[1].data[3];

	const ham_f32 coef04 = m.cols[2].data[1] * m.cols[3].data[3] - m.cols[3].data[1] * m.cols[2].data[3];
	const ham_f32 coef06 = m.cols[1].data[1] * m.cols[3].data[3] - m.cols[3].data[1] * m.cols[1].data[3];
	const ham_f32 coef07 = m.cols[1].data[1] * m.cols[2].data[3] - m.cols[2].data[1] * m.cols[1].data[3];

	const ham_f32 coef08 = m.cols[2].data[1] * m.cols[3].data[2] - m.cols[3].data[1] * m.cols[2].data[2];
	const ham_f32 coef10 = m.cols[1].data[1] * m.cols[3].data[2] - m.cols[3].data[1] * m.cols[1].data[2];
	const ham_f32 coef11 = m.cols[1].data[1] * m.cols[2].data[2] - m.cols[2].data[1] * m.cols[1].data[2];

	const ham_f32 coef12 = m.cols[2].data[0] * m.cols[3].data[3] - m.cols[3].data[0] * m.cols[2].data[3];
	const ham_f32 coef14 = m.cols[1].data[0] * m.cols[3].data[3] - m.cols[3].data[0] * m.cols[1].data[3];
	const ham_f32 coef15 = m.cols[1].data[0] * m.cols[2].data[3] - m.cols[2].data[0] * m.cols[1].data[3];

	const ham_f32 coef16 = m.cols[2].data[0] * m.cols[3].data[2] - m.cols[3].data[0] * m.cols[2].data[2];
	const ham_f32 coef18 = m.cols[1].data[0] * m.cols[3].data[2] - m.cols[3].data[0] * m.cols[1].data[2];
	const ham_f32 coef19 = m.cols[1].data[0] * m.cols[2].data[2] - m.cols[2].data[0] * m.cols[1].data[2];

	const ham_f32 coef20 = m.cols[2].data[0] * m.cols[3].data[1] - m.cols[3].data[0] * m.cols[2].data[1];
	const ham_f32 coef22 = m.cols[1].data[0] * m.cols[3].data[1] - m.cols[3].data[0] * m.cols[1].data[1];
	const ham_f32 coef23 = m.cols[1].data[0] * m.cols[2].data[1] - m.cols[2].data[0] * m.cols[1].data[1];

	const ham_vec4 fac0 = ham_make_vec4(coef00, coef00, coef02, coef03);
	const ham_vec4 fac1 = ham_make_vec4(coef04, coef04, coef06, coef07);
	const ham_vec4 fac2 = ham_make_vec4(coef08, coef08, coef10, coef11);
	const ham_vec4 fac3 = ham_make_vec4(coef12, coef12, coef14, coef15);
	const ham_vec4 fac4 = ham_make_vec4(coef16, coef16, coef18, coef19);
	const ham_vec4 fac5 = ham_make_vec4(coef20, coef20, coef22, coef23);

	const ham_vec4 vec0 = ham_make_vec4(m.cols[1].data[0], m.cols[0].data[0], m.cols[0].data[0], m.cols[0].data[0]);
	const ham_vec4 vec1 = ham_make_vec4(m.cols[1].data[1], m.cols[0].data[1], m.cols[0].data[1], m.cols[0].data[1]);
	const ham_vec4 vec2 = ham_make_vec4(m.cols[1].data[2], m.cols[0].data[2], m.cols[0].data[2], m.cols[0].data[2]);
	const ham_vec4 vec3 = ham_make_vec4(m.cols[1].data[3], m.cols[0].data[3], m.cols[0].data[3], m.cols[0].data[3]);

	const ham_vec4 inv0 = ham_vec4_add(ham_vec4_sub(ham_vec4_mul(vec1, fac0), ham_vec4_mul(vec2, fac1)), ham_vec4_mul(vec3, fac2));
	const ham_vec4 inv1 = ham_vec4_add(ham_vec4_sub(ham_vec4_mul(vec0, fac0), ham_vec4_mul(vec2, fac3)), ham_vec4_mul(vec3, fac4));
	const ham_vec4 inv2 = ham_vec4_add(ham_vec4_sub(ham_vec4_mul(vec0, fac1), ham_vec4_mul(vec1, fac3)), ham_vec4_mul(vec3, fac5));
	const ham_vec4 inv3 = ham_vec4_add(ham_vec4_sub(ham_vec4_mul(vec0, fac2), ham_vec4_mul(vec1, fac4)), ham_vec4_mul(vec2, fac5));

	const ham_vec4 sign_a = ham_make_vec4( 1.f, -1.f,  1.f, -1.f);
	const ham_vec4 sign_b = ham_make_vec4(-1.f,  1.f, -1.f,  1.f);
	const ham_mat4 inverse = ham_make_mat4_cols(
		ham_vec4_mul(inv0, sign_a),
		ham_vec4_mul(inv1, sign_b),
		ham_vec4_mul(inv2, sign_a),
		ham_vec4_mul(inv3, sign_b)
	);

	const ham_vec4 row0 = ham_make_vec4(inverse.cols[0].data[0], inverse.cols[1].data[0], inverse.cols[2].data[0], inverse.cols[3].data[0]);

	const ham_vec4 dot0 = ham_vec4_mul(m.cols[0], row0);

	const ham_f32 dot1 = (dot0.x + dot0.y) + (dot0.z + dot0.w);

	const ham_f32 inv_det = 1.f / dot1;

	return ham_mat4_mul(inverse, ham_mat4_from_f32(inv_det));
}

ham_constexpr ham_math_api ham_quat ham_quat_from_mat3(ham_mat3 m){
	ham_f32 four_x_squared_minus_1 = m.cols[0].data[0] - m.cols[1].data[1] - m.cols[2].data[2];
	ham_f32 four_y_squared_minus_1 = m.cols[1].data[1] - m.cols[0].data[0] - m.cols[2].data[2];
	ham_f32 four_z_squared_minus_1 = m.cols[2].data[2] - m.cols[0].data[0] - m.cols[1].data[1];
	ham_f32 four_w_squared_minus_1 = m.cols[0].data[0] + m.cols[1].data[1] + m.cols[2].data[2];

	int biggest_idx = 0;
	ham_f32 four_max_squared_minus_1 = four_w_squared_minus_1;

	if(four_x_squared_minus_1 > four_max_squared_minus_1){
		four_max_squared_minus_1 = four_x_squared_minus_1;
		biggest_idx = 1;
	}

	if(four_y_squared_minus_1 > four_max_squared_minus_1){
		four_max_squared_minus_1 = four_y_squared_minus_1;
		biggest_idx = 2;
	}

	if(four_z_squared_minus_1 > four_max_squared_minus_1){
		four_max_squared_minus_1 = four_z_squared_minus_1;
		biggest_idx = 3;
	}

	ham_f32 biggestVal = sqrtf(four_max_squared_minus_1 + 1.f) * 0.5f;
	ham_f32 mult = 0.25f / biggestVal;

	switch(biggest_idx){
		case 0:
			return ham_make_quat((m.cols[1].data[2] - m.cols[2].data[1]) * mult, (m.cols[2].data[0] - m.cols[0].data[2]) * mult, (m.cols[0].data[1] - m.cols[1].data[0]) * mult, biggestVal);
		case 1:
			return ham_make_quat(biggestVal, (m.cols[0].data[1] + m.cols[1].data[0]) * mult, (m.cols[2].data[0] + m.cols[0].data[2]) * mult, (m.cols[1].data[2] - m.cols[2].data[1]) * mult);
		case 2:
			return ham_make_quat((m.cols[0].data[1] + m.cols[1].data[0]) * mult, biggestVal, (m.cols[1].data[2] + m.cols[2].data[1]) * mult, (m.cols[2].data[0] - m.cols[0].data[2]) * mult);
		case 3:
			return ham_make_quat((m.cols[2].data[0] + m.cols[0].data[2]) * mult, (m.cols[1].data[2] + m.cols[2].data[1]) * mult, biggestVal, (m.cols[0].data[1] - m.cols[1].data[0]) * mult);
		default:
			return ham_make_quat(0.f, 0.f, 0.f, 1.f);
	}
}

ham_constexpr ham_math_api ham_quat ham_quat_look_at(ham_vec3 dir, ham_vec3 up){
	const ham_vec3 right = ham_vec3_cross(up, dir);

	ham_mat3 rot;

	rot.cols[0] = ham_vec3_mul(right, ham_make_vec3_scalar(ham_rsqrtf(ham_vec3_dot(right, right))));
	rot.cols[2] = dir;
	rot.cols[1] = ham_vec3_cross(rot.cols[2], rot.cols[0]);

	return ham_quat_from_mat3(rot);
}

/**
 * @}
 */

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
ham_math_api void ham_aint_init(ham_aint *aint){ mpz_init(aint->mpz); }

ham_nonnull_args(1)
ham_math_api void ham_aint_finish(ham_aint *aint){ mpz_clear(aint->mpz); }

//
// Simultaneous init and set
//

ham_nonnull_args(1, 2)
ham_math_api void ham_aint_init_set(ham_aint *aint, const ham_aint *other){ mpz_init_set(aint->mpz, other->mpz); }

ham_nonnull_args(1)
ham_math_api void ham_aint_init_iptr(ham_aint *aint, ham_iptr val){ mpz_init_set_si(aint->mpz, val); }

ham_nonnull_args(1)
ham_math_api void ham_aint_init_uptr(ham_aint *aint, ham_uptr val){ mpz_init_set_ui(aint->mpz, val); }

ham_nonnull_args(1)
ham_math_api bool ham_aint_init_str_utf8(ham_aint *aint, ham_str8 str, ham_u16 base){
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
ham_math_api bool ham_aint_init_str_utf16(ham_aint *aint, ham_str16 str, ham_u16 base){
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
ham_math_api bool ham_aint_init_str_utf32(ham_aint *aint, ham_str32 str, ham_u16 base){
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
ham_math_api void ham_aint_set(ham_aint *ret, const ham_aint *other){ mpz_set(ret->mpz, other->mpz); }

ham_nonnull_args(1)
ham_math_api void ham_aint_set_iptr(ham_aint *ret, ham_iptr val){ mpz_set_si(ret->mpz, val); }

ham_nonnull_args(1)
ham_math_api void ham_aint_set_uptr(ham_aint *ret, ham_uptr val){ mpz_set_ui(ret->mpz, val); }

//
// Arithmetic
//

ham_nonnull_args(1, 2, 3)
ham_math_api void ham_aint_add(ham_aint *ret, const ham_aint *lhs, const ham_aint *rhs){ mpz_add(ret->mpz, lhs->mpz, rhs->mpz); }

ham_nonnull_args(1, 2, 3)
ham_math_api void ham_aint_sub(ham_aint *ret, const ham_aint *lhs, const ham_aint *rhs){ mpz_sub(ret->mpz, lhs->mpz, rhs->mpz); }

ham_nonnull_args(1, 2, 3)
ham_math_api void ham_aint_mul(ham_aint *ret, const ham_aint *lhs, const ham_aint *rhs){ mpz_mul(ret->mpz, lhs->mpz, rhs->mpz); }

ham_nonnull_args(1, 2, 3)
ham_math_api void ham_aint_div(ham_aint *ret, const ham_aint *lhs, const ham_aint *rhs){ mpz_div(ret->mpz, lhs->mpz, rhs->mpz); }

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
ham_math_api void ham_arat_init(ham_arat *arat){ mpq_init(arat->mpq); }

ham_nonnull_args(1)
ham_math_api void ham_arat_finish(ham_arat *arat){ mpq_clear(arat->mpq); }

//
// Simultaneous init and set
//

ham_nonnull_args(1, 2)
ham_math_api void ham_arat_init_set(ham_arat *arat, const ham_arat *other){
	mpq_init(arat->mpq);
	mpq_set(arat->mpq, other->mpq);
}

ham_nonnull_args(1, 2, 3)
ham_math_api void ham_arat_init_aint(ham_arat *arat, const ham_aint *numerator, const ham_aint *denominator){
	mpq_init(arat->mpq);
	mpz_set(mpq_numref(arat->mpq), numerator->mpz);
	mpz_set(mpq_denref(arat->mpq), denominator->mpz);
	mpq_canonicalize(arat->mpq);
}

ham_nonnull_args(1)
ham_math_api void ham_arat_init_iptr(ham_arat *arat, ham_iptr numerator, ham_uptr denominator){
	mpq_init(arat->mpq);
	mpz_set_si(mpq_numref(arat->mpq), numerator);
	mpz_set_ui(mpq_denref(arat->mpq), denominator);
	mpq_canonicalize(arat->mpq);
}

ham_nonnull_args(1)
ham_math_api void ham_arat_init_uptr(ham_arat *arat, ham_uptr numerator, ham_uptr denominator){
	mpq_init(arat->mpq);
	mpz_set_ui(mpq_numref(arat->mpq), numerator);
	mpz_set_ui(mpq_denref(arat->mpq), denominator);
	mpq_canonicalize(arat->mpq);
}

//
// Assignment
//

ham_nonnull_args(1, 2)
ham_math_api void ham_arat_set(ham_arat *ret, const ham_arat *other){ mpq_set(ret->mpq, other->mpq); }

ham_nonnull_args(1, 2, 3)
ham_math_api void ham_arat_set_aint(ham_arat *ret, const ham_aint *num, const ham_aint *den){
	mpz_set(mpq_numref(ret->mpq), num->mpz);
	mpz_set(mpq_denref(ret->mpq), den->mpz);
	mpq_canonicalize(ret->mpq);
}

ham_nonnull_args(1)
ham_math_api void ham_arat_set_iptr(ham_arat *ret, ham_iptr num, ham_uptr den){
	mpz_set_si(mpq_numref(ret->mpq), num);
	mpz_set_ui(mpq_denref(ret->mpq), den);
	mpq_canonicalize(ret->mpq);
};

ham_nonnull_args(1)
ham_math_api void ham_arat_set_uptr(ham_arat *ret, ham_uptr num, ham_uptr den){
	mpz_set_ui(mpq_numref(ret->mpq), num);
	mpz_set_ui(mpq_denref(ret->mpq), den);
	mpq_canonicalize(ret->mpq);
};

//
// Arithmetic
//

ham_nonnull_args(1, 2, 3)
ham_math_api void ham_arat_add(ham_arat *ret, const ham_arat *lhs, const ham_arat *rhs){ mpq_add(ret->mpq, lhs->mpq, rhs->mpq); }

ham_nonnull_args(1, 2, 3)
ham_math_api void ham_arat_sub(ham_arat *ret, const ham_arat *lhs, const ham_arat *rhs){ mpq_sub(ret->mpq, lhs->mpq, rhs->mpq); }

ham_nonnull_args(1, 2, 3)
ham_math_api void ham_arat_mul(ham_arat *ret, const ham_arat *lhs, const ham_arat *rhs){ mpq_mul(ret->mpq, lhs->mpq, rhs->mpq); }

ham_nonnull_args(1, 2, 3)
ham_math_api void ham_arat_div(ham_arat *ret, const ham_arat *lhs, const ham_arat *rhs){ mpq_div(ret->mpq, lhs->mpq, rhs->mpq); }

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

//
// Initialization and finalization
//

ham_nonnull_args(1)
ham_math_api void ham_areal_init(ham_areal *areal){ mpfr_init(areal->mpfr); }

ham_nonnull_args(1)
ham_math_api void ham_areal_finish(ham_areal *areal){ mpfr_clear(areal->mpfr); }

//
// Simultaneous init and set
//

ham_nonnull_args(1, 2)
ham_math_api void ham_areal_init_set(ham_areal *areal, const ham_areal *other){ mpfr_init_set(areal->mpfr, other->mpfr, MPFR_RNDN); }

ham_nonnull_args(1, 2)
ham_math_api void ham_areal_init_arat(ham_areal *areal, const ham_arat *arat){ mpfr_init_set_q(areal->mpfr, arat->mpq, MPFR_RNDN); }

ham_nonnull_args(1, 2)
ham_math_api void ham_areal_init_aint(ham_areal *areal, const ham_aint *aint){ mpfr_init_set_z(areal->mpfr, aint->mpz, MPFR_RNDN); }

ham_nonnull_args(1)
ham_math_api void ham_areal_init_iptr(ham_areal *areal, ham_iptr val){ mpfr_init_set_si(areal->mpfr, val, MPFR_RNDN); }

ham_nonnull_args(1)
ham_math_api void ham_areal_init_uptr(ham_areal *areal, ham_uptr val){ mpfr_init_set_ui(areal->mpfr, val, MPFR_RNDN); }

ham_nonnull_args(1)
ham_math_api void ham_areal_init_f32 (ham_areal *areal, ham_f32 val){ mpfr_init_set_d(areal->mpfr, val, MPFR_RNDN); }

ham_nonnull_args(1)
ham_math_api void ham_areal_init_f64 (ham_areal *areal, ham_f64 val){ mpfr_init_set_d(areal->mpfr, val, MPFR_RNDN); }

ham_nonnull_args(1)
ham_math_api bool ham_areal_init_str_utf8(ham_areal *areal, ham_str8 str, ham_u16 base){
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
ham_math_api bool ham_areal_init_str_utf16(ham_areal *areal, ham_str16 str, ham_u16 base){
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
ham_math_api bool ham_areal_init_str_utf32(ham_areal *areal, ham_str32 str, ham_u16 base){
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
ham_math_api void ham_areal_set(ham_areal *ret, const ham_areal *other){ mpfr_set(ret->mpfr, other->mpfr, MPFR_RNDN); }

ham_nonnull_args(1, 2)
ham_math_api void ham_areal_set_arat(ham_areal *ret, const ham_arat *val){ mpfr_set_q(ret->mpfr, val->mpq, MPFR_RNDN); }

ham_nonnull_args(1, 2)
ham_math_api void ham_areal_set_aint(ham_areal *ret, const ham_aint *val){ mpfr_set_z(ret->mpfr, val->mpz, MPFR_RNDN); }

ham_nonnull_args(1)
ham_math_api void ham_areal_set_iptr(ham_areal *ret, ham_iptr val){ mpfr_set_si(ret->mpfr, val, MPFR_RNDN); }

ham_nonnull_args(1)
ham_math_api void ham_areal_set_uptr(ham_areal *ret, ham_uptr val){ mpfr_set_ui(ret->mpfr, val, MPFR_RNDN); }

ham_nonnull_args(1)
ham_math_api void ham_areal_set_f32(ham_areal *ret, ham_f32 val){ mpfr_set_flt(ret->mpfr, val, MPFR_RNDN); }

ham_nonnull_args(1)
ham_math_api void ham_areal_set_f64(ham_areal *ret, ham_f64 val){ mpfr_set_d(ret->mpfr, val, MPFR_RNDN); }

//
// Arithmetic
//

ham_nonnull_args(1, 2, 3)
ham_math_api void ham_areal_add(ham_areal *ret, const ham_areal *lhs, const ham_areal *rhs){ mpfr_add(ret->mpfr, lhs->mpfr, rhs->mpfr, MPFR_RNDN); }

ham_nonnull_args(1, 2, 3)
ham_math_api void ham_areal_sub(ham_areal *ret, const ham_areal *lhs, const ham_areal *rhs){ mpfr_sub(ret->mpfr, lhs->mpfr, rhs->mpfr, MPFR_RNDN); }

ham_nonnull_args(1, 2, 3)
ham_math_api void ham_areal_mul(ham_areal *ret, const ham_areal *lhs, const ham_areal *rhs){ mpfr_mul(ret->mpfr, lhs->mpfr, rhs->mpfr, MPFR_RNDN); }

ham_nonnull_args(1, 2, 3)
ham_math_api void ham_areal_div(ham_areal *ret, const ham_areal *lhs, const ham_areal *rhs){ mpfr_div(ret->mpfr, lhs->mpfr, rhs->mpfr, MPFR_RNDN); }

/**
 * @}
 */

HAM_C_API_END

#ifdef __cplusplus

namespace ham{
	namespace detail{
		template<typename T, usize N>
		union ham_vec_gen{
			struct {
				T x, y, z, w;
			};
			T data[N];
		};

		template<typename T>
		union ham_vec_gen<T, 2>{
			struct {
				T x, y;
			};
			T data[2];
		};

		template<typename T>
		union ham_vec_gen<T, 3>{
			struct {
				T x, y, z;
			};
			T data[3];
		};

		template<typename T, usize N>
		struct vec_data: id<ham_vec_gen<T, N>>{};

		template<> struct vec_data<f32, 2>: id<ham_vec2>{};
		template<> struct vec_data<f32, 3>: id<ham_vec3>{};
		template<> struct vec_data<f32, 4>: id<ham_vec4>{};

		template<typename T, usize N>
		using vec_data_t = typename vec_data<T, N>::type;

		template<typename T, usize N>
		struct vec_ops{
			using data_type = vec_data_t<T, N>;

			static constexpr void add(data_type &ret, const data_type &lhs, const data_type &rhs) noexcept{
				static_for_n<N>([&ret, &lhs, &rhs]<usize Idx>(){ ret.data[Idx] = lhs.data[Idx] + rhs.data[Idx]; });
			}

			static constexpr void sub(data_type &ret, const data_type &lhs, const data_type &rhs) noexcept{
				static_for_n<N>([&ret, &lhs, &rhs]<usize Idx>(){ ret.data[Idx] = lhs.data[Idx] - rhs.data[Idx]; });
			}

			static constexpr void mul(data_type &ret, const data_type &lhs, const data_type &rhs) noexcept{
				static_for_n<N>([&ret, &lhs, &rhs]<usize Idx>(){ ret.data[Idx] = lhs.data[Idx] * rhs.data[Idx]; });
			}

			static constexpr void div(data_type &ret, const data_type &lhs, const data_type &rhs) noexcept{
				static_for_n<N>([&ret, &lhs, &rhs]<usize Idx>(){ ret.data[Idx] = lhs.data[Idx] / rhs.data[Idx]; });
			}

			static constexpr void dot(T &ret, const data_type &lhs, const data_type &rhs) noexcept{
				ret = T(0);
				for(usize i = 0; i < N; i++){
					ret += lhs.data[i] * rhs.data[i];
				}
				ret = std::sqrt(ret);
			}

			static constexpr void length(T &ret, const data_type &v) noexcept{
				dot(ret, v, v);
			}

			template<
				bool IsFloat = std::is_floating_point_v<T>,
				std::enable_if_t<IsFloat, int> = 0
			>
			static constexpr void normalize(data_type &ret, const data_type &a) noexcept{
				T len_;
				length(len_, a);
				for(usize i = 0; i < N; i++){
					ret.data[i] = a.data[i] / len_;
				}
			}

			template<
				bool IsVec3 = (N == 3),
				std::enable_if_t<IsVec3, int> = 0
			>
			static constexpr void cross(data_type &ret, const data_type &a, const data_type &b) noexcept{
				ret = ham_vec3{
					.data = {
						a.y * b.z - a.z * b.y,
						a.z * b.x - a.x * b.z,
						a.x * b.y - a.y * b.x
					}
				};
			}
		};

		template<>
		struct vec_ops<f32, 4>{
			static constexpr void add(ham_vec4 &ret, const ham_vec4 &lhs, const ham_vec4 &rhs) noexcept{
				ret.v4f32 = lhs.v4f32 + rhs.v4f32;
			}

			static constexpr void sub(ham_vec4 &ret, const ham_vec4 &lhs, const ham_vec4 &rhs) noexcept{
				ret.v4f32 = lhs.v4f32 - rhs.v4f32;
			}

			static constexpr void mul(ham_vec4 &ret, const ham_vec4 &lhs, const ham_vec4 &rhs) noexcept{
				ret.v4f32 = lhs.v4f32 * rhs.v4f32;
			}

			static constexpr void div(ham_vec4 &ret, const ham_vec4 &lhs, const ham_vec4 &rhs) noexcept{
				ret.v4f32 = lhs.v4f32 / rhs.v4f32;
			}

			static constexpr void dot(f32 &ret, const ham_vec4 &lhs, const ham_vec4 &rhs) noexcept{
				ret = ham_vec4_dot(lhs, rhs);
			}

			static constexpr void length(f32 &ret, const ham_vec4 &v) noexcept{
				dot(ret, v, v);
			}

			static constexpr void normalize(ham_vec4 &ret, const ham_vec4 &v) noexcept{
				f32 len_;
				length(len_, v);
				ret.v4f32 = v.v4f32 / len_;
			}
		};
	}

	template<typename T, usize N>
	class basic_vec{
		public:
			static_assert(N > 1);

			constexpr static bool is_ham_vec2 = std::same_as<T, f32> && (N == 2);
			constexpr static bool is_ham_vec3 = std::same_as<T, f32> && (N == 3);
			constexpr static bool is_ham_vec4 = std::same_as<T, f32> && (N == 4);

			using ops = detail::vec_ops<T, N>;

			constexpr basic_vec() noexcept = default;

			constexpr basic_vec(T x_, T y_) noexcept requires (N == 2)
				: m_data{ .data = { x_, y_ } }{}

			constexpr basic_vec(T x_, T y_, T z_) noexcept requires (N == 3)
				: m_data{ .data = { x_, y_, z_ } }{}

			constexpr basic_vec(T x_, T y_, T z_, T w_) noexcept requires (N == 4)
				: m_data{ .data = { x_, y_, z_, w_ } }{}

			constexpr explicit basic_vec(T all) noexcept
				: basic_vec(all, meta::make_index_seq<N>()){}

			constexpr basic_vec(const basic_vec&) noexcept = default;

			constexpr basic_vec &operator=(const basic_vec&) noexcept = default;

			// Conversion ham_vecN <-> basic_vec<f32, N>

			constexpr basic_vec(const ham_vec2 &v) noexcept requires is_ham_vec2
				: m_data(v){}

			constexpr basic_vec(const ham_vec3 &v) noexcept requires is_ham_vec3
				: m_data(v){}

			constexpr basic_vec(const ham_vec4 &v) noexcept requires is_ham_vec4
				: m_data(v){}

			constexpr operator ham_vec2&() noexcept requires is_ham_vec2{ return m_data; }
			constexpr operator const ham_vec2&() const noexcept requires is_ham_vec2{ return m_data; }

			constexpr operator ham_vec3&() noexcept requires is_ham_vec3{ return m_data; }
			constexpr operator const ham_vec3&() const noexcept requires is_ham_vec3{ return m_data; }

			constexpr operator ham_vec4&() noexcept requires is_ham_vec4{ return m_data; }
			constexpr operator const ham_vec4&() const noexcept requires is_ham_vec4{ return m_data; }

			constexpr T &x() noexcept{ return m_data.x; }
			constexpr const T &x() const noexcept{ return m_data.x; }

			constexpr T &y() noexcept{ return m_data.y; }
			constexpr const T &y() const noexcept{ return m_data.y; }

			constexpr T &z() noexcept requires (N > 2){ return m_data.z; }
			constexpr const T &z() const noexcept requires (N > 2){ return m_data.z; }

			constexpr T &w() noexcept requires (N > 3){ return m_data.w; }
			constexpr const T &w() const noexcept requires (N > 3){ return m_data.w; }

			constexpr T *data() noexcept{ return m_data; }
			constexpr const T *data() const noexcept{ return m_data; }

			constexpr basic_vec &operator+=(const basic_vec &other) noexcept{
				ops::add(m_data, m_data, other.m_data);
				return *this;
			}

			constexpr basic_vec &operator-=(const basic_vec &other) noexcept{
				ops::sub(m_data, m_data, other.m_data);
				return *this;
			}

			constexpr basic_vec &operator*=(const basic_vec &other) noexcept{
				ops::mul(m_data, m_data, other.m_data);
				return *this;
			}

			constexpr basic_vec &operator/=(const basic_vec &other) noexcept{
				ops::div(m_data, m_data, other.m_data);
				return *this;
			}

			constexpr basic_vec operator+(const basic_vec &other) noexcept{
				basic_vec ret;
				ops::add(ret.m_data, m_data, other.m_data);
				return ret;
			}

			constexpr basic_vec operator-(const basic_vec &other) noexcept{
				basic_vec ret;
				ops::sub(ret.m_data, m_data, other.m_data);
				return ret;
			}

			constexpr basic_vec operator*(const basic_vec &other) noexcept{
				basic_vec ret;
				ops::mul(ret.m_data, m_data, other.m_data);
				return ret;
			}

			constexpr basic_vec operator/(const basic_vec &other) noexcept{
				basic_vec ret;
				ops::div(ret.m_data, m_data, other.m_data);
				return ret;
			}

			constexpr T dot(const basic_vec &other) noexcept{
				T ret;
				ops::dot(ret, m_data, other.m_data);
				return ret;
			}

			constexpr basic_vec cross(const basic_vec &other) noexcept requires (N == 3){
				basic_vec ret;
				ops::cross(ret.m_data, m_data, other.m_data);
				return ret;
			}

			constexpr T length() const noexcept{
				T ret;
				ops::length(ret, m_data);
				return ret;
			}

			constexpr basic_vec &normalize() noexcept{
				ops::normalize(m_data, m_data);
				return *this;
			}

			constexpr basic_vec normalized() const noexcept{
				basic_vec ret;
				ops::normalize(ret.m_data, m_data);
				return ret;
			}

		private:
			template<usize ... Is>
			basic_vec(T all, meta::index_seq<Is...>)
				: m_data{ .data = { ((void)Is, all)... } }{}

			detail::vec_data_t<T, N> m_data;
	};

	using vec2 = basic_vec<f32, 2>;
	using vec3 = basic_vec<f32, 3>;
	using vec4 = basic_vec<f32, 4>;

	using dvec2 = basic_vec<f64, 2>;
	using dvec3 = basic_vec<f64, 3>;
	using dvec4 = basic_vec<f64, 4>;

	template<typename T, usize N>
	constexpr inline T dot(const basic_vec<T, N> &lhs, const basic_vec<T, N> &rhs) noexcept{
		return lhs.dot(rhs);
	}

	template<typename T>
	constexpr inline basic_vec<T, 3> cross(const basic_vec<T, 3> &lhs, const basic_vec<T, 3> &rhs) noexcept{
		return lhs.cross(rhs);
	}

	template<typename T, usize N>
	constexpr inline T length(const basic_vec<T, N> &v) noexcept{
		return v.length();
	}

	template<typename T, usize N>
	constexpr inline basic_vec<T, N> normalize(const basic_vec<T, N> &v) noexcept{
		return v.normalized();
	}

	class quat{
		public:
			constexpr quat() noexcept
				: m_q{ .data = { 0.f, 0.f, 0.f, 1.f } }{}

			constexpr quat(f32 x_, f32 y_, f32 z_, f32 w_)
				: m_q{ .data = { x_, y_, z_, w_ } }{}

			constexpr quat(const ham_quat &q) noexcept
				: m_q(q){}

			constexpr quat(const quat&) noexcept = default;

			constexpr quat &operator=(const quat&) noexcept = default;

			operator ham_quat&() noexcept{ return m_q; }
			operator const ham_quat&() const noexcept{ return m_q; }

			f32 &x() noexcept{ return m_q.x; }
			const f32 &x() const noexcept{ return m_q.x; }

			f32 &y() noexcept{ return m_q.y; }
			const f32 &y() const noexcept{ return m_q.y; }

			f32 &z() noexcept{ return m_q.z; }
			const f32 &z() const noexcept{ return m_q.z; }

			f32 &w() noexcept{ return m_q.w; }
			const f32 &w() const noexcept{ return m_q.w; }

			f32 *data() noexcept{ return m_q.data; }
			const f32 *data() const noexcept{ return m_q.data; }

			constexpr static quat from_angles(f32 angle, const vec3 &axis) noexcept{ return ham_quat_axis(angle, axis); }

			constexpr static quat from_pitch(f32 pitch) noexcept{ return ham_quat_pitch(pitch); }
			constexpr static quat from_yaw(f32 yaw) noexcept{ return ham_quat_yaw(yaw); }
			constexpr static quat from_roll(f32 roll) noexcept{ return ham_quat_roll(roll); }

			constexpr quat &operator+=(const quat &other) noexcept{
				m_q = ham_quat_add(m_q, other.m_q);
				return *this;
			}

			constexpr quat &operator*=(const quat &other) noexcept{
				m_q = ham_quat_mul(m_q, other.m_q);
				return *this;
			}

			constexpr quat operator+(const quat &other) const noexcept{ return ham_quat_add(m_q, other.m_q); }
			constexpr quat operator*(const quat &other) const noexcept{ return ham_quat_mul(m_q, other.m_q); }

			// Rotate a vector
			constexpr vec3 operator*(const vec3 &v) const noexcept{ return ham_quat_mul_vec3(m_q, v); }

			constexpr f32 length() const noexcept{ return ham_quat_length(m_q); }

			constexpr quat &normalize() noexcept{
				m_q = ham_quat_normalize(m_q);
				return *this;
			}

			constexpr quat normalized() const noexcept{ return ham_quat_normalize(m_q); }

			constexpr ham_mat3 to_mat3() const noexcept{ return ham_mat3_from_quat(m_q); }
			constexpr ham_mat4 to_mat4() const noexcept{ return ham_mat4_from_quat(m_q); }

		private:
			ham_quat m_q;
	};

	namespace detail{
		template<typename T, usize N> struct ham_mat_ctype;

		template<>
		struct ham_mat_ctype<f32, 2>: id<ham_mat2>{
			constexpr static auto identity() noexcept{ return ham_mat2_identity(); }
			constexpr static auto from_f32(f32 f) noexcept{ return ham_mat2_from_f32(f); }
			constexpr static auto mul(const ham_mat2 &a, const ham_mat2 &b) noexcept{ return ham_mat2_mul(a, b); }
		};

		template<>
		struct ham_mat_ctype<f32, 3>: id<ham_mat3>{
			constexpr static auto identity() noexcept{ return ham_mat3_identity(); }
			constexpr static auto from_f32(f32 f) noexcept{ return ham_mat3_from_f32(f); }
			constexpr static auto mul(const ham_mat3 &a, const ham_mat3 &b) noexcept{ return ham_mat3_mul(a, b); }
		};

		template<>
		struct ham_mat_ctype<f32, 4>: id<ham_mat4>{
			constexpr static auto identity() noexcept{ return ham_mat4_identity(); }
			constexpr static auto from_f32(f32 f) noexcept{ return ham_mat4_from_f32(f); }
			constexpr static auto mul(const ham_mat4 &a, const ham_mat4 &b) noexcept{ return ham_mat4_mul(a, b); }
		};

		template<typename T, usize N>
		using ham_mat_ctype_t = typename ham_mat_ctype<T, N>::type;

		template<typename T, usize N>
		constexpr static inline auto ham_mat_ctype_mul(const ham_mat_ctype_t<T, N> &a, const ham_mat_ctype_t<T, N> &b) noexcept{
			return ham_mat_ctype<T, N>::mul(a, b);
		}

		template<typename T, usize N>
		constexpr inline auto ham_mat_ctype_identity_v = ham_mat_ctype<T, N>::identity();

		template<typename T, usize N> struct ham_mat_ctype;
	}

	template<typename T, usize N, usize M>
		requires std::integral<T> || std::floating_point<T>
	class basic_mat;

	// Square matrices
	template<typename T, usize N>
	class basic_mat<T, N, N>{
		public:
			using ctype = detail::ham_mat_ctype_t<T, N>;

			constexpr static bool is_ham_mat2 = std::same_as<ctype, ham_mat2>;
			constexpr static bool is_ham_mat3 = std::same_as<ctype, ham_mat3>;
			constexpr static bool is_ham_mat4 = std::same_as<ctype, ham_mat4>;

			constexpr basic_mat(T scalar) noexcept
				: m_val(detail::ham_mat_ctype<T, N>::from_f32(f32(scalar))){}

			constexpr basic_mat() noexcept
				: m_val(detail::ham_mat_ctype_identity_v<T, N>){}

			constexpr basic_mat(const ctype &val) noexcept
				: m_val(val){}

			static basic_mat from_quat(quat q) noexcept requires is_ham_mat4{
				return ham_mat4_from_quat(q);
			}

			operator ctype&() noexcept{ return m_val; }
			operator const ctype&() const noexcept{ return m_val; }

			constexpr basic_mat &operator*=(const basic_mat &other) noexcept{
				m_val = detail::ham_mat_ctype_mul(m_val, other.m_val);
				return *this;
			}

			constexpr basic_mat operator*(const basic_mat &other) const noexcept{
				return detail::ham_mat_ctype_mul<T, N>(m_val, other.m_val);
			}

			T *data() noexcept{ return m_val.data; }
			const T *data() const noexcept{ return m_val.data; }

			constexpr basic_mat &translate(const vec3 &amnt) noexcept requires is_ham_mat4{
				m_val = ham_mat4_translate(m_val, amnt);
				return *this;
			}

			constexpr basic_mat translated(const vec3 &amnt) const noexcept requires is_ham_mat4{
				return ham_mat4_translate(m_val, amnt);
			}

			constexpr basic_mat &rotate(f32 angle, const vec3 &axis) noexcept requires is_ham_mat4{
				m_val = ham_mat4_rotate(m_val, angle, axis);
				return *this;
			}

			constexpr basic_mat rotated(f32 angle, const vec3 &axis) noexcept requires is_ham_mat4{
				return ham_mat4_rotate(m_val, angle, axis);
			}

			constexpr basic_mat &scale(const vec3 &amnt) noexcept requires is_ham_mat4{
				m_val = ham_mat4_scale(m_val, amnt);
				return *this;
			}

			constexpr basic_mat scaled(const vec3 &amnt) noexcept requires is_ham_mat4{
				return ham_mat4_scale(m_val, amnt);
			}

			constexpr basic_mat inverse() const noexcept requires is_ham_mat4{
				return ham_mat4_inverse(m_val);
			}

		private:
			ctype m_val;
	};

	template<typename T, usize N> using basic_matn = basic_mat<T, N, N>;

	template<typename T> using basic_mat2 = basic_matn<T, 2>;
	template<typename T> using basic_mat3 = basic_matn<T, 3>;
	template<typename T> using basic_mat4 = basic_matn<T, 4>;

	using mat2 = basic_mat2<f32>;
	using mat3 = basic_mat3<f32>;
	using mat4 = basic_mat4<f32>;

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

constexpr inline ham_mat4 operator*(const ham_mat4 &lhs, const ham_mat4 &rhs) noexcept{ return ham_mat4_mul(lhs, rhs); }

constexpr inline ham_vec2 operator+(ham_vec2 lhs, ham_vec2 rhs) noexcept{ return ham_vec2_add(lhs, rhs); }
constexpr inline ham_vec2 operator-(ham_vec2 lhs, ham_vec2 rhs) noexcept{ return ham_vec2_sub(lhs, rhs); }
constexpr inline ham_vec2 operator*(ham_vec2 lhs, ham_vec2 rhs) noexcept{ return ham_vec2_mul(lhs, rhs); }
constexpr inline ham_vec2 operator/(ham_vec2 lhs, ham_vec2 rhs) noexcept{ return ham_vec2_div(lhs, rhs); }

constexpr inline ham_vec2 operator*(ham_f32 coef, ham_vec2 rhs) noexcept{ return ham_vec2_mul(ham_make_vec2(coef, coef), rhs); }
constexpr inline ham_vec2 operator*(ham_vec2 lhs, ham_f32 coef) noexcept{ return ham_vec2_mul(lhs, ham_make_vec2(coef, coef)); }
constexpr inline ham_vec2 operator/(ham_vec2 lhs, ham_f32 coef) noexcept{ return ham_vec2_div(lhs, ham_make_vec2(coef, coef)); }

constexpr inline ham_vec3 operator+(const ham_vec3 &lhs, const ham_vec3 &rhs) noexcept{ return ham_vec3_add(lhs, rhs); }
constexpr inline ham_vec3 operator-(const ham_vec3 &lhs, const ham_vec3 &rhs) noexcept{ return ham_vec3_sub(lhs, rhs); }
constexpr inline ham_vec3 operator*(const ham_vec3 &lhs, const ham_vec3 &rhs) noexcept{ return ham_vec3_mul(lhs, rhs); }
constexpr inline ham_vec3 operator/(const ham_vec3 &lhs, const ham_vec3 &rhs) noexcept{ return ham_vec3_div(lhs, rhs); }

constexpr inline ham_vec3 operator*(ham_f32 coef, ham_vec3 rhs) noexcept{ return ham_vec3_mul(ham_make_vec3_scalar(coef), rhs); }
constexpr inline ham_vec3 operator*(ham_vec3 lhs, ham_f32 coef) noexcept{ return ham_vec3_mul(lhs, ham_make_vec3_scalar(coef)); }
constexpr inline ham_vec3 operator/(ham_vec3 lhs, ham_f32 coef) noexcept{ return ham_vec3_div(lhs, ham_make_vec3_scalar(coef)); }

constexpr inline ham_vec4 operator+(const ham_vec4 &lhs, const ham_vec4 &rhs) noexcept{ return ham_vec4_add(lhs, rhs); }
constexpr inline ham_vec4 operator-(const ham_vec4 &lhs, const ham_vec4 &rhs) noexcept{ return ham_vec4_sub(lhs, rhs); }
constexpr inline ham_vec4 operator*(const ham_vec4 &lhs, const ham_vec4 &rhs) noexcept{ return ham_vec4_mul(lhs, rhs); }
constexpr inline ham_vec4 operator/(const ham_vec4 &lhs, const ham_vec4 &rhs) noexcept{ return ham_vec4_div(lhs, rhs); }

constexpr inline ham_vec4 operator*(ham_f32 coef, ham_vec4 rhs) noexcept{ return ham_vec4_mul(ham_make_vec4_scalar(coef), rhs); }
constexpr inline ham_vec4 operator*(ham_vec4 lhs, ham_f32 coef) noexcept{ return ham_vec4_mul(lhs, ham_make_vec4_scalar(coef)); }
constexpr inline ham_vec4 operator/(ham_vec4 lhs, ham_f32 coef) noexcept{ return ham_vec4_div(lhs, ham_make_vec4_scalar(coef)); }


#endif // __cplusplus

/**
 * @}
 */

#endif // !HAM_MATH_H
