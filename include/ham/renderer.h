#ifndef HAM_RENDERER_H
#define HAM_RENDERER_H 1

/**
 * @defgroup HAM_RENDERER Rendering
 * @ingroup HAM
 * @{
 */

#include "typedefs.h"

HAM_C_API_BEGIN

typedef struct ham_renderer ham_renderer;

ham_api ham_renderer *ham_renderer_create(const char *plugin_id, const char *obj_id);

ham_api void ham_renderer_destroy(ham_renderer *renderer);

ham_api void ham_renderer_loop(ham_renderer *renderer, ham_f64 dt);

HAM_C_API_END

/**
 * @}
 */

#endif // !HAM_RENDERER_H
