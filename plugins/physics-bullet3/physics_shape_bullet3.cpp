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

#include "ham/log.h"

#include "BulletCollision/CollisionShapes/btBoxShape.h"
#include "BulletCollision/CollisionShapes/btBox2dShape.h"
#include "BulletCollision/CollisionShapes/btConvexHullShape.h"
#include "BulletCollision/CollisionShapes/btCompoundShape.h"

#include "BulletDynamics/Dynamics/btDynamicsWorld.h"

#include "VHACD.h"

using namespace ham::typedefs;

HAM_C_API_BEGIN

static inline btCollisionShape *ham_physics_bullet3_new_shape(ham_physics_world_bullet3 *world, const ham_shape *shape){
	const auto allocator = ham_super(world)->phys->allocator;

	switch(ham_shape_get_kind(shape)){
		case HAM_SHAPE_CUBE:
		case HAM_SHAPE_CUBOID:{
			const auto shape_verts = ham_shape_vertices(shape);
			return ham_allocator_new(allocator, btBoxShape, {fabs(shape_verts[0].x), fabs(shape_verts[0].y), fabs(shape_verts[0].z)});
		}

		case HAM_SHAPE_HEXAHEDRON:{
			const auto shape_verts = ham_shape_vertices(shape);
			return ham_allocator_new(allocator, btConvexHullShape, (const btScalar*)shape_verts, ham_shape_num_points(shape), sizeof(ham_vec3));
		}

		case HAM_SHAPE_SQUARE:
		case HAM_SHAPE_RECT:{
			const auto shape_verts = ham_shape_vertices(shape);
			return ham_allocator_new(allocator, btBox2dShape, {fabs(shape_verts[0].x), fabs(shape_verts[0].y), 0.f});
		}

		case HAM_SHAPE_TRIANGLE_MESH:{
			static_assert(sizeof(ham_vec3) == sizeof(float) * 3);

			VHACD::IVHACD::Parameters params;

			const auto vhacd = VHACD::CreateVHACD_ASYNC();

			if(!vhacd->Compute(
				(const float*)ham_shape_vertices(shape),
				ham_shape_num_points(shape),
				ham_shape_indices(shape),
				ham_shape_num_indices(shape) / 3,
				params
			)){
				ham_logapierrorf("Error in IVHACD::Compute");
				return nullptr;
			}

			// TODO: not spinlock here
			while(!vhacd->IsReady()){
				ham_sleep(ham_duration_from_milli64(10));
			}

			const u32 n_hulls = vhacd->GetNConvexHulls();

			btAlignedObjectArray<btCollisionShape*> child_shapes;
			child_shapes.reserve(n_hulls);

			const auto compound_shape = ham_allocator_new(allocator, btCompoundShape);

			for(u32 i = 0; i < n_hulls; i++){
				VHACD::IVHACD::ConvexHull hull;
				if(!vhacd->GetConvexHull(i, hull)){
					ham_logapierrorf("Error getting hull %u of VHACD shape decomposition", i);
					continue;
				}

				btAlignedObjectArray<btVector3> verts;
				verts.reserve(hull.m_points.size());

				for(u32 j = 0; j < hull.m_points.size(); j++){
					const auto &point = hull.m_points[j];
					verts.push_back(btVector3{ (float)point.mX, (float)point.mY, (float)point.mZ });
				}

				btTransform hull_trans;
				hull_trans.setIdentity();
				hull_trans.setOrigin(btVector3(hull.m_center.GetX(), hull.m_center.GetY(), hull.m_center.GetZ()));

				const auto hull_shape = ham_allocator_new(allocator, btConvexHullShape, (const btScalar*)&verts[0], verts.size(), sizeof(btVector3));

				compound_shape->addChildShape(hull_trans, hull_shape);
			}

			vhacd->Release();

			return compound_shape;
		}

		default:{
			ham_logapierrorf("Unrecognized shape kind");
			return nullptr;
		}
	}
}

ham_def_ctor(ham_physics_shape_bullet3, nargs, va){
	if(nargs != 4){
		ham_logapierrorf("Expected 2 args (num_shapes: u32, shapes: const ham_shape**, offsets: const ham_vec3*, orientations: const ham_quat*)");
		return nullptr;
	}

	const auto phys_world = (ham_physics_world_bullet3*)ham_super(self)->phys_world;

	const auto num_shapes = va_arg(va, ham_u32);
	const auto shapes = va_arg(va, const ham_shape*const*);
	const auto offsets = va_arg(va, const ham_vec3*);
	const auto orientations = va_arg(va, const ham_quat*);

	if(num_shapes > 1){
		const auto allocator = ham_super(phys_world)->phys->allocator;

		const auto compound_shape = ham_allocator_new(allocator, btCompoundShape);

		for(u32 i = 0; i < num_shapes; i++){
			const auto child_shape = ham_physics_bullet3_new_shape(phys_world, shapes[i]);

			btTransform child_trans;
			child_trans.setIdentity();
			child_trans.setOrigin(ham_to_b2(offsets[i]));
			child_trans.setRotation(ham_to_b2(orientations[i]));

			compound_shape->addChildShape(child_trans, child_shape);
		}

		self->shape = compound_shape;
	}
	else{
		self->shape = ham_physics_bullet3_new_shape(phys_world, shapes[0]);
	}

	return self;
}

static inline void ham_physics_bullet3_delete_shape(const ham_allocator *allocator, btCollisionShape *shape){
	const auto compound_shape = dynamic_cast<btCompoundShape*>(shape);
	if(compound_shape){
		const int num_children = compound_shape->getNumChildShapes();

		btAlignedObjectArray<btCollisionShape*> child_shapes;
		child_shapes.reserve(num_children);

		for(int i = 0; i < num_children; i++){
			const auto child_shape = compound_shape->getChildShape(i);
			child_shapes.push_back(child_shape);
		}

		ham_allocator_delete(allocator, shape);

		for(int i = 0; i < num_children; i++){
			ham_physics_bullet3_delete_shape(allocator, child_shapes[i]);
		}
	}
	else{
		ham_allocator_delete(allocator, shape);
	}
}

ham_def_dtor(ham_physics_shape_bullet3){
	const auto allocator = ham_super(self)->phys_world->phys->allocator;
	ham_physics_bullet3_delete_shape(allocator, self->shape);
}

ham_def_physics_shape_object(ham_physics_shape_bullet3)

HAM_C_API_END
