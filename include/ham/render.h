#ifndef HAM_RENDER_H
#define HAM_RENDER_H 1

/**
 * @defgroup HAM_RENDER Rendering
 * @ingroup HAM
 * @{
 */

#include "typedefs.h"

HAM_C_API_BEGIN

/**
 * @defgroup HAM_RENDER_CONTEXT Context management
 * @{
 */

typedef struct ham_render_context ham_render_context;

ham_api ham_render_context *ham_render_context_create(const char *plugin_id);

ham_api void ham_render_context_destroy(ham_render_context *ctx);

ham_api void ham_render_context_loop(ham_render_context *ctx, ham_f64 dt);

/**
 * @}
 */

HAM_C_API_END

/**
 * @}
 */

#endif // HAM_RENDER_H 1
