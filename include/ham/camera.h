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

#ifndef HAM_CAMERA_H
#define HAM_CAMERA_H 1

/**
 * @defgroup HAM_CAMERA Camera
 * @ingroup HAM
 * @{
 */

#include "typedefs.h"

typedef struct ham_camera{
	ham_mat4 proj, view;
	ham_vec3 pos, pyr;

	//! @cond ignore
	bool _ham_dirty;
	//! @endcond
} ham_camera;

ham_nonnull_args(1)
ham_api void ham_camera_reset_perspective(ham_camera *cam, ham_f32 aspect, ham_f32 fovy, ham_f32 minz, ham_f32 maxz);

ham_nonnull_args(1)
ham_api void ham_camera_reset_ortho(ham_camera *cam, ham_f32 top, ham_f32 bottom, ham_f32 left, ham_f32 right, ham_f32 minz, ham_f32 maxz);

ham_nonnull_args(1)
ham_constexpr ham_nothrow static inline const ham_mat4 *ham_camera_proj_matrix(const ham_camera *cam){
	return &cam->proj;
}

ham_nonnull_args(1)
ham_api const ham_mat4 *ham_camera_view_matrix(const ham_camera *cam);

ham_nonnull_args(1)
ham_constexpr ham_nothrow static inline void ham_camera_rotate(ham_camera *cam, ham_f32 rads, ham_vec3 axis){
	cam->pyr.x += axis.x * rads;
	cam->pyr.y += axis.y * rads;
	cam->pyr.z += axis.z * rads;
	cam->_ham_dirty = true;
}

ham_nonnull_args(1)
ham_constexpr ham_nothrow static inline void ham_camera_translate(ham_camera *cam, ham_vec3 amnt){
	cam->pos.x += amnt.x;
	cam->pos.y += amnt.y;
	cam->pos.z += amnt.z;
	cam->_ham_dirty = true;
}

ham_nonnull_args(1)
ham_constexpr ham_nothrow static inline void ham_camera_set_rotation(ham_camera *cam, ham_vec3 pyr){
	cam->pyr = pyr;
	cam->_ham_dirty = true;
}

ham_nonnull_args(1)
ham_constexpr ham_nothrow static inline void ham_camera_set_position(ham_camera *cam, ham_vec3 pos){
	cam->pos = pos;
	cam->_ham_dirty = true;
}

/**
 * @}
 */

#endif // !HAM_CAMERA_H
