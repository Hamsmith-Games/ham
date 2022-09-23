#ifndef HAM_TYPESYS_H
#define HAM_TYPESYS_H 1

/**
 * @defgroup HAM_TYPESYS Runtime type construction
 * @ingroup HAM
 * @{
 */

#include "typedefs.h"

HAM_C_API_BEGIN

typedef struct ham_typeset ham_typeset;

ham_api ham_typeset *ham_typeset_create();

ham_api void ham_typeset_destroy(ham_typeset *ts);

HAM_C_API_END

/**
 * @}
 */

#endif // !HAM_TYPESYS_H
