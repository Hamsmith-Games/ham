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

#include "ham/physics-object.h"
#include "ham/check.h"

using namespace ham::typedefs;

HAM_C_API_BEGIN

//
// Context management
//

static inline uptr ham_physics_thread_routine(void *user){
	const auto phys = reinterpret_cast<ham_physics*>(user);
	(void)phys;
	return 0;
}

ham_physics *ham_physics_create_alloc(const ham_allocator *allocator, ham_physics_vptr vptr){
	if(!ham_check(vptr != NULL)) return nullptr;

	const auto obj_vptr = ham_super(vptr);
	const auto obj_info = obj_vptr->info;

	const auto obj = (ham_object*)ham_allocator_alloc(allocator, obj_info->alignment, obj_info->size);
	if(!obj){
		return nullptr;
	}

	const auto world_man = ham_object_manager_create(ham_super(vptr->physics_world_vptr()));

	const auto phys = (ham_physics*)obj;
	phys->allocator = allocator;
	phys->world_man = world_man;

	ham_workgroup_init(&phys->workgroup);

	if(!ham_construct_object(obj, obj_vptr)){
		ham_logapierrorf("Error constructing '%s'", obj_info->type_id);
		ham_allocator_free(allocator, obj);
		return nullptr;
	}

#ifdef HAM_DEBUG
	if(phys->allocator != allocator || phys->world_man != world_man){
		ham_logapierrorf("Base ham_physics object corrupted in construction of '%s'", obj_info->type_id);
		ham_destroy_object(obj);
		ham_allocator_free(allocator, obj);
		return nullptr;
	}
#endif

	return phys;
}

ham_nothrow void ham_physics_destroy(ham_physics *phys){
	if(!phys) return;

	const auto allocator = phys->allocator;

	ham_workgroup_finish(&phys->workgroup);

	ham_object_manager_destroy(phys->world_man);

	ham_destroy_object(ham_super(phys));
	ham_allocator_free(allocator, phys);
}

//
// Physics worlds
//

struct world_create_args{
	ham_physics *phys;
	ham_async_result *result;
};

ham_physics_world *ham_physics_world_create(ham_physics *phys){
	if(!ham_check(phys != NULL)) return nullptr;

	const auto phys_vptr = (ham_physics_vptr)ham_super(phys)->vptr;
	const auto shape_vptr = phys_vptr->physics_shape_vptr();
	const auto shape_man = ham_object_manager_create(ham_super(shape_vptr));

	struct {
		ham_physics *phys;
		ham_object_manager *shape_man;
	} arg_data{ phys, shape_man };

	const auto obj = ham_object_new_init(
		phys->world_man,
		[](ham_object *obj, void *user){
			const auto args = reinterpret_cast<decltype(arg_data)*>(user);
			const auto phys_world = reinterpret_cast<ham_physics_world*>(obj);
			phys_world->phys = args->phys;
			phys_world->shape_man = args->shape_man;
			return true;
		},
		&arg_data
	);
	if(!obj){
		return nullptr;
	}

	const auto phys_world = (ham_physics_world*)obj;

#ifdef HAM_DEBUG
	if(phys_world->phys != phys || phys_world->shape_man != shape_man){
		ham_logapierrorf("Base ham_physics_world object corrupted in construction of '%s'", ham_super(shape_vptr)->info->type_id);
		ham_object_delete(phys->world_man, obj);
		return nullptr;
	}
#endif

	return phys_world;
}

static inline void ham_physics_world_create_async_fn(void *user){
	const auto args = reinterpret_cast<world_create_args*>(user);
	const auto allocator = args->phys->allocator;

	const auto ret = ham_physics_world_create(args->phys);
	if(!ret){
		ham_async_result_finish(args->result);
	}
	else{
		ham_async_result_set(args->result, ret);
	}

	ham_allocator_delete(allocator, args);
}

bool ham_physics_world_create_async(ham_physics *phys, ham_async_result *result){
	if(!ham_check(phys != NULL) || !ham_check(result != NULL)){
		return false;
	}

	const auto args = ham_allocator_new(phys->allocator, world_create_args);
	args->phys = phys;
	args->result = result;

	if(!ham_workgroup_push(&phys->workgroup, ham_physics_world_create_async_fn, args)){
		ham_logapierrorf("Error in ham_workgroup_push");
		return false;
	}

	return true;
}

ham_nothrow void ham_physics_world_destroy(ham_physics_world *phys_world){
	if(!phys_world) return;
	ham_object_delete(phys_world->phys->world_man, ham_super(phys_world));
}

void ham_physics_world_tick(ham_physics_world *phys_world, ham_f64 dt){
	if(!ham_check(phys_world != NULL) || !ham_check(dt >= 0.0)){
		return;
	}

	const auto vptr = (ham_physics_world_vptr)ham_super(phys_world)->vptr;
	vptr->tick(phys_world, dt);
}

//
// Physics shapes
//

ham_physics_shape *ham_physics_shape_create(ham_physics_world *phys_world, ham_usize num_shapes, const ham_shape *const *shapes, const ham_vec3 *offsets, const ham_quat *orientations){
	if(
	   !ham_check(phys_world != NULL) ||
	   !ham_check(num_shapes > 0) ||
	   !ham_check(shapes != NULL) ||
	   !ham_check(offsets != NULL) ||
	   !ham_check(orientations != NULL)
	){
		return nullptr;
	}

	const auto obj = ham_object_new(phys_world->shape_man, num_shapes, shapes, offsets, orientations);
	if(!obj){
		ham_logapierrorf("Error constructing ham_physics_shape object");
		return nullptr;
	}

	return (ham_physics_shape*)obj;
}

struct shape_create_args{
	ham_physics_world *phys_world;
	ham_usize num_shapes;
	const ham_shape *const *shapes;
	const ham_vec3 *offsets;
	const ham_quat *orientations;
	ham_async_result *result;
};

static inline void ham_physics_shape_create_async_fn(void *user){
	const auto args = reinterpret_cast<shape_create_args*>(user);
	const auto allocator = args->phys_world->phys->allocator;

	const auto ret = ham_physics_shape_create(args->phys_world, args->num_shapes, args->shapes, args->offsets, args->orientations);
	if(!ret){
		ham_logapierrorf("Error in ham_physics_shape_create");
		ham_async_result_finish(args->result);
	}
	else{
		ham_async_result_set(args->result, ret);
	}

	ham_allocator_delete(allocator, args);
}

bool ham_physics_shape_create_async(
	ham_physics_world *phys_world,
	ham_usize num_shapes, const ham_shape *const *shapes, const ham_vec3 *offsets, const ham_quat *orientations,
	ham_async_result *result
){
	if(!ham_check(result != NULL)){
		return false;
	}

	const auto args = ham_allocator_new(phys_world->phys->allocator, shape_create_args);
	args->phys_world   = phys_world;
	args->num_shapes   = num_shapes;
	args->shapes       = shapes;
	args->offsets      = offsets;
	args->orientations = orientations;
	args->result       = result;

	if(!ham_workgroup_push(&phys_world->phys->workgroup, ham_physics_shape_create_async_fn, args)){
		ham_logapierrorf("Error in ham_workgroup_push");
		ham_allocator_delete(phys_world->phys->allocator, args);
		return false;
	}

	return true;
}

static inline void ham_physics_shape_destroy_fn(void *user){
	const auto phys_shape = reinterpret_cast<ham_physics_shape*>(user);
	ham_object_delete(phys_shape->phys_world->shape_man, ham_super(phys_shape));
}

ham_nothrow void ham_physics_shape_destroy(ham_physics_shape *phys_shape){
	if(!phys_shape) return;
	ham_workgroup_push(&phys_shape->phys_world->phys->workgroup, ham_physics_shape_destroy_fn, phys_shape);
}

//
// Physics bodies
//

ham_physics_body *ham_physics_body_create(ham_physics_world *phys_world){
	ham_logapierrorf("UNIMPLEMENTED");
	return nullptr;
}

ham_nothrow void ham_physics_body_destroy(ham_physics_body *phys_body){
	if(!phys_body) return;
	ham_logapierrorf("UNIMPLEMENTED");
}

HAM_C_API_END
