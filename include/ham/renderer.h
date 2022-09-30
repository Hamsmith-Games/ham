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

#ifndef HAM_RENDERER_H
#define HAM_RENDERER_H 1

/**
 * @defgroup HAM_RENDERER Rendering
 * @ingroup HAM
 * @{
 */

#include "shape.h"

#include <stdarg.h>

HAM_C_API_BEGIN

typedef struct ham_renderer ham_renderer;

ham_api ham_renderer *ham_renderer_vcreate(const char *plugin_id, const char *obj_id, ham_usize nargs, va_list va);

//! @cond ignore
static inline ham_renderer *ham_impl_renderer_create(const char *plugin_id, const char *obj_id, ham_usize nargs, ...){
	va_list va;
	va_start(va, nargs);
	const ham_auto ret = ham_renderer_vcreate(plugin_id, obj_id, nargs, va);
	va_end(va);
	return ret;
}
//! @endcond

#define ham_renderer_create(plugin_id, obj_id, ...) (ham_impl_renderer_create((plugin_id), (obj_id), HAM_NARGS(__VA_ARGS__) __VA_OPT__(,) __VA_ARGS__))

ham_api void ham_renderer_destroy(ham_renderer *renderer);

ham_api void ham_renderer_loop(ham_renderer *renderer, ham_f64 dt);

/**
 * @defgroup HAM_RENDERER_TEXTURE Textures
 * @{
 */

typedef struct ham_texture ham_texture;

/**
 * @}
 */

/**
 * @defgroup HAM_RENDERER_DRAW_GROUP Draw groups
 * @{
 */

typedef struct ham_draw_group ham_draw_group;

// typedef struct ham_draw

ham_api ham_draw_group *ham_draw_group_create(
	ham_renderer *r,
	ham_usize num_shapes, const ham_shape *const *shapes
);

ham_api void ham_draw_group_destroy(ham_draw_group *group);

typedef bool(*ham_draw_group_instance_iterate_fn);

ham_api ham_usize ham_draw_group_instance_iterate(
	ham_draw_group *group,
	ham_draw_group_instance_iterate_fn fn,
	void *user
);

static inline ham_usize ham_draw_group_num_instances(ham_draw_group *group){
	return ham_draw_group_instance_iterate(group, ham_null, ham_null);
}

/**
 * @}
 */

HAM_C_API_END

/**
 * @}
 */

#endif // !HAM_RENDERER_H
