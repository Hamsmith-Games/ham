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
	const auto ret = ham_renderer_vcreate(plugin_id, obj_id, nargs, va);
	va_end(va);
	return ret;
}
//! @endcond

#define ham_renderer_create(plugin_id, obj_id, ...) (ham_impl_renderer_create((plugin_id), (obj_id), HAM_NARGS(__VA_ARGS__) __VA_OPT__(,) __VA_ARGS__))

ham_api void ham_renderer_destroy(ham_renderer *renderer);

ham_api void ham_renderer_loop(ham_renderer *renderer, ham_f64 dt);

/**
 * @defgroup HAM_RENDERER_DRAW_GROUP Draw groups
 * @{
 */

typedef struct ham_renderer_draw_group ham_renderer_draw_group;

ham_api ham_renderer_draw_group *ham_renderer_draw_group_create(
	ham_renderer *r,
	ham_usize num_shapes, const ham_shape *shapes
);

ham_api void ham_renderer_draw_group_destroy(ham_renderer_draw_group *group);

/**
 * @}
 */

HAM_C_API_END

/**
 * @}
 */

#endif // !HAM_RENDERER_H
