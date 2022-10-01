/*
 * Ham World Engine Runtime
 * Copyright (C) 2022  Hamsmith Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "ham/check.h"
#include "ham/async.h"
#include "ham/engine/world.h"

#include "robin_hood.h"

using namespace ham::typedefs;

HAM_C_API_BEGIN

struct ham_world{
	ham::mutex mut;
	robin_hood::unordered_flat_map<const ham_entity_vtable*, ham_object_manager*> obj_mans;
};

ham_entity *ham_entity_vcreate(
	ham_world *world, const ham_entity_vtable *entity_vt,
	ham_usize nargs, va_list va
){
	if(!ham_check(world != NULL) || !ham_check(entity_vt != NULL)){
		return nullptr;
	}

	ham_object_manager *man = nullptr;

	{
		ham::scoped_lock lock(world->mut);

		const auto man_res = world->obj_mans.find(entity_vt);
		if(man_res == world->obj_mans.end()){
			// TODO: create object manager
			man = ham_object_manager_create(ham_super(entity_vt));
			if(!man){
				ham_logapierrorf("Failed to create object manager for \"%s\"", ham_super(entity_vt)->info()->type_id);
				return nullptr;
			}

			const auto emplace_res = world->obj_mans.try_emplace(entity_vt, man);
			if(!emplace_res.second){
				ham_logapierrorf("Failed to emplace object manager into ham_world");
				ham_object_manager_destroy(man);
				return nullptr;
			}
		}
		else{
			man = man_res->second;
		}
	}

	const auto obj = ham_object_vnew(man, nargs, va);
	if(!obj){
		ham_logapierrorf("Failed to create new entity of type \"%s\"", ham_super(entity_vt)->info()->type_id);
		return nullptr;
	}

	return (ham_entity*)obj;
}

void ham_entity_destroy(ham_entity *ent){
	if(ham_unlikely(ent == NULL)) return;

	const auto world = ent->world;

	const auto vtable = (const ham_entity_vtable*)ham_super(ent)->vtable;

	ham_object_manager *man;

	{
		ham::scoped_lock lock(world->mut);
		man = world->obj_mans[vtable];
	}

	ham_object_delete(man, ham_super(ent));
}

HAM_C_API_END
