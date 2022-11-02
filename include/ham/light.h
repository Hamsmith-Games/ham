#ifndef HAM_LIGHT_H
#define HAM_LIGHT_H 1

/**
 * @defgroup HAM_LIGHT Lights
 * @ingroup HAM
 * @{
 */

#include "math.h"

HAM_C_API_BEGIN

typedef struct ham_light{
	ham_vec3 pos;
	ham_f32 effective_radius;

	ham_vec3 color;
	ham_f32 intensity;
} ham_light;

static_assert(sizeof(ham_light) == sizeof(ham_vec4)*2, "Invalid ham_light alignment/size");

HAM_C_API_END

/**
 * @}
 */

#endif // !HAM_LIGHT_H
