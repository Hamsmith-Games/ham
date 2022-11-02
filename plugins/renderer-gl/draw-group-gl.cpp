#include "renderer-gl.hpp"

#include "ham/log.h"
#include "ham/buffer.h"
#include "ham/gl.h"

#include "glad/glad.h" // IWYU pragma: keep

using namespace ham::typedefs;

HAM_C_API_BEGIN

static inline GLenum ham_vertex_order_to_gl(ham_vertex_order order){
	switch(order){
		case HAM_VERTEX_TRIANGLES: return GL_TRIANGLES;
		case HAM_VERTEX_TRIANGLE_FAN: return GL_TRIANGLE_FAN;
		case HAM_VERTEX_TRIANGLE_STRIP: return GL_TRIANGLE_STRIP;

		default: return GL_TRIANGLES;
	}
}

static inline ham_draw_group_gl *ham_draw_group_gl_ctor(ham_draw_group_gl *group, u32 nargs, va_list va){
	if(nargs != 3){
		ham::logapierror(
			"Wrong number of arguments passed: {}, expected 3 (ham_usize num_shapes, const ham_shape **shapes, const ham_image **images)",
			nargs
		);
		return nullptr;
	}

	const ham_usize num_shapes = va_arg(va, ham_usize);
	const ham_shape *const *const shapes = va_arg(va, const ham_shape* const*);
	const ham_image *const *const images = va_arg(va, const ham_image* const*);

	GLuint vao;
	GLuint bufs[HAM_DRAW_BUFFER_GL_DATA_COUNT];
	GLuint diffuse_tex_arr;

	glCreateVertexArrays(1, &vao);
	glCreateBuffers(HAM_DRAW_BUFFER_GL_DATA_COUNT, bufs);
	glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &diffuse_tex_arr);

	ham_u32 max_w = 0, max_h = 0;

	for(ham_usize i = 0; i < num_shapes; i++){
		const auto img_i = images[i];
		max_w = ham_max(max_w, ham_image_width(img_i));
		max_h = ham_max(max_h, ham_image_height(img_i));
	}

	const GLsizei num_diffuse_levels = 1 + (GLsizei)std::floor(std::log2(ham_max(max_w, max_h)));

	glTextureStorage3D(diffuse_tex_arr, num_diffuse_levels, GL_RGBA8, (GLsizei)max_w, (GLsizei)max_h, (GLsizei)num_shapes);

	for(ham_usize i = 0; i < num_shapes; i++){
		const auto img_i = ham_super(group)->images[i];
		const auto img_w = ham_image_width(img_i);
		const auto img_h = ham_image_height(img_i);
		const auto img_p = ham_image_pixels(img_i);
		glTextureSubImage3D(
			diffuse_tex_arr,
			0, 0, 0,
			0,
			img_w, img_h, 1,
			GL_RGBA,
			GL_UNSIGNED_BYTE,
			img_p
		);
	}

	glGenerateTextureMipmap(diffuse_tex_arr);

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
	glEnableVertexArrayAttrib(vao, 3);

	// translation matrix takes 4 attributes, 1 for each col
	glEnableVertexArrayAttrib(vao, 4);
	glEnableVertexArrayAttrib(vao, 5);
	glEnableVertexArrayAttrib(vao, 6);
	glEnableVertexArrayAttrib(vao, 7);

	glVertexArrayAttribBinding(vao, 0, 0);
	glVertexArrayAttribBinding(vao, 1, 0);
	glVertexArrayAttribBinding(vao, 2, 0);

	glVertexArrayAttribBinding(vao, 3, 1);

	// translation matrix cols
	glVertexArrayAttribBinding(vao, 4, 1);
	glVertexArrayAttribBinding(vao, 5, 1);
	glVertexArrayAttribBinding(vao, 6, 1);
	glVertexArrayAttribBinding(vao, 7, 1);

	glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, false, (GLuint)interleaved_vertex_off);
	glVertexArrayAttribFormat(vao, 1, 3, GL_FLOAT, false, (GLuint)interleaved_normal_off);
	glVertexArrayAttribFormat(vao, 2, 2, GL_FLOAT, false, (GLuint)interleaved_uv_off);

	glVertexArrayAttribFormat(vao, 3, 4, GL_FLOAT, false, (GLuint)offsetof(ham_draw_group_instance_data, color));

	// translation matrix cols
	glVertexArrayAttribFormat(vao, 4, 4, GL_FLOAT, false, (GLuint)offsetof(ham_draw_group_instance_data, trans));
	glVertexArrayAttribFormat(vao, 5, 4, GL_FLOAT, false, (GLuint)(offsetof(ham_draw_group_instance_data, trans) + sizeof(ham_vec4)));
	glVertexArrayAttribFormat(vao, 6, 4, GL_FLOAT, false, (GLuint)(offsetof(ham_draw_group_instance_data, trans) + (2 * sizeof(ham_vec4))));
	glVertexArrayAttribFormat(vao, 7, 4, GL_FLOAT, false, (GLuint)(offsetof(ham_draw_group_instance_data, trans) + (3 * sizeof(ham_vec4))));

	glVertexArrayBindingDivisor(vao, 1, 1);

	const auto ret = new(group) ham_draw_group_gl;

	ret->mode = ham_vertex_order_to_gl(ham_shape_vertex_order(shapes[0]));

	ret->vao = vao;
	memcpy(ret->bufs, bufs, sizeof(GLuint) * HAM_DRAW_BUFFER_GL_DATA_COUNT);

	ret->diffuse_tex_arr = diffuse_tex_arr;

	ret->inst_cap = 0;
	ret->inst_map = nullptr;

	return ret;
}

ham_nothrow static inline void ham_draw_group_gl_dtor(ham_draw_group_gl *group){
	// not needed for persistent mappings?
//	if(group->instance_mapping){
//		glUnmapNamedBuffer(group->bufs[HAM_DRAW_BUFFER_GL_INSTANCE_DATA]);
//	}

	glDeleteVertexArrays(1, &group->vao);
	glDeleteBuffers(HAM_DRAW_BUFFER_GL_DATA_COUNT, group->bufs);

	glDeleteTextures(1, &group->diffuse_tex_arr);

	std::destroy_at(group);
}

static bool ham_draw_group_gl_set_num_instances(ham_draw_group_gl *group, ham_u32 n){
	const auto set_commands = [group, n]{
		const auto num_shapes = ham_super(group)->num_shapes;

		void *cmd_mapping = glMapNamedBufferRange(group->bufs[HAM_DRAW_BUFFER_GL_COMMANDS], 0, num_shapes * sizeof(ham_gl_draw_elements_indirect_command), GL_MAP_WRITE_BIT | GL_MAP_FLUSH_EXPLICIT_BIT);
		if(!cmd_mapping){
			ham::logerror("ham_draw_group_gl_set_num_instances", "Error in glMapNamedBufferRange");
			return false;
		}

		const auto cmds = reinterpret_cast<ham_gl_draw_elements_indirect_command*>(cmd_mapping);
		for(usize i = 0; i < num_shapes; i++){
			cmds->instance_count = n;
		}

		glFlushMappedNamedBufferRange(group->bufs[HAM_DRAW_BUFFER_GL_COMMANDS], 0, num_shapes * sizeof(ham_gl_draw_elements_indirect_command));
		glUnmapNamedBuffer(group->bufs[HAM_DRAW_BUFFER_GL_COMMANDS]);

		return true;
	};

	if(n == 0) return set_commands();

	const auto cur_n = ham_super(group)->num_instances;

	constexpr ham_draw_group_instance_data default_data = {
		ham_mat4_identity(),
		ham_make_vec4(1.f, 1.f, 1.f, 1.f),
	};

	if(!group->inst_map || group->inst_cap < n){
		GLuint new_buffer;
		glCreateBuffers(1, &new_buffer);

		GLbitfield access_flags = GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;

		glNamedBufferStorage(new_buffer, n * sizeof(ham_draw_group_instance_data), nullptr, access_flags);

		void *new_mapping = glMapNamedBufferRange(new_buffer, 0, n * sizeof(ham_draw_group_instance_data), access_flags);
		if(!new_mapping){
			ham::logapierror("Error in glMapNamedBufferRange");
			glDeleteBuffers(1, &new_buffer);
			return false;
		}

		if(group->inst_map && cur_n){
			memcpy(new_mapping, group->inst_map, cur_n * sizeof(ham_draw_group_instance_data));
		}

		glVertexArrayVertexBuffer(group->vao, 1, new_buffer, 0, (GLsizei)sizeof(ham_draw_group_instance_data));

		glDeleteBuffers(1, &group->bufs[HAM_DRAW_BUFFER_GL_INSTANCE_DATA]);

		group->inst_cap = n;
		group->inst_map  = new_mapping;
		group->bufs[HAM_DRAW_BUFFER_GL_INSTANCE_DATA] = new_buffer;
	}

	const isize diff_n = n - cur_n;
	const isize zero_n = std::abs(diff_n);

	const auto data_ptr = (ham_draw_group_instance_data*)group->inst_map + (n < cur_n ? n : cur_n);

	for(isize i = 0; i < zero_n; i++){
		memcpy(data_ptr + i, &default_data, sizeof(ham_draw_group_instance_data));
	}

	return set_commands();
}

static ham_draw_group_instance_data *ham_draw_group_gl_instance_data(ham_draw_group_gl *group){
	return (ham_draw_group_instance_data*)group->inst_map;
}

HAM_C_API_END

ham_define_draw_group(ham_draw_group_gl)
