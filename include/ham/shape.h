#ifndef HAM_SHAPE_H
#define HAM_SHAPE_H 1

/**
 * @defgroup HAM_SHAPE 3D Shapes and meshes
 * @ingroup HAM
 * @{
 */

#include "check.h"
#include "memory.h"

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

typedef struct ham_shape{
	const ham_allocator *allocator;
	ham_shape_kind kind;
	ham_u32 num_points, num_indices;
	ham_vec3 *verts, *norms;
	ham_vec2 *uvs;
	ham_u32 *indices;
} ham_shape;

//! @cond ignore
ham_nonnull_args(1)
static inline bool ham_impl_shape_init(
	ham_shape *shape,
	ham_shape_kind kind,
	ham_u32 num_points,
	const ham_vec3 *verts,
	const ham_vec3 *norms,
	const ham_vec2 *uvs,
	ham_u32 num_indices,
	const ham_u32 *indices
){
	const auto allocator = ham_current_allocator();

	const auto new_verts = ham_allocator_alloc(allocator, alignof(ham_vec3), sizeof(ham_vec3) * num_points);
	if(!new_verts) return false;

	const auto new_norms = ham_allocator_alloc(allocator, alignof(ham_vec3), sizeof(ham_vec3) * num_points);
	if(!new_norms){
		ham_allocator_free(allocator, new_verts);
		return false;
	}

	const auto new_uvs = ham_allocator_alloc(allocator, alignof(ham_vec2), sizeof(ham_vec2) * num_points);
	if(!new_uvs){
		ham_allocator_free(allocator, new_verts);
		ham_allocator_free(allocator, new_norms);
		return false;
	}

	const auto new_indices = ham_allocator_alloc(allocator, alignof(ham_u32), sizeof(ham_u32) * num_indices);
	if(!new_indices){
		ham_allocator_free(allocator, new_verts);
		ham_allocator_free(allocator, new_norms);
		ham_allocator_free(allocator, new_uvs);
		return false;
	}

	memcpy(new_verts,   verts,   sizeof(ham_vec3) * num_points);
	memcpy(new_norms,   norms,   sizeof(ham_vec3) * num_points);
	memcpy(new_uvs,     uvs,     sizeof(ham_vec2) * num_points);
	memcpy(new_indices, indices, sizeof(ham_u32)  * num_indices);

	shape->allocator = allocator;
	shape->num_points = num_points;
	shape->num_indices = num_indices;
	shape->verts   = (ham_vec3*)new_verts;
	shape->norms   = (ham_vec3*)new_norms;
	shape->uvs     = (ham_vec2*)new_uvs;
	shape->indices = (ham_u32*)new_indices;

	return true;
}
//! @endcond

static inline void ham_shape_finish(ham_shape *shape){
	if(ham_unlikely(shape == NULL)) return;

	const auto allocator = shape->allocator;

	ham_allocator_free(allocator, shape->verts);
	ham_allocator_free(allocator, shape->norms);
	ham_allocator_free(allocator, shape->uvs);
	ham_allocator_free(allocator, shape->indices);

	shape->allocator = nullptr;
	shape->num_points = 0;
	shape->num_indices = 0;
	shape->verts = nullptr;
	shape->norms = nullptr;
	shape->uvs = nullptr;
	shape->indices = nullptr;
}

static inline ham_shape_kind ham_shape_get_kind(const ham_shape *shape){ return ham_check(shape != NULL) ? shape->kind : HAM_SHAPE_KIND_COUNT; }

static inline ham_usize ham_shape_num_points(const ham_shape *shape){ return ham_check(shape != NULL) ? shape->num_points : (ham_usize)-1; }
static inline ham_usize ham_shape_num_indices(const ham_shape *shape){ return ham_check(shape != NULL) ? shape->num_indices : (ham_usize)-1; }

static inline const ham_vec3 *ham_shape_vertices(const ham_shape *shape){ return ham_check(shape != NULL) ? shape->verts : nullptr; }
static inline const ham_vec3 *ham_shape_normals(const ham_shape *shape){ return ham_check(shape != NULL) ? shape->norms : nullptr; }
static inline const ham_vec2 *ham_shape_uvs(const ham_shape *shape){ return ham_check(shape != NULL) ? shape->uvs : nullptr; }

static inline const ham_u32 *ham_shape_indices(const ham_shape *shape){ return ham_check(shape != NULL) ? shape->indices : nullptr; }

HAM_C_API_END

/**
 * @}
 */

#endif // !HAM_SHAPE_H
