/*
 * Ham Runtime
 * Copyright (C) 2022 Keith Hammond
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef HAM_SHAPE_H
#define HAM_SHAPE_H 1

/**
 * @defgroup HAM_SHAPE 3D shapes and meshes
 * @ingroup HAM
 * @{
 */

#include "math.h"

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

ham_used
ham_nothrow static inline const char *ham_shape_kind_str(ham_shape_kind kind){
	switch(kind){
	#define HAM_CASE(x) case (x): return #x;

		HAM_CASE(HAM_SHAPE_TRIANGLE_MESH)

		HAM_CASE(HAM_SHAPE_QUAD)
		HAM_CASE(HAM_SHAPE_RECT)
		HAM_CASE(HAM_SHAPE_SQUARE)

		HAM_CASE(HAM_SHAPE_HEXAHEDRON)
		HAM_CASE(HAM_SHAPE_CUBOID)
		HAM_CASE(HAM_SHAPE_CUBE)

	#undef HAM_CASE
		default: return "INVALID";
	}
}

typedef enum ham_vertex_order{
	HAM_VERTEX_TRIANGLES,
	HAM_VERTEX_TRIANGLE_FAN,
	HAM_VERTEX_TRIANGLE_STRIP,

	HAM_VERTEX_ORDER_COUNT
} ham_vertex_order;

ham_used
ham_nothrow static inline const char *ham_vertex_order_str(ham_vertex_order order){
	switch(order){
	#define HAM_CASE(x) case (x): return #x;

		HAM_CASE(HAM_VERTEX_TRIANGLES)
		HAM_CASE(HAM_VERTEX_TRIANGLE_FAN)
		HAM_CASE(HAM_VERTEX_TRIANGLE_STRIP)

	#undef HAM_CASE
		default: return "INVALID";
	}
}

typedef struct ham_shape ham_shape;

#define HAM_SHAPE_BONE_WEIGHT_MAX_AFFECTED 4

typedef struct ham_shape_bone{
	ham_mat4 transform;
} ham_shape_bone;

typedef struct ham_shape_material{
	float metallic;
	float roughness;
	float rim;
	float pad0;
	ham_vec4 albedo;
} ham_shape_material;

ham_api const ham_shape *ham_shape_unit_square();

ham_api ham_shape *ham_shape_create_triangle_mesh(
	ham_u32 num_points,
	const ham_vec3 *verts,
	const ham_vec3 *norms,
	const ham_vec2 *uvs,
	const ham_vec4i *bone_indices,
	const ham_vec4 *bone_weights,
	ham_u32 num_indices,
	const ham_u32 *indices,
	ham_u32 num_bones,
	const ham_shape_bone *bones
);

/**
 * Create a quadrilateral shape.
 * @param points points given in order top-left, top-right, bottom-left, bottom-right
 * @returns newly created shape or ``NULL`` on error
 */
ham_api ham_shape *ham_shape_create_quad(const ham_vec2 *points);

ham_api ham_shape *ham_shape_create_rect(ham_f32 w, ham_f32 h);

ham_api ham_shape *ham_shape_create_square(ham_f32 dim);

ham_api ham_shape *ham_shape_create_cuboid(ham_f32 w, ham_f32 h, ham_f32 d);

ham_api ham_shape *ham_shape_create_cube(ham_f32 dim);

ham_api ham_shape *ham_shape_create_heightfield();

/**
 * Destroy a shape.
 * @param shape shape to destroy
 * @see ham_shape_create_quad
 * @see ham_shape_create_rect
 * @see ham_shape_create_square
 */
ham_api void ham_shape_destroy(ham_shape *shape);

ham_api ham_shape_kind ham_shape_get_kind(const ham_shape *shape);
ham_api ham_vertex_order ham_shape_vertex_order(const ham_shape *shape);

ham_api ham_u32 ham_shape_num_points(const ham_shape *shape);
ham_api ham_u32 ham_shape_num_indices(const ham_shape *shape);
ham_api ham_u32 ham_shape_num_bones(const ham_shape *shape);

ham_api const ham_vec3 *ham_shape_vertices(const ham_shape *shape);
ham_api const ham_vec3 *ham_shape_normals(const ham_shape *shape);
ham_api const ham_vec2 *ham_shape_uvs(const ham_shape *shape);

ham_api const ham_u32 *ham_shape_indices(const ham_shape *shape);

ham_api const ham_shape_bone *ham_shape_bones(const ham_shape *shape);

HAM_C_API_END

/**
 * @}
 */

#endif // !HAM_SHAPE_H
