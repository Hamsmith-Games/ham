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

#ifndef HAM_ENGINE_ENTITY_DRAW_INSTANCE_COMPONENT_H
#define HAM_ENGINE_ENTITY_DRAW_INSTANCE_COMPONENT_H 1

/**
 * @defgroup HAM_ENGINE_ENTITY_DRAW_INSTANCE_COMPONENT Draw instance component
 * @ingroup HAM_ENGINE_ENTITY_COMPONENTS
 * @{
 */

#include "../entity.h"

#include "ham/renderer.h"

HAM_C_API_BEGIN

ham_declare_object(ham_entity_component_draw_instance, ham_entity_component)

struct ham_entity_component_draw_instance{
	ham_derive(ham_entity_component)

	ham_u32 id;
	ham_draw_group *group;
};

struct ham_entity_component_draw_instance_vtable{
	ham_derive(ham_entity_component_vtable)

	// TODO: draw instance component functions
};

HAM_C_API_END

/**
 * @}
 */

#endif // !HAM_ENGINE_ENTITY_DRAW_INSTANCE_COMPONENT_H
