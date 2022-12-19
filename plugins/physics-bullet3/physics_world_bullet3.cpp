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

#include "physics_bullet3.h"

#include "BulletCollision/CollisionDispatch/btDefaultCollisionConfiguration.h"
#include "BulletCollision/BroadphaseCollision/btDbvtBroadphase.h"
#include "BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.h"
#include "BulletSoftBody/btSoftRigidDynamicsWorld.h"
#include "BulletSoftBody/btDefaultSoftBodySolver.h"

HAM_C_API_BEGIN

ham_def_ctor(ham_physics_world_bullet3, nargs, va){
	(void)nargs; (void)va;

	const auto phys = (ham_physics_bullet3*)ham_super(self)->phys;

	self->vhacd = VHACD::CreateVHACD_ASYNC();

	self->use_gpu = phys->use_gpu;

	btDefaultCollisionConfiguration collision_config;

	// TODO: use multithreaded versions

	self->broadphase  = new btDbvtBroadphase();
	self->dispatcher  = new btCollisionDispatcher(&collision_config);
	self->solver      = new btSequentialImpulseConstraintSolver();
	self->soft_solver = new btDefaultSoftBodySolver();
	self->dynamics    = new btSoftRigidDynamicsWorld(self->dispatcher, self->broadphase, self->solver, &collision_config, self->soft_solver);

	return self;
}

ham_def_dtor(ham_physics_world_bullet3){
	self->vhacd->Release();

	delete self->dynamics;
	delete self->dispatcher;
	delete self->broadphase;
	delete self->solver;
	delete self->soft_solver;
}

ham_def_physics_world_object(ham_physics_world_bullet3)

HAM_C_API_END
