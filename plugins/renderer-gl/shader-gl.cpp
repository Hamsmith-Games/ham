#include "renderer-gl.hpp"

#include "glad/glad.h"

using namespace ham::typedefs;

HAM_C_API_BEGIN

static inline GLenum ham_shader_kind_to_gl(ham_shader_kind kind){
	switch(kind){
		case HAM_SHADER_VERTEX: return GL_VERTEX_SHADER;
		case HAM_SHADER_GEOMETRY: return GL_GEOMETRY_SHADER;

		case HAM_SHADER_FRAGMENT:
		default: return GL_FRAGMENT_SHADER;
	}
}

ham_used
static inline GLbitfield ham_shader_kind_to_gl_bit(ham_shader_kind kind){
	switch(kind){
		case HAM_SHADER_VERTEX: return GL_VERTEX_SHADER_BIT;
		case HAM_SHADER_GEOMETRY: return GL_GEOMETRY_SHADER_BIT;

		case HAM_SHADER_FRAGMENT:
		default: return GL_FRAGMENT_SHADER_BIT;
	}
}

static inline ham_shader_type ham_shader_type_from_gl(GLenum type){
	switch(type){
		case GL_INT:          return HAM_SHADER_INT;
		case GL_UNSIGNED_INT: return HAM_SHADER_UINT;
		case GL_FLOAT:        return HAM_SHADER_FLOAT;
		case GL_FLOAT_VEC2:   return HAM_SHADER_VEC2;
		case GL_FLOAT_VEC3:   return HAM_SHADER_VEC3;
		case GL_FLOAT_VEC4:   return HAM_SHADER_VEC4;
		case GL_FLOAT_MAT2:   return HAM_SHADER_MAT2;
		case GL_FLOAT_MAT3:   return HAM_SHADER_MAT3;
		case GL_FLOAT_MAT4:   return HAM_SHADER_MAT4;
		default: return HAM_SHADER_TYPE_COUNT;
	}
}

ham_def_ctor(ham_shader_gl, nargs, args){
	if(nargs != 1){
		ham::logapierror("Wrong number of args %u, expected 1: (ham_shader_kind kind)");
		return nullptr;
	}

	const auto kind = (ham_shader_kind)va_arg(args, u32);

	GLuint shad = glCreateShader(ham_shader_kind_to_gl(kind));

	self->handle = shad;
	self->num_uniforms = 0;

	const auto allocator = ham_super(self)->r->allocator;

	ham_buffer_init_allocator(&self->uniforms, allocator, alignof(ham_shader_uniform), sizeof(ham_shader_uniform) * 8);
	ham_buffer_init_allocator(&self->uniform_names, allocator, alignof(void*), sizeof(void*) * 8);

	return self;
}

ham_def_dtor(ham_shader_gl){
	const auto allocator = ham_super(self)->r->allocator;

	if(ham_super(self)->compiled){
		glDeleteProgram(self->handle);
	}
	else{
		glDeleteShader(self->handle);
	}

	ham_buffer_foreach(&self->uniform_names, char*, ptr){
		ham_allocator_free(allocator, ptr);
	}

	ham_buffer_finish(&self->uniform_names);
	ham_buffer_finish(&self->uniforms);
}

static inline bool ham_def_method(ham_shader_gl, add_include, ham_str8 name, ham_str8 src){
	ham_name_buffer_utf8 name_buf;
	name_buf[0] = '/';
	memcpy(name_buf + 1, name.ptr, name.len);
	name_buf[name.len + 1] = '\0';

	glNamedStringARB(self->handle, (GLint)name.len + 1, name_buf, (GLint)src.len, src.ptr);

	return true;
}

static inline bool ham_def_method(ham_shader_gl, set_source, ham_shader_source_kind kind, ham_str8 src){
	self->src_kind = kind;

	switch(kind){
		case HAM_SHADER_SOURCE_SPIRV:{
			glShaderBinary(1, &self->handle, GL_SHADER_BINARY_FORMAT_SPIR_V, (const void*)src.ptr, (GLsizei)src.len);
			break;
		}

		case HAM_SHADER_SOURCE_GLSL:
		default:{
			const GLchar *srcs[] = { src.ptr };
			const GLint   lens[] = { (GLint)src.len };

			glShaderSource(self->handle, 1, srcs, lens);

			break;
		}
	}

	return true;
}

static inline bool ham_def_method(ham_shader_gl, compile){
	switch(self->src_kind){
		case HAM_SHADER_SOURCE_SPIRV:{
			glSpecializeShader(self->handle, "main", 0, nullptr, nullptr);
			break;
		}

		case HAM_SHADER_SOURCE_GLSL:
		default:{
			const GLchar *search_paths[] = { "/" };
			const GLint   search_lens[]  = { 1 };

			glCompileShaderIncludeARB(self->handle, 1, search_paths, search_lens);
			break;
		}
	}

	GLint res;
	glGetShaderiv(self->handle, GL_COMPILE_STATUS, &res);

	if(res != GL_TRUE){
		glGetShaderiv(self->handle, GL_INFO_LOG_LENGTH, &res);

		ham::str_buffer_utf8 msg_buf;
		msg_buf.resize(res, '\0');

		glGetShaderInfoLog(self->handle, res, &res, msg_buf.ptr());

		ham::logapierror("Error compiling shader: {}", msg_buf.c_str());

		return false;
	}

	GLuint prog = glCreateProgram();

	glProgramParameteri(prog, GL_PROGRAM_SEPARABLE, GL_TRUE);

	glAttachShader(prog, self->handle);

	glLinkProgram(prog);

	glGetProgramiv(prog, GL_LINK_STATUS, &res);

	if(res != GL_TRUE){
		glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &res);

		ham::str_buffer_utf8 msg_buf;
		msg_buf.resize(res, '\0');

		glGetProgramInfoLog(prog, res, &res, msg_buf.ptr());

		ham::logapierror("Error linking shader: {}", msg_buf.c_str());

		glDeleteProgram(prog);

		return false;
	}

	glDeleteShader(self->handle);
	self->handle = prog;

	GLint num_uniforms;
	glGetProgramInterfaceiv(prog, GL_UNIFORM, GL_ACTIVE_RESOURCES, &num_uniforms);

	const GLenum uniform_props[] = { GL_BLOCK_INDEX, GL_TYPE, GL_NAME_LENGTH, GL_LOCATION };

	u32 num_non_block_uniforms = 0;

	const auto allocator = ham_super(self)->r->allocator;

	for(GLint i = 0; i < num_uniforms; i++){
		const auto num_props = (GLsizei)std::size(uniform_props);

		GLint prop_values[std::size(uniform_props)];

		glGetProgramResourceiv(
			prog, GL_UNIFORM, i,
			num_props, uniform_props,
			num_props, nullptr, prop_values
		);

		if(prop_values[0] != -1){
			// uniform block
			continue;
		}

		const auto name_str = (char*)ham_allocator_alloc(allocator, alignof(void*), (usize)prop_values[2] + 1);
		glGetProgramResourceName(prog, GL_UNIFORM, i, prop_values[2], nullptr, name_str);
		name_str[prop_values[2]] = '\0';

		ham::logapiverbose("Found uniform by name '{}'", name_str);

		const ham_shader_uniform uniform = {
			.location = (u32)prop_values[3],
			.name     = ham_str8{name_str, (usize)prop_values[2]},
			.type     = ham_shader_type_from_gl(prop_values[1])
		};

		ham_buffer_insert(&self->uniform_names, num_non_block_uniforms * sizeof(char*), &name_str, sizeof(char*));
		ham_buffer_insert(&self->uniforms, num_non_block_uniforms * sizeof(ham_shader_uniform), &uniform, sizeof(uniform));

		++num_non_block_uniforms;
	}

	self->num_uniforms = num_non_block_uniforms;

	return true;
}

static inline ham_u32 ham_def_cmethod(ham_shader_gl, num_uniforms){ return self->num_uniforms; }
static inline const ham_shader_uniform *ham_def_cmethod(ham_shader_gl, uniforms){ return (const ham_shader_uniform*)ham_buffer_data((ham_buffer*)&self->uniforms); }

ham_define_shader(ham_shader_gl)

HAM_C_API_END
