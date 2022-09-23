#ifndef HAM_NET_H
#define HAM_NET_H 1

/**
 * @defgroup HAM_NET Networking
 * @ingroup HAM_ENGINE
 * @{
 */

#include "typedefs.h"

HAM_C_API_BEGIN

typedef struct ham_net_context ham_net_context;

ham_api ham_net_context *ham_net_context_create(const char *plugin_id);

ham_api void ham_net_context_destroy(ham_net_context *net);

ham_api void ham_net_context_loop(ham_net_context *net, ham_f64 dt);

HAM_C_API_END

/**
 * @}
 */

#endif // !HAM_NET_H
