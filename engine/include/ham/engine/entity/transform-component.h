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

/**
 * @defgroup HAM_ENGINE_ENTITY_TRANSFORM_COMPONENT Transform component
 * @ingroup HAM_ENGINE_ENTITY_COMPONENTS
 * @{
 */

#ifndef HAM_ENGINE_ENTITY_TRANSFORM_COMPONENT_H
#define HAM_ENGINE_ENTITY_TRANSFORM_COMPONENT_H 1

#include "../entity.h"

ham_declare_object(ham_entity_component_transform, ham_entity_component)

struct ham_entity_component_transform{
	ham_derive(ham_entity_component)

	ham_mat4 trans;
};

/**
 * @}
 */

#endif // !HAM_ENGINE_ENTITY_TRANSFORM_COMPONENT_H
