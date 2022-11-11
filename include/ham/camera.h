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

#ifndef HAM_CAMERA_H
#define HAM_CAMERA_H 1

/**
 * @defgroup HAM_CAMERA Camera
 * @ingroup HAM
 * @{
 */

#include "transform.h"

typedef enum ham_camera_kind{
	HAM_CAMERA_IDENTITY,
	HAM_CAMERA_PERSPECTIVE,
	HAM_CAMERA_ORTHOGRAPHIC,

	HAM_CAMERA_PERSPECTIVE_REV,

	HAM_CAMERA_KIND_COUNT
} ham_camera_kind;

typedef struct ham_camera{
	ham_camera_kind kind;
	ham_f32 near_z, far_z;
	ham_mat4 proj;

	//! @cond ignore
	ham_transform _impl_trans;
	ham_mutable ham_vec3 _impl_forward, _impl_right, _impl_up;
	//! @endcond
} ham_camera;

ham_nonnull_args(1)
ham_constexpr ham_nothrow static inline void ham_camera_init(ham_camera *cam){
	cam->kind = HAM_CAMERA_IDENTITY;

	cam->near_z = 0.f;
	cam->far_z  = 1.f;

	cam->proj = ham_mat4_identity();

	cam->_impl_forward = ham_make_vec3(0.f, 0.f, 1.f);
	cam->_impl_right   = ham_make_vec3(1.f, 0.f, 0.f);
	cam->_impl_up      = ham_make_vec3(0.f, 1.f, 0.f);

	ham_transform_reset(&cam->_impl_trans);
}

ham_constexpr ham_nothrow static inline void ham_camera_reset_transform(ham_camera *cam){
	cam->_impl_forward = ham_make_vec3(0.f, 0.f, 1.f);
	cam->_impl_right   = ham_make_vec3(1.f, 0.f, 0.f);
	cam->_impl_up      = ham_make_vec3(0.f, 1.f, 0.f);

	ham_transform_reset(&cam->_impl_trans);
}

//! @cond ignore

ham_constexpr ham_nothrow static inline void ham_impl_camera_update(const ham_camera *cam){
	if(!cam->_impl_trans._impl_dirty) return;

#ifdef __cplusplus
#	define mut_ptr cam
#else
#	ham_camera *const mut_ptr = (ham_camera*)cam;
#endif

	const ham_quat qp = ham_quat_pitch(mut_ptr->_impl_trans.pyr.x);
	const ham_quat qy = ham_quat_yaw(mut_ptr->_impl_trans.pyr.y);
	const ham_quat qr = ham_quat_roll(mut_ptr->_impl_trans.pyr.z);
	const ham_quat qpyr = ham_quat_mul(ham_quat_mul(qr, qy), qp);

	mut_ptr->_impl_trans._impl_rot = ham_quat_normalize(qpyr);

	mut_ptr->_impl_forward = ham_vec3_normalize(ham_quat_mul_vec3(cam->_impl_trans._impl_rot, (ham_vec3){0.f, 0.f, 1.f}));
	mut_ptr->_impl_right   = ham_vec3_normalize(ham_quat_mul_vec3(cam->_impl_trans._impl_rot, (ham_vec3){1.f, 0.f, 0.f}));
	mut_ptr->_impl_up      = ham_vec3_normalize(ham_quat_mul_vec3(cam->_impl_trans._impl_rot, (ham_vec3){0.f, 1.f, 0.f}));

	const ham_mat4 trans_m = ham_mat4_translate(ham_mat4_identity(), ham_vec3_neg(cam->_impl_trans.pos));

	const ham_quat viewq = ham_quat_look_at(mut_ptr->_impl_forward, mut_ptr->_impl_up);

//	mut_ptr->_impl_trans._impl_trans = ham_mat4_inverse(mut_ptr->_impl_trans._impl_trans);
//	mut_ptr->_impl_trans._impl_trans = ham_look_at(
//		cam->_impl_trans.pos,
//		ham_vec3_add(cam->_impl_trans.pos, cam->_impl_forward),
//		ham_make_vec3(0.f, 1.f, 0.f)
//	);

	mut_ptr->_impl_trans._impl_trans = ham_mat4_mul(ham_mat4_inverse(ham_mat4_from_quat(viewq)), trans_m);
	mut_ptr->_impl_trans._impl_dirty = false;

#ifdef __cplusplus
#	undef mut_ptr
#endif
}

//! @endcond

/**
 * @brief Set a camera to a left-handed, zero-to-one depth perspective projection.
 * @param cam camera to set
 * @param aspect width to height ratio
 * @param fov_y vertical field of view
 * @param near_z near clip plane distance
 * @param far_z far clip plane distance
 */
ham_nonnull_args(1)
ham_constexpr ham_nothrow static inline void ham_camera_set_perspective(ham_camera *cam, ham_f32 aspect, ham_f32 fov_y, ham_f32 near_z, ham_f32 far_z){
	const ham_f32 tan_fov_2 = tan(fov_y * 0.5f);

	cam->kind = HAM_CAMERA_PERSPECTIVE;

	cam->near_z = near_z;
	cam->far_z  = far_z;

	cam->proj = (ham_mat4){
		.data = {
			// column 0
			1.f / (aspect * tan_fov_2),
			0.f,
			0.f,
			0.f,

			// column 1
			0.f,
			1.f / tan_fov_2,
			0.f,
			0.f,

			// column 2
			0.f,
			0.f,
			far_z / (far_z - near_z),
			1.f,

			// column 3
			0.f,
			0.f,
			-(near_z * far_z) / (far_z - near_z),
			0.f,
		}
	};
}

/**
 * @brief Set a camera to a left-handed, zero-to-one reversed depth perspective projection.
 * @param cam camera to set
 * @param aspect width to height ratio
 * @param fov_y vertical field of view
 * @param near_z near clip plane distance
 * @param far_z far clip plane distance
 */
ham_nonnull_args(1)
ham_constexpr ham_nothrow static inline void ham_camera_set_perspective_rev(ham_camera *cam, ham_f32 aspect, ham_f32 fov_y, ham_f32 near_z, ham_f32 far_z){
	const ham_f32 tan_fov_2 = tan(fov_y * 0.5f);

	cam->kind = HAM_CAMERA_PERSPECTIVE_REV;

	cam->near_z = near_z;
	cam->far_z  = far_z;

	cam->proj = (ham_mat4){
		.data = {
			// column 0
			1.f / (aspect * tan_fov_2),
			0.f,
			0.f,
			0.f,

			// column 1
			0.f,
			1.f / tan_fov_2,
			0.f,
			0.f,

			// column 2
			0.f,
			0.f,
			-near_z / (far_z - near_z),
			1.f,

			// column 3
			0.f,
			0.f,
			(near_z * far_z) / (far_z - near_z),
			0.f,
		}
	};
}

/**
 * @brief Set a camera to a left-handed, zero-to-one depth orthographic projection.
 * @param cam camera to set
 * @param top top y coord
 * @param bottom bottom y coord
 * @param left far left x coord
 * @param right far right x coord
 * @param near_z near clip plane distance
 * @param far_z far clip plane distance
 */
ham_nonnull_args(1)
ham_constexpr ham_nothrow static inline void ham_camera_set_orthographic(ham_camera *cam, ham_f32 top, ham_f32 bottom, ham_f32 left, ham_f32 right, ham_f32 near_z, ham_f32 far_z){
	cam->kind = HAM_CAMERA_ORTHOGRAPHIC;

	cam->near_z = near_z;
	cam->far_z  = far_z;

	cam->proj = ham_mat4_from_f32(1.f);

	cam->proj.data[0]  = 2.f / (right - left);
	cam->proj.data[5]  = 2.f / (top - bottom);
	cam->proj.data[10] = 1.f / (far_z - near_z);

	cam->proj.data[12] = -(right + left) / (right - left);
	cam->proj.data[13] = -(top + bottom) / (top - bottom);
	cam->proj.data[14] = -near_z / (far_z - near_z);
}

ham_nonnull_args(1)
ham_constexpr ham_nothrow static inline ham_mat4 ham_camera_proj_matrix(const ham_camera *cam){
	return cam->proj;
}

ham_nonnull_args(1)
ham_constexpr ham_nothrow static inline ham_mat4 ham_camera_view_matrix(const ham_camera *cam){
	ham_impl_camera_update(cam);
	return cam->_impl_trans._impl_trans;
}

ham_nonnull_args(1)
ham_constexpr ham_nothrow static inline ham_f32 ham_camera_near_z(const ham_camera *cam){ return cam->near_z; }

ham_nonnull_args(1)
ham_constexpr ham_nothrow static inline ham_f32 ham_camera_far_z(const ham_camera *cam){ return cam->far_z; }

ham_nonnull_args(1)
ham_constexpr ham_nothrow static inline void ham_camera_rotate(ham_camera *cam, ham_f32 rads, ham_vec3 axis){
	ham_transform_rotate(&cam->_impl_trans, rads, axis);

	ham_constexpr const ham_f32 max_angle = M_PI_2 - 0.0001f;

	// pitch constraint
	if(cam->_impl_trans.pyr.x >= max_angle){
		cam->_impl_trans.pyr.x = max_angle;
	}
	else if(cam->_impl_trans.pyr.x <= -max_angle){
		cam->_impl_trans.pyr.x = -max_angle;
	}

	// roll constraint
	if(cam->_impl_trans.pyr.z >= max_angle){
		cam->_impl_trans.pyr.z = max_angle;
	}
	else if(cam->_impl_trans.pyr.z <= -max_angle){
		cam->_impl_trans.pyr.z = -max_angle;
	}
}

ham_nonnull_args(1)
ham_constexpr ham_nothrow static inline void ham_camera_translate(ham_camera *cam, ham_vec3 amnt){
	ham_transform_translate(&cam->_impl_trans, amnt);
}

ham_nonnull_args(1)
ham_constexpr ham_nothrow static inline void ham_camera_set_rotation(ham_camera *cam, ham_vec3 pyr){
	ham_transform_set_rotation(&cam->_impl_trans, pyr);
}

ham_nonnull_args(1)
ham_constexpr ham_nothrow static inline void ham_camera_set_position(ham_camera *cam, ham_vec3 pos){
	ham_transform_set_position(&cam->_impl_trans, pos);
}

ham_nonnull_args(1)
ham_constexpr ham_nothrow static inline ham_vec3 ham_camera_position(const ham_camera *cam){
	return ham_transform_position(&cam->_impl_trans);
}

ham_nonnull_args(1)
ham_constexpr ham_nothrow static inline ham_quat ham_camera_rotation(const ham_camera *cam){
	ham_impl_camera_update(cam);
	return cam->_impl_trans._impl_rot;
}

ham_nonnull_args(1)
ham_constexpr ham_nothrow static inline ham_vec3 ham_camera_forward(const ham_camera *cam){
	ham_impl_camera_update(cam);
	return cam->_impl_forward;
}

ham_nonnull_args(1)
ham_constexpr ham_nothrow static inline ham_vec3 ham_camera_right(const ham_camera *cam){
	ham_impl_camera_update(cam);
	return cam->_impl_right;
}

ham_nonnull_args(1)
ham_constexpr ham_nothrow static inline ham_vec3 ham_camera_up(const ham_camera *cam){
	ham_impl_camera_update(cam);
	return cam->_impl_up;
}

#ifdef __cplusplus

namespace ham{
	template<typename ... Tags>
	class basic_camera_view{
		public:
			constexpr static bool is_mutable = meta::type_list_contains_v<meta::type_list<Tags...>, mutable_tag>;

			using pointer = std::conditional_t<is_mutable, ham_camera*, const ham_camera*>;
			using reference = std::conditional_t<is_mutable, ham_camera&, const ham_camera&>;

			constexpr basic_camera_view(pointer ptr = nullptr) noexcept
				: m_ptr(ptr){}

			constexpr basic_camera_view(ham_camera &ref) noexcept
				: m_ptr(&ref){}

			constexpr reference operator*() const noexcept{ return *m_ptr; }

			constexpr operator pointer() const noexcept{ return m_ptr; }

			constexpr pointer ptr() const noexcept{ return m_ptr; }

			constexpr operator basic_camera_view<>() const noexcept requires is_mutable{ return m_ptr; }

			constexpr void reset_transform() noexcept requires is_mutable{
				ham_transform_reset(&m_ptr->trans);
			}

			constexpr void set_perspective(f32 aspect, f32 fov_y, f32 near_z, f32 far_z) noexcept requires is_mutable{
				ham_camera_set_perspective(m_ptr, aspect, fov_y, near_z, far_z);
			}

			constexpr void set_orthographic(f32 top, f32 bottom, f32 left, f32 right, f32 near_z, f32 far_z) noexcept requires is_mutable{
				ham_camera_set_orthographic(m_ptr, top, bottom, left, right, near_z, far_z);
			}

			constexpr mat4 projection_matrix() const noexcept{ return ham_camera_proj_matrix(m_ptr); }
			constexpr mat4 view_matrix() const noexcept{ return ham_camera_view_matrix(m_ptr); }

			constexpr f32 near_z() const noexcept{ return ham_camera_near_z(m_ptr); }
			constexpr f32 far_z() const noexcept{ return ham_camera_far_z(m_ptr); }

			constexpr vec3 forward() const noexcept{ return ham_camera_forward(m_ptr); }
			constexpr vec3 right() const noexcept{ return ham_camera_right(m_ptr); }
			constexpr vec3 up() const noexcept{ return ham_camera_up(m_ptr); }

			constexpr vec3 position() const noexcept{ return ham_camera_position(m_ptr); }
			constexpr quat rotation() const noexcept{ return ham_camera_rotation(m_ptr); }

			constexpr void translate(const vec3 &amnt) noexcept requires is_mutable{ ham_camera_translate(m_ptr, amnt); }
			constexpr void rotate(f32 angle, const vec3 &axis) noexcept requires is_mutable{ ham_camera_rotate(m_ptr, angle, axis); }

		private:
			pointer m_ptr;
	};

	using camera_view = basic_camera_view<mutable_tag>;
	using const_camera_view = basic_camera_view<>;

	class camera{
		public:
			constexpr camera() noexcept{
				ham_camera_init(&m_cam);
			}

			constexpr camera(ham_camera cam) noexcept
				: m_cam(cam){}

			static constexpr camera perspective(f32 aspect, f32 fov_y, f32 near_z, f32 far_z) noexcept{
				camera ret;
				ret.set_perspective(aspect, fov_y, near_z, far_z);
				return ret;
			}

			static constexpr camera orthographic(f32 top, f32 bottom, f32 left, f32 right, f32 near_z, f32 far_z) noexcept{
				camera ret;
				ret.set_orthographic(top, bottom, left, right, near_z, far_z);
				return ret;
			}

			constexpr operator ham_camera&() noexcept{ return m_cam; }
			constexpr operator const ham_camera&() const noexcept{ return m_cam; }

			constexpr operator camera_view() noexcept{ return &m_cam; }
			constexpr operator const_camera_view() const noexcept{ return &m_cam; }

			constexpr ham_camera *ptr() noexcept{ return &m_cam; }
			constexpr const ham_camera *ptr() const noexcept{ return &m_cam; }

			constexpr void reset_transform() noexcept{ ham_camera_reset_transform(&m_cam); }

			constexpr void set_perspective(f32 aspect, f32 fov_y, f32 near_z, f32 far_z) noexcept{
				ham_camera_set_perspective(&m_cam, aspect, fov_y, near_z, far_z);
			}

			constexpr void set_perspective_rev(f32 aspect, f32 fov_y, f32 near_z, f32 far_z) noexcept{
				ham_camera_set_perspective_rev(&m_cam, aspect, fov_y, near_z, far_z);
			}

			constexpr void set_orthographic(f32 top, f32 bottom, f32 left, f32 right, f32 near_z, f32 far_z) noexcept{
				ham_camera_set_orthographic(&m_cam, top, bottom, left, right, near_z, far_z);
			}

			constexpr mat4 projection_matrix() const noexcept{ return ham_camera_proj_matrix(&m_cam); }
			constexpr mat4 view_matrix() const noexcept{ return ham_camera_view_matrix(&m_cam); }

			constexpr f32 near_z() const noexcept{ return ham_camera_near_z(&m_cam); }
			constexpr f32 far_z() const noexcept{ return ham_camera_far_z(&m_cam); }

			constexpr vec3 forward() const noexcept{ return ham_camera_forward(&m_cam); }
			constexpr vec3 right() const noexcept{ return ham_camera_right(&m_cam); }
			constexpr vec3 up() const noexcept{ return ham_camera_up(&m_cam); }

			constexpr vec3 position() const noexcept{ return ham_camera_position(&m_cam); }
			constexpr quat rotation() const noexcept{ return ham_camera_rotation(&m_cam); }

			constexpr void translate(const vec3 &amnt) noexcept{ ham_camera_translate(&m_cam, amnt); }
			constexpr void rotate(f32 angle, const vec3 &axis) noexcept{ ham_camera_rotate(&m_cam, angle, axis); }

		private:
			ham_camera m_cam;
	};
}

#endif // __cplusplus

/**
 * @}
 */

#endif // !HAM_CAMERA_H
