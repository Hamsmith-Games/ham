/*
 * Ham Runtime Plugins
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

#ifndef HAM_PHYSICS_BULLET3_H
#define HAM_PHYSICS_BULLET3_H 1

#include "ham/physics-object.h" // IWYU pragma: keep

#include "Bullet3OpenCL/Initialize/b3OpenCLInclude.h" // IWYU pragma: keep

#include "LinearMath/btVector3.h"
#include "LinearMath/btQuaternion.h"

#include "VHACD.h"

class btBroadphaseInterface;
class btCollisionShape;
class btCollisionDispatcher;
class btConstraintSolver;
class btSoftRigidDynamicsWorld;
class btSoftBodySolver;

ham_used
ham_nothrow static inline btVector3 ham_to_b2(const ham_vec3 &vec) noexcept{ return { vec.x, vec.y, vec.z }; }

ham_used
ham_nothrow static inline btQuaternion ham_to_b2(const ham_quat &q) noexcept{ return { q.x, q.y, q.z, q.w }; }

HAM_C_API_BEGIN

struct ham_physics_bullet3{
	ham_derive(ham_physics)

	bool use_gpu;
	cl_context cl_ctx;
	cl_device_id cl_dev_id;
	cl_command_queue cl_queue;
};

struct ham_physics_world_bullet3{
	ham_derive(ham_physics_world)

	VHACD::IVHACD *vhacd;

	bool use_gpu;

	btBroadphaseInterface *broadphase;
	btCollisionDispatcher *dispatcher;
	btConstraintSolver *solver;
	btSoftBodySolver *soft_solver;
	btSoftRigidDynamicsWorld *dynamics;
};

struct ham_physics_shape_bullet3{
	ham_derive(ham_physics_shape)

	btCollisionShape *shape;
	int shape_id;
};

struct ham_physics_body_bullet3{
	ham_derive(ham_physics_body)
};

HAM_C_API_END

#endif // !HAM_PHYSICS_BULLET3_H
