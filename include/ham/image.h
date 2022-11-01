#ifndef HAM_IMAGE_H
#define HAM_IMAGE_H

/**
 * @defgroup HAM_IMAGE Images
 * @ingroup HAM
 * @{
 */

#include "ham/typedefs.h"

HAM_C_API_BEGIN

typedef struct ham_image ham_image;

ham_api ham_image *ham_image_create(ham_color_format format, ham_u32 w, ham_u32 h, const void *data);
ham_api ham_image *ham_image_create_view(const ham_image *img);

ham_api void ham_image_destroy(ham_image *img);

ham_api ham_image *ham_image_load_from_mem(ham_color_format format, ham_usize len, const void *data);
ham_api ham_image *ham_image_load(ham_color_format format, ham_str8 filepath);

ham_api const void *ham_image_pixels(const ham_image *img);
ham_api ham_u32 ham_image_width(const ham_image *img);
ham_api ham_u32 ham_image_height(const ham_image *img);

HAM_C_API_END

/**
 * @}
 */

#endif // !HAM_IMAGE_H
