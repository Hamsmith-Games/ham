#ifndef HAM_NET_H
#define HAM_NET_H 1

/**
 * @defgroup HAM_NET Networking
 * @ingroup HAM_ENGINE
 * @{
 */

#include "typedefs.h"

HAM_C_API_BEGIN

typedef struct ham_net ham_net;

ham_api ham_net *ham_net_create(const char *plugin_id, const char *obj_id);

ham_api void ham_net_destroy(ham_net *net);

ham_api void ham_net_loop(ham_net *net, ham_f64 dt);

HAM_C_API_END

/**
 * @}
 */

#endif // !HAM_NET_H
