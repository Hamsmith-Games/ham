#ifndef HAM_ENGINE_INPUT_H
#define HAM_ENGINE_INPUT_H 1

/**
 * @defgroup HAM_ENGINE_INPUT Input management
 * @ingroup HAM_ENGINE
 * @{
 */

#include "ham/engine/config.h"

#include "ham/typedefs.h"

HAM_C_API_BEGIN

typedef struct ham_input_device ham_input_device;

typedef void(*ham_input_device_pointer_motion_fn)(const ham_input_device *dev, ham_i32 xrel, ham_i32 yrel);

typedef struct ham_input_manager ham_input_manager;

ham_engine_api ham_input_manager *ham_input_manager_create();

ham_engine_api void ham_input_manager_destroy(ham_input_manager *input);

HAM_C_API_END

/**
 * @}
 */

#endif // !HAM_ENGINE_INPUT_H
