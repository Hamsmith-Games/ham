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

#ifndef HAM_ENGINE_TRANSFORM_H
#define HAM_ENGINE_TRANSFORM_H 1

/**
 * @defgroup HAM_TRANSFORM Transformations
 * @ingroup HAM
 * @{
 */

#include "ham/math.h"

HAM_C_API_BEGIN

typedef struct ham_transform{
	ham_vec3 pos, scale, pyr;

	//! @cond ignore
	ham_mutable bool _impl_dirty;
	ham_mutable ham_quat _impl_rot;
	ham_mutable ham_mat4 _impl_trans;
	//! @endcond
} ham_transform;

ham_nonnull_args(1)
ham_constexpr ham_math_api void ham_transform_reset(ham_transform *trans){
	trans->pos   = ham_make_vec3(0.f, 0.f, 0.f);
	trans->scale = ham_make_vec3(1.f, 1.f, 1.f);
	trans->pyr   = ham_make_vec3(0.f, 0.f, 0.f);

	trans->_impl_dirty = false;
	trans->_impl_rot   = ham_make_quat(0.f, 0.f, 0.f, 1.f);
	trans->_impl_trans = ham_mat4_identity();
}

//! @cond ignore

ham_constexpr ham_math_api void ham_impl_transform_update(const ham_transform *trans){
	// :/
#ifdef __cplusplus
#	define mut_ptr trans
#else
	ham_transform *const mut_ptr = (ham_transform*)trans;
#endif

	ham_constexpr const ham_mat4 ident = ham_mat4_identity();

	const ham_mat4 translation = ham_mat4_translate(ident, ham_vec3_neg(trans->pos));

	const ham_quat qp = ham_quat_pitch(trans->pyr.x);
	const ham_quat qy = ham_quat_yaw(trans->pyr.y);
	const ham_quat qr = ham_quat_roll(trans->pyr.z);
	const ham_quat qpyr = ham_quat_mul(ham_quat_mul(qp, qy), qr);
	mut_ptr->_impl_rot = ham_quat_normalize(qpyr);

	const ham_mat4 rotation = ham_mat4_from_quat(mut_ptr->_impl_rot);

	const ham_mat4 scaling = ham_mat4_scale(ident, trans->scale);

	mut_ptr->_impl_trans = ham_mat4_mul(ham_mat4_mul(translation, rotation), scaling);
	mut_ptr->_impl_dirty = false;

#ifdef __cplusplus
#	undef mut_ptr
#endif
}

//! @endcond

ham_nonnull_args(1)
ham_constexpr ham_math_api ham_vec3 ham_transform_position(const ham_transform *trans){
	return trans->pos;
}

ham_nonnull_args(1)
ham_constexpr ham_math_api ham_quat ham_transform_rotation(const ham_transform *trans){
	if(trans->_impl_dirty) ham_impl_transform_update(trans);
	return trans->_impl_rot;
}

ham_nonnull_args(1)
ham_constexpr ham_math_api ham_vec3 ham_transform_pyr(const ham_transform *trans){
	return trans->pyr;
}

ham_nonnull_args(1)
ham_constexpr ham_math_api ham_vec3 ham_transform_scaling(const ham_transform *trans){
	return trans->scale;
}

ham_nonnull_args(1)
ham_constexpr ham_math_api ham_mat4 ham_transform_matrix(const ham_transform *trans){
	if(trans->_impl_dirty) ham_impl_transform_update(trans);
	return trans->_impl_trans;
}

ham_nonnull_args(1)
ham_constexpr ham_math_api void ham_transform_set_position(ham_transform *trans, ham_vec3 pos){
	trans->pos    = pos;
	trans->_impl_dirty = true;
}

ham_nonnull_args(1)
ham_constexpr ham_math_api void ham_transform_translate(ham_transform *trans, ham_vec3 dP){
	trans->pos    = ham_vec3_add(trans->pos, dP);
	trans->_impl_dirty = true;
}

ham_nonnull_args(1)
ham_constexpr ham_math_api void ham_transform_set_rotation(ham_transform *trans, ham_vec3 pyr){
	trans->pyr = pyr;
	trans->_impl_dirty = true;
}

ham_nonnull_args(1)
ham_constexpr ham_math_api void ham_transform_rotate(ham_transform *trans, ham_f32 angle, ham_vec3 axis){
	trans->pyr = ham_vec3_add(trans->pyr, ham_vec3_mul(ham_make_vec3_scalar(angle), axis));

	trans->pyr.x = fmodf(trans->pyr.x, M_PI * 2.f);
	trans->pyr.y = fmodf(trans->pyr.y, M_PI * 2.f);
	trans->pyr.z = fmodf(trans->pyr.z, M_PI * 2.f);

	trans->_impl_dirty = true;
}

ham_nonnull_args(1)
ham_constexpr ham_math_api void ham_transform_set_scale(ham_transform *trans, ham_vec3 scale){
	trans->scale = scale;
	trans->_impl_dirty = true;
}

ham_nonnull_args(1)
ham_constexpr ham_math_api void ham_transform_scale(ham_transform *trans, ham_vec3 coef){
	trans->scale = ham_vec3_mul(trans->scale, coef);
	trans->_impl_dirty = true;
}

HAM_C_API_END

#ifdef __cplusplus

namespace ham{
	class transform{
		public:
			constexpr transform() noexcept{ ham_transform_reset(&m_trans); }

			constexpr transform(const ham_transform &val) noexcept
				: m_trans(val){}

			constexpr transform(const transform&) noexcept = default;

			constexpr operator ham_transform&() noexcept{ return m_trans; }
			constexpr operator const ham_transform&() const noexcept{ return m_trans; }

			constexpr vec3 position() const noexcept{ return m_trans.pos; }
			constexpr vec3 scaling() const noexcept{ return m_trans.scale; }
			constexpr quat rotation() const noexcept{ return ham_transform_rotation(&m_trans); }
			constexpr vec3 pyr() const noexcept{ return ham_transform_pyr(&m_trans); }

			constexpr mat4 matrix() const noexcept{ return ham_transform_matrix(&m_trans); }

			constexpr transform &operator=(const ham_transform &trans) noexcept{
				m_trans = trans;
				return *this;
			}

			ham_transform *ptr() noexcept{ return &m_trans; }
			const ham_transform *ptr() const noexcept{ return &m_trans; }

			constexpr void reset() noexcept{
				ham_transform_reset(&m_trans);
			}

			constexpr void set_position(const vec3 &pos) noexcept{
				ham_transform_set_position(&m_trans, pos);
			}

			constexpr void set_scale(const vec3 &scale) noexcept{
				ham_transform_set_scale(&m_trans, scale);
			}

			constexpr void set_rotation(const vec3 &pyr) noexcept{
				ham_transform_set_rotation(&m_trans, pyr);
			}

			constexpr void translate(const vec3 &xyz) noexcept{
				ham_transform_translate(&m_trans, xyz);
			}

			constexpr void scale(const vec3 &amnt) noexcept{
				ham_transform_scale(&m_trans, amnt);
			}

			constexpr void rotate(f32 angle, const vec3 &axis) noexcept{
				ham_transform_rotate(&m_trans, angle, axis);
			}

		private:
			ham_transform m_trans;
	};
}

#endif

/**
 * @}
 */

#endif // !HAM_TRANSFORM_H
