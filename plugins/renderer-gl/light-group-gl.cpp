#include "renderer-gl.hpp"

#include "ham/gl.h"

#include "glad/glad.h"

using namespace ham::typedefs;

HAM_C_API_BEGIN

ham_def_ctor(ham_light_group_gl, nargs, va){
	glCreateVertexArrays(1, &self->vao);
	glCreateBuffers(1, &self->cbo);
	glCreateBuffers(1, &self->ibo);
	glCreateBuffers(1, &self->vbo);
	glCreateBuffers(1, &self->buf);

	constexpr u32 indices[] = { 0, 1, 2, 3 };

	constexpr ham_vec3 verts[] = {
		ham_make_vec3(-1.f, -1.f, 0.f),
		ham_make_vec3( 1.f, -1.f, 0.f),
		ham_make_vec3( 1.f,  1.f, 0.f),
		ham_make_vec3(-1.f,  1.f, 0.f),
	};

	ham_gl_draw_elements_indirect_command cmd;

	cmd.index_count = 4;
	cmd.instance_count = 1;
	cmd.first_index = 0;
	cmd.base_vertex = 0;
	cmd.base_instance = 0;

	// command buffer

	GLbitfield map_bits = GL_MAP_WRITE_BIT;

	glNamedBufferStorage(self->cbo, sizeof(ham_gl_draw_elements_indirect_command), &cmd, map_bits);

	// vertex buffer

	map_bits = GL_MAP_READ_BIT;

	glNamedBufferStorage(self->vbo, sizeof(verts), verts, map_bits);

	// index buffer

	glNamedBufferStorage(self->ibo, sizeof(indices), indices, map_bits);

	// light/instance buffer

	self->inst_cap = 8;

	map_bits = GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_COHERENT_BIT | GL_MAP_PERSISTENT_BIT;

	glNamedBufferStorage(self->buf, sizeof(ham_light) * self->inst_cap, nullptr, map_bits);

	self->inst_map = glMapNamedBufferRange(self->buf, 0, sizeof(ham_light) * self->inst_cap, map_bits);

	glVertexArrayVertexBuffer(self->vao, 0, self->buf, 0, sizeof(ham_light));
	glVertexArrayElementBuffer(self->vao, self->ibo);

	glEnableVertexArrayAttrib(self->vao, 0);
	glEnableVertexArrayAttrib(self->vao, 1);
	glEnableVertexArrayAttrib(self->vao, 2);
	glEnableVertexArrayAttrib(self->vao, 3);

	glVertexArrayAttribBinding(self->vao, 0, 0);
	glVertexArrayAttribBinding(self->vao, 1, 0);
	glVertexArrayAttribBinding(self->vao, 2, 0);
	glVertexArrayAttribBinding(self->vao, 3, 0);

	glVertexArrayAttribFormat(self->vao, 0, 3, GL_FLOAT, GL_FALSE, offsetof(ham_light, pos));
	glVertexArrayAttribFormat(self->vao, 1, 1, GL_FLOAT, GL_FALSE, offsetof(ham_light, effective_radius));
	glVertexArrayAttribFormat(self->vao, 2, 3, GL_FLOAT, GL_FALSE, offsetof(ham_light, color));
	glVertexArrayAttribFormat(self->vao, 3, 1, GL_FLOAT, GL_FALSE, offsetof(ham_light, intensity));

	glVertexArrayBindingDivisor(self->vao, 0, 1);

	return self;
}

ham_def_dtor(ham_light_group_gl){
	glDeleteVertexArrays(1, &self->vao);
	glDeleteBuffers(1, &self->buf);
	glDeleteBuffers(1, &self->cbo);
	glDeleteBuffers(1, &self->ibo);
	glDeleteBuffers(1, &self->vbo);
}

static inline bool ham_light_group_gl_set_num_instances(ham_light_group_gl *self, ham_u32 n){
	if(n > self->inst_cap) return false;

	const auto count_offset = offsetof(ham_gl_draw_elements_indirect_command, instance_count);

	const auto count_map = glMapNamedBufferRange(self->cbo, count_offset, sizeof(ham_u32), GL_MAP_WRITE_BIT | GL_MAP_FLUSH_EXPLICIT_BIT);
	if(!count_map){
		ham::logapierror("Failed to map command buffer");
		return false;
	}

	memcpy(count_map, &n, sizeof(ham_u32));

	glFlushMappedNamedBufferRange(self->cbo, 0, sizeof(ham_u32));
	glUnmapNamedBuffer(self->cbo);

	return true;
}

static inline ham_light *ham_light_group_gl_instance_data(ham_light_group_gl *self){
	return (ham_light*)self->inst_map;
}

ham_define_light_group(ham_light_group_gl)

HAM_C_API_END
