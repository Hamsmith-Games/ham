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
	ham_world *world;
	ham_f64 length;
	ham_world_partition *facing[HAM_WORLD_PARTITION_FACE_COUNT];
	ham_entity **ents;
};

struct ham_world{
	const ham_allocator *allocator;

	ham::str_buffer_utf8 name;

	ham::mutex mut;
	robin_hood::unordered_flat_map<const ham_entity_vtable*, ham_object_manager*> obj_mans;
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

	// TODO: setup communication over socket

	return ret;
}

void ham_world_destroy(ham_world *world){
	if(ham_unlikely(!world)) return;

	const auto allocator = world->allocator;

	ham_allocator_delete(allocator, world);
}

HAM_C_API_END
