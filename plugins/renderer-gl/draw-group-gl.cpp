#include "renderer-gl.hpp"

#include "ham/log.h"
#include "ham/buffer.h"
#include "ham/gl.h"

#include "glad/glad.h" // IWYU pragma: keep

using namespace ham::typedefs;

HAM_C_API_BEGIN

static inline ham_draw_group_gl *ham_draw_group_gl_ctor(ham_draw_group_gl *group, u32 nargs, va_list va){
	if(nargs != 2){
		ham::logapierror("Wrong number of arguments passed: {}, expected 2 (ham_usize num_shapes, const ham_shape *const *shapes)", nargs);
		return nullptr;
	}

	const ham_usize num_shapes = va_arg(va, ham_usize);
	const ham_shape *const *const shapes = va_arg(va, const ham_shape* const*);

	GLuint vao;
	GLuint bufs[3];

	glCreateVertexArrays(1, &vao);
	glCreateBuffers(3, bufs);

	constexpr usize interleaved_point_size = (2UL * sizeof(ham_vec3)) + sizeof(ham_vec2);
	constexpr usize interleaved_vertex_off = 0;
	constexpr usize interleaved_normal_off = sizeof(ham_vec3);
	constexpr usize interleaved_uv_off     = sizeof(ham_vec3) * 2UL;

	u32 total_points = 0, total_indices = 0;

	ham::basic_buffer<ham_gl_draw_elements_indirect_command> cmds;

	for(usize i = 0; i < num_shapes; i++){
		const auto shape = shapes[i];

		const auto num_points  = ham_shape_num_points(shape);
		const auto num_indices = ham_shape_num_indices(shape);

		cmds.emplace_back(ham_gl_draw_elements_indirect_command{
			.index_count    = (u32)num_indices,
			.instance_count = 0,
			.first_index    = (u32)total_indices,
			.base_vertex    = (i32)total_points,
			.base_instance  = 0,
		});

		total_points  += num_points;
		total_indices += num_indices;
	}

	glNamedBufferStorage(bufs[HAM_DRAW_BUFFER_GL_POINTS],   total_points  * interleaved_point_size, nullptr, GL_MAP_WRITE_BIT);
	glNamedBufferStorage(bufs[HAM_DRAW_BUFFER_GL_INDICES],  total_indices * sizeof(ham_u32), nullptr, GL_MAP_WRITE_BIT);
	glNamedBufferStorage(bufs[HAM_DRAW_BUFFER_GL_COMMANDS], num_shapes    * sizeof(ham_gl_draw_elements_indirect_command), cmds.data(), GL_MAP_WRITE_BIT | GL_MAP_READ_BIT);

	const auto points_map = (char*)glMapNamedBuffer(bufs[HAM_DRAW_BUFFER_GL_POINTS], GL_WRITE_ONLY);
	const auto indices_map = (char*)glMapNamedBuffer(bufs[HAM_DRAW_BUFFER_GL_INDICES], GL_WRITE_ONLY);

	for(usize i = 0; i < num_shapes; i++){
		const auto shape = shapes[i];
		const auto &cmd  = cmds[i];

		const usize points_map_off  = interleaved_point_size * cmd.base_vertex;
		const usize indices_map_off = sizeof(u32) * cmd.first_index;

		const auto num_points  = ham_shape_num_points(shape);
		const auto num_indices = ham_shape_num_indices(shape);

		const auto verts = ham_shape_vertices(shape);
		const auto norms = ham_shape_normals(shape);
		const auto uvs   = ham_shape_uvs(shape);

		const auto indices = ham_shape_indices(shape);

		auto write_ptr = points_map + points_map_off;

		for(usize i = 0; i < num_points; i++){
			memcpy(write_ptr, verts + i, sizeof(ham_vec3));
			write_ptr += sizeof(ham_vec3);

			memcpy(write_ptr, norms + i, sizeof(ham_vec3));
			write_ptr += sizeof(ham_vec3);

			memcpy(write_ptr, uvs + i, sizeof(ham_vec2));
			write_ptr += sizeof(ham_vec2);
		}

		memcpy(indices_map + indices_map_off, indices, sizeof(u32) * num_indices);
	}

	//glFlushMappedNamedBufferRange(bufs[HAM_DRAW_BUFFER_GL_POINTS],  0, total_points  * interleaved_point_size);
	//glFlushMappedNamedBufferRange(bufs[HAM_DRAW_BUFFER_GL_INDICES], 0, total_indices * sizeof(ham_u32));

	glUnmapNamedBuffer(bufs[HAM_DRAW_BUFFER_GL_POINTS]);
	glUnmapNamedBuffer(bufs[HAM_DRAW_BUFFER_GL_INDICES]);

	glVertexArrayVertexBuffer(vao, 0, bufs[HAM_DRAW_BUFFER_GL_POINTS], 0, (GLsizei)interleaved_point_size);
	glVertexArrayElementBuffer(vao, bufs[HAM_DRAW_BUFFER_GL_INDICES]);

	glEnableVertexArrayAttrib(vao, 0);
	glEnableVertexArrayAttrib(vao, 1);
	glEnableVertexArrayAttrib(vao, 2);

	glVertexArrayAttribBinding(vao, 0, 0);
	glVertexArrayAttribBinding(vao, 1, 0);
	glVertexArrayAttribBinding(vao, 2, 0);

	glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, false, (GLuint)interleaved_vertex_off);
	glVertexArrayAttribFormat(vao, 1, 3, GL_FLOAT, false, (GLuint)interleaved_normal_off);
	glVertexArrayAttribFormat(vao, 2, 2, GL_FLOAT, false, (GLuint)interleaved_uv_off);

	const auto ret = new(group) ham_draw_group_gl;

	ret->num_instances = 0;
	ret->vao = vao;
	memcpy(ret->bufs, bufs, sizeof(GLuint) * HAM_DRAW_BUFFER_GL_DATA_COUNT);

	return ret;
}

ham_nothrow static inline void ham_draw_group_gl_dtor(ham_draw_group_gl *group){
	glDeleteVertexArrays(1, &group->vao);
	glDeleteBuffers(HAM_DRAW_BUFFER_GL_DATA_COUNT, group->bufs);

	std::destroy_at(group);
}

HAM_C_API_END

ham_define_draw_group(ham_draw_group_gl)
