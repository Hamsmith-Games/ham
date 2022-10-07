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

#include "ham/object.h"
#include "ham/check.h"
#include "ham/colony.h"
#include "ham/memory.h"

#include <stdexcept>
#include <mutex>
#include <algorithm>

HAM_C_API_BEGIN

struct ham_object_manager{
	const ham_allocator *allocator;
	const ham_object_vtable *obj_vtable;
	const ham_object_info *obj_info;

	ham_colony *instances;
};

ham_object_manager *ham_object_manager_create(const ham_object_vtable *vtable){
	if(!ham_check(vtable != NULL)) return nullptr;

	const auto allocator = ham::allocator<ham_object_manager>();

	const auto mem = allocator.allocate(1);
	if(!mem){
		ham_logapierrorf("Out of memory");
		return nullptr;
	}

	const auto ptr = allocator.construct(mem);
	if(!ptr){
		ham_logapierrorf("Error constructing ham_object_manager");
		allocator.deallocate(mem);
		return nullptr;
	}

	ptr->allocator = allocator;
	ptr->obj_vtable = vtable;
	ptr->obj_info = vtable->info();

	ptr->instances = ham_colony_create(ptr->obj_info->alignment, ptr->obj_info->size);
	if(!ptr->instances){
		ham_logapierrorf("Error in ham_colony_create");
		allocator.destroy(ptr);
		allocator.deallocate(mem);
		return nullptr;
	}

	return ptr;
}

void ham_object_manager_destroy(ham_object_manager *manager){
	if(ham_unlikely(manager == NULL)) return;

	const auto allocator = ham::allocator<ham_object_manager>(manager->allocator);

	ham_colony_iterate(
		manager->instances,
		[](void *mem, void *user) -> bool{
			const auto man = (ham_object_manager*)user;
			const auto ptr = (ham_object*)mem;
			man->obj_vtable->dtor(ptr);
			return true;
		},
		manager
	);

	ham_colony_destroy(manager->instances);

	allocator.destroy(manager);
	allocator.deallocate(manager);
}

ham_usize ham_object_manager_iterate(ham_object_manager *manager, ham_object_manager_iterate_fn fn, void *user){
	if(!ham_check(manager != NULL)){
		return (ham_usize)-1;
	}

	if(fn){
		struct colony_iter_data{
			ham_object_manager_iterate_fn fn;
			void *user;
		} data{ fn, user };

		return ham_colony_iterate(
			manager->instances,
			[](void *mem, void *user){
				const auto obj  = (ham_object*)mem;
				const auto data = (colony_iter_data*)user;
				return data->fn(obj, data->user);
			},
			&data
		);
	}
	else{
		return ham_colony_iterate(manager->instances, nullptr, nullptr);
	}
}

bool ham_object_manager_contains(const ham_object_manager *manager, const ham_object *obj){
	if(!ham_check(manager != NULL) || !obj){
		return false;
	}

	return ham_colony_contains(manager->instances, obj);
}

ham_api ham_object *ham_object_vnew_init(
	ham_object_manager *manager,
	ham_object_manager_iterate_fn init_fn, void *user,
	ham_usize nargs, va_list va
){
	if(!ham_check(manager != NULL)) return nullptr;

	const auto mem = (ham_object*)ham_colony_emplace(manager->instances);
	if(!mem){
		ham_logapierrorf("Failed to emplace new object in storage");
		return nullptr;
	}

	const auto vtable = manager->obj_vtable;

	mem->vtable = vtable;

	if(init_fn && !init_fn(mem, user)){
		ham_logapierrorf("Failed to initialize object memory");
		ham_colony_erase(manager->instances, mem);
		return nullptr;
	}

	const auto ret = vtable->ctor(mem, nargs, va);
	if(!ret){
		ham_logapierrorf("Failed to construct object");
		ham_colony_erase(manager->instances, mem);
		return nullptr;
	}

	ret->vtable = vtable; // make sure no funny business happens

	return ret;
}

bool ham_object_delete(ham_object_manager *manager, ham_object *obj){
	if(!ham_check(manager != NULL) || !ham_check(obj != NULL)){
		return false;
	}

	return ham_colony_view_erase(
		manager->instances, obj,
		[](void *mem, void *user) -> bool{
			const auto man = (ham_object_manager*)user;
			const auto ptr = (ham_object*)mem;
			man->obj_vtable->dtor(ptr);
			return true;
		},
		manager
	);
}

HAM_C_API_END
