#ifndef HAM_SHAPE_H
#define HAM_SHAPE_H 1

/**
 * @defgroup HAM_SHAPE 3D shapes and meshes
 * @ingroup HAM
 * @{
 */

#include "typedefs.h"

HAM_C_API_BEGIN

typedef enum ham_shape_kind{
	HAM_SHAPE_TRIANGLE_MESH,

	HAM_SHAPE_QUAD,
	HAM_SHAPE_RECT,
	HAM_SHAPE_SQUARE,

	HAM_SHAPE_HEXAHEDRON,
	HAM_SHAPE_CUBOID,
	HAM_SHAPE_CUBE,

	HAM_SHAPE_KIND_COUNT,
} ham_shape_kind;

typedef enum ham_vertex_order{
	HAM_VERTEX_TRIANGLES,
	HAM_VERTEX_TRIANGLE_FAN,
	HAM_VERTEX_TRIANGLE_STRIP,

	HAM_VERTEX_ORDER_COUNT
} ham_vertex_order;

typedef struct ham_shape ham_shape;

/**
 * Create a quadrilateral shape.
 * @param points points given in order top-left, top-right, bottom-left, bottom-right
 * @returns newly created shape or ``NULL`` on error
 */
ham_api ham_shape *ham_shape_create_quad(const ham_vec2 *points);

ham_api ham_shape *ham_shape_create_rect(ham_f32 w, ham_f32 h);

ham_api ham_shape *ham_shape_create_square(ham_f32 dim);

/**
 * Destroy a shape.
 * @param shape shape to destroy
 * @see ham_shape_create_quad
 * @see ham_shape_create_rect
 * @see ham_shape_create_square
 */
ham_api void ham_shape_destroy(ham_shape *shape);

ham_api ham_shape_kind ham_shape_get_kind(const ham_shape *shape);

ham_api ham_usize ham_shape_num_points(const ham_shape *shape);
ham_api ham_usize ham_shape_num_indices(const ham_shape *shape);

ham_api const ham_vec3 *ham_shape_vertices(const ham_shape *shape);
ham_api const ham_vec3 *ham_shape_normals(const ham_shape *shape);
ham_api const ham_vec2 *ham_shape_uvs(const ham_shape *shape);

ham_api const ham_u32 *ham_shape_indices(const ham_shape *shape);

HAM_C_API_END

/**
 * @}
 */

#endif // !HAM_SHAPE_H
