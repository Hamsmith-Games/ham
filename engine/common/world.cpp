/*
 * Ham World Engine Runtime
 * Copyright (C) 2022 Hamsmith Ltd.
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

//
// World management
//

typedef enum ham_world_partition_face{
	HAM_WORLD_PARTITION_FRONT,
	HAM_WORLD_PARTITION_BACK,
	HAM_WORLD_PARTITION_LEFT,
	HAM_WORLD_PARTITION_RIGHT,
	HAM_WORLD_PARTITION_TOP,
	HAM_WORLD_PARTITION_BOTTOM,

	HAM_WORLD_PARTITION_FACE_COUNT
} ham_world_partition_face;

struct ham_world_partition{
	ham::recursive_mutex mut;
	ham_world *world;
	ham_f64 length;
	ham_world_partition *facing[HAM_WORLD_PARTITION_FACE_COUNT];
	ham::basic_buffer<ham_entity*> ents;
};

struct ham_world{
	const ham_allocator *allocator;

	ham::str_buffer_utf8 name;

	ham::mutex mut;
	robin_hood::unordered_flat_map<const ham_entity_vtable*, ham_object_manager*> obj_mans;

	ham_world_partition root_partition;
};

ham_world *ham_world_create(ham_str8 name){
	if(!ham_check(name.len > 0) || !ham_check(name.ptr != NULL)) return nullptr;

	const auto allocator = ham_current_allocator();

	const auto ret = ham_allocator_new(allocator, ham_world);
	if(!ret){
		ham::logapierror("Error allocating ham_world");
		return nullptr;
	}

	ret->allocator = allocator;

	ret->name = name;

	ret->root_partition.world = ret;
	ret->root_partition = (ham_world_partition){
		.world  = ret,
		.length = 1024.f,
		.facing = { nullptr },
		.ents   = ham::basic_buffer<ham_entity*>()
	};

	// TODO: setup communication over socket

	return ret;
}

void ham_world_destroy(ham_world *world){
	if(ham_unlikely(!world)) return;

	const auto allocator = world->allocator;

	{
		ham::scoped_lock lock(world->mut, world->root_partition.mut);

		for(auto &&obj_man_p : world->obj_mans){
			ham_object_manager_iterate(
				obj_man_p.second,
				[](ham_object *obj, void*) -> bool{
					const auto ent = (ham_entity*)obj;
					ham_entity_destroy(ent);
					return true;
				},
				nullptr
			);

			ham_object_manager_destroy(obj_man_p.second);
		}
	}

	ham_allocator_delete(allocator, world);
}

ham_entity *ham_entity_vcreate(ham_world *world, const ham_entity_vtable *ent_vt, ham_u32 nargs, va_list va){
	if(!ham_check(world != NULL) || !ham_check(ent_vt != NULL)){
		return nullptr;
	}

	constexpr auto create_from_man = [](ham_world *world, const ham_entity_vtable *ent_vt, ham_object_manager *man, ham_u32 nargs, va_list va) -> ham_entity*{
		const auto obj = ham_object_vnew_init(
			man,
			[](ham_object *obj, void *user) -> bool{
				const auto world = (ham_world*)user;
				const auto ent = (ham_entity*)obj;

				ham_transform_reset(&ent->transform);

				ent->world_partition = &world->root_partition;
				ent->parent = nullptr;

				if(!ham_buffer_init_allocator(&ent->children, world->allocator, alignof(ham_entity*), sizeof(ham_entity*) * 8)){
					return false;
				}

				if(!ham_buffer_init_allocator(&ent->components, world->allocator, alignof(ham_entity_component*), sizeof(ham_entity_component*) * 8)){
					ham_buffer_finish(&ent->children);
					return false;
				}

				return true;
			}, world,
			nargs, va
		);

		// TODO: fix leak here when object fails to be constructed but buffers aren't finalized

		if(!obj){
			ham::logerror("ham_entity_vcreate", "Failed to create entity of type '{}'", ham_super(ent_vt)->info->type_id);
		}

		return (ham_entity*)obj;
	};

	ham_object_manager *man;

	{
		ham::scoped_lock lock(world->mut);

		const auto man_res = world->obj_mans.find(ent_vt);
		if(man_res != world->obj_mans.end()){
			man = man_res->second;
		}
		else{
			man = ham_object_manager_create(ham_super(ent_vt));
			if(!man){
				ham::logapierror("Failed to create object manager for entity of type '{}'", ham_super(ent_vt)->info->type_id);
				return nullptr;
			}

			const auto emplace_res = world->obj_mans.try_emplace(ent_vt, man);
			if(!emplace_res.second){
				ham::logapierror("Failed to emplace object manager for entity of type '{}'", ham_super(ent_vt)->info->type_id);
				ham_object_manager_destroy(man);
				return nullptr;
			}
		}
	}

	const auto ret = create_from_man(world, ent_vt, man, nargs, va);

	{
		ham::scoped_lock lock(world->root_partition.mut);

		auto &ents_buf = world->root_partition.ents;

		const auto insert_res = std::lower_bound(ents_buf.begin(), ents_buf.end(), ret);
		if(!ents_buf.insert((uptr)(ents_buf.end() - insert_res), ret)){
			ham_entity_destroy(ret);
			return nullptr;
		}
	}

	return ret;
}

void ham_entity_destroy(ham_entity *ent){
	if(ham_unlikely(!ent)) return;

	ham::scoped_lock lock(ent->world_partition->mut);

	if(ent->parent){
		const auto num_children = ham_buffer_size(&ent->parent->children)/sizeof(ham_entity*);
		const auto children = (ham_entity**)ham_buffer_data(&ent->parent->children);
		const auto children_end = children + num_children;

		const auto parent_it = std::find(children, children_end, ent);

		if(parent_it != children_end){
			ham_buffer_erase(&ent->parent->children, (uptr)(parent_it - children), sizeof(ham_entity*));
		}
	}

	const auto partition = ent->world_partition;
	const auto world = partition->world;

	const auto ent_it = std::lower_bound(partition->ents.begin(), partition->ents.end(), ent);
	if(ent_it != partition->ents.end() && *ent_it == ent){
		partition->ents.erase((uptr)(partition->ents.end() - ent_it));
	}
	else{
		ham::logapiwarn("Entity could not be found within world partition");
	}

	const auto allocator = world->allocator;

	const auto num_children = ham_buffer_size(&ent->children) / sizeof(ham_entity*);
	for(usize i = 0; i < num_children; i++){
		const auto child = *((ham_entity**)ham_buffer_data(&ent->children) + i);
		ham_entity_destroy(child);
	}

	ham_buffer_finish(&ent->children);

	const auto num_comps = ham_buffer_size(&ent->components) / sizeof(ham_entity_component*);
	for(usize i = 0; i < num_comps; i++){
		const auto comp = *((ham_entity_component**)ham_buffer_data(&ent->components) + i);
		const auto comp_vptr = ham_super(comp)->vptr;
		comp_vptr->dtor(ham_super(comp));
		ham_allocator_free(world->allocator, comp);
	}

	ham_buffer_finish(&ent->components);

	const auto man_res = world->obj_mans.find((const ham_entity_vtable*)ham_super(ent)->vptr);
	if(man_res == world->obj_mans.end()){
		const auto vptr = ham_super(ent)->vptr;
		ham::logapiwarn("Could not find manager for entity of type '{}'", vptr->info->type_id);
		vptr->dtor(ham_super(ent));
		ham_allocator_free(allocator, ent);
	}
	else{
		ham_object_delete(man_res->second, ham_super(ent));
	}
}

HAM_C_API_END
