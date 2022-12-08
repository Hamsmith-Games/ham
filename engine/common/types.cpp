/*
 * Ham World Engine Runtime
 * Copyright (C) 2022 Keith Hammond
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

#include "ham/engine/types.h"
#include "ham/engine/graph.h"
#include "ham/engine/entity.h"

using namespace ham::typedefs;

HAM_C_API_BEGIN

static inline const ham_type *ham_impl_ensure_entity(ham::typeset_view ts){
	constexpr ham::str8 ent_name = ham::meta::type_name_v<ham_entity>;

	const auto existing = ts.get_object(ent_name);
	if(existing){
		return existing;
	}

	ham::type_builder ent_build;

	ent_build.set_parent(ham::get_type<ham_object>(ts));
	ent_build.set_name(ent_name);

	/*
	ham_transform transform;

	//! @brief World partition this entity belongs to.
	ham_world_partition *world_partition;

	//! @brief Parent of this entity.
	ham_entity *parent;

	//! @brief Buffer of `ham_entity*`.
	ham_buffer children;

	//! @brief Buffer of `ham_entity_component*`.
	ham_buffer components;
	*/

	ent_build.add_member("transform", ham::get_type<ham_transform>(ts));
	ent_build.add_member("world_partition", ham::get_type<ham_world_partition*>(ts));
	ent_build.add_member("parent", ham::get_type<void*>(ts));
	ent_build.add_member("children", ham::get_type<ham_buffer>(ts));
	ent_build.add_member("components", ham::get_type<ham_buffer>(ts));

	return ent_build.instantiate(ts);
}

bool ham_engine_ensure_types(ham_typeset *ts){
	if(
		!ham_check(ham_graph_exec_type(ts) != nullptr)
	){
		return false;
	}



	return true;
}

HAM_C_API_END
