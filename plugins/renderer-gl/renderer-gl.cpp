/*
 * Ham Runtime Plugins
 * Copyright (C) 2022 Hamsmith Ltd.
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

#include "renderer-gl.hpp"

#include "ham/check.h"
#include "ham/fs.h"
#include "ham/gl.h"

#include "glad/glad.h"

#include "robin_hood.h"

using namespace ham::typedefs;

HAM_C_API_BEGIN

HAM_PLUGIN(
	ham_renderer_gl,
	HAM_RENDERER_GL_PLUGIN_UUID,
	HAM_RENDERER_GL_PLUGIN_NAME,
	HAM_VERSION,
	"OpenGL Rendering",
	"Hamsmith Ltd.",
	"LGPLv3+",
	HAM_RENDERER_PLUGIN_CATEGORY,
	"Rendering using OpenGL 4.3+",

	ham_plugin_init_pass,
	ham_plugin_fini_pass
)

constexpr static inline GLenum ham_renderer_gl_fbo_attachment_format(ham_renderer_gl_fbo_attachment attachment) noexcept{
	switch(attachment){
		case HAM_RENDERER_GL_FBO_DEPTH_STENCIL: return GL_DEPTH32F_STENCIL8;
		case HAM_RENDERER_GL_FBO_DIFFUSE:       return GL_RGBA8;
		case HAM_RENDERER_GL_FBO_NORMAL:        return GL_RGB16F;
		case HAM_RENDERER_GL_FBO_SCENE:         return GL_R11F_G11F_B10F;
		default: return GL_RGBA8; // squelch warnings
	}
}

constexpr static inline const char *ham_gl_enum_str(GLenum val){
	switch(val){
	#define HAM_CASE(val_) case (val_): return #val_;

		HAM_CASE(GL_DEBUG_TYPE_ERROR)
		HAM_CASE(GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR)
		HAM_CASE(GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR)
		HAM_CASE(GL_DEBUG_TYPE_PORTABILITY)
		HAM_CASE(GL_DEBUG_TYPE_PERFORMANCE)
		HAM_CASE(GL_DEBUG_TYPE_OTHER)
		HAM_CASE(GL_DEBUG_TYPE_MARKER)
		HAM_CASE(GL_DEBUG_TYPE_PUSH_GROUP)
		HAM_CASE(GL_DEBUG_TYPE_POP_GROUP)

		HAM_CASE(GL_DEBUG_SEVERITY_HIGH)
		HAM_CASE(GL_DEBUG_SEVERITY_MEDIUM)
		HAM_CASE(GL_DEBUG_SEVERITY_LOW)
		HAM_CASE(GL_DEBUG_SEVERITY_NOTIFICATION)

		HAM_CASE(GL_DEBUG_SOURCE_API)
		HAM_CASE(GL_DEBUG_SOURCE_WINDOW_SYSTEM)
		HAM_CASE(GL_DEBUG_SOURCE_SHADER_COMPILER)
		HAM_CASE(GL_DEBUG_SOURCE_THIRD_PARTY)
		HAM_CASE(GL_DEBUG_SOURCE_APPLICATION)
		HAM_CASE(GL_DEBUG_SOURCE_OTHER)

		HAM_CASE(GL_FRAMEBUFFER_COMPLETE)
		HAM_CASE(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT)
		HAM_CASE(GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT)
		HAM_CASE(GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER)
		HAM_CASE(GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER)
		HAM_CASE(GL_FRAMEBUFFER_UNSUPPORTED)

		HAM_CASE(GL_VERTEX_SHADER)
		HAM_CASE(GL_FRAGMENT_SHADER)
		HAM_CASE(GL_GEOMETRY_SHADER)
		HAM_CASE(GL_COMPUTE_SHADER)

	#undef HAM_CASE

		default: return "UNKNOWN";
	}
}

static inline void ham_renderer_gl_debug_callback(
	GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar *message,
	const void *user_param
){
	(void)user_param;

	ham::log_level level;

	switch(type){
		case GL_DEBUG_TYPE_ERROR:{
			switch(severity){
				case GL_DEBUG_SEVERITY_HIGH:{
					level = ham::log_level::error;
					break;
				}

				default:{
					level = ham::log_level::warning;
					break;
				}
			}

			break;
		}

		default:{
			level = ham::log_level::verbose;
			break;
		}
	}

	ham::log(
		level,
		"OpenGL",
		"[{}] [{}] [{}] {}: {}",
		ham_gl_enum_str(source),
		ham_gl_enum_str(type),
		ham_gl_enum_str(severity),
		id,
		ham::str8(message, length)
	);
}

static inline ham_renderer_gl *ham_renderer_gl_ctor(ham_renderer_gl *r, ham_u32 nargs, va_list va){
	if(nargs != 1){
		ham::logapierror("Wrong number of arguments passed: {}, expected 1 (const ham_renderer_create_args*)", nargs);
		return nullptr;
	}

	const ham_renderer_create_args *create_args = va_arg(va, const ham_renderer_create_args*);

	const auto getProcAddr = create_args->gl.glGetProcAddr;

	if(!gladLoadGLLoader(getProcAddr)){
		ham::logapierror("Error in gladLoadGLLoader");
		return nullptr;
	}

	GLint num_exts;
	glGetIntegerv(GL_NUM_EXTENSIONS, &num_exts);

	robin_hood::unordered_flat_set<std::string_view> required_exts = {
		"GL_KHR_debug",
		"GL_ARB_direct_state_access",
		"GL_ARB_separate_shader_objects",
		"GL_ARB_clip_control",
		//"GL_ARB_gl_spirv",
		//"GL_ARB_spirv_extensions",
		"GL_ARB_shading_language_include",
	};

	for(GLint i = 0; i < num_exts; i++){
		const auto ext_str = (const char*)glGetStringi(GL_EXTENSIONS, (GLuint)i);
		const auto ext_res = required_exts.find(ext_str);
		if(ext_res != required_exts.end()){
			required_exts.erase(ext_res);
			if(required_exts.empty()){
				break;
			}
		}
	}

	if(!required_exts.empty()){
		ham::str_buffer_utf8 ext_lines;

		for(const auto &ext : required_exts){
			ext_lines += ham::format("\n    {}", ext);
		}

		ham::logapierror("The following extensions are not supported on this platform:{}", ext_lines);
		return nullptr;
	}

#ifdef HAM_DEBUG
	glDebugMessageCallback(ham_renderer_gl_debug_callback, r);
	glEnable(GL_DEBUG_OUTPUT);
#endif

	GLuint global_ubo;
	glCreateBuffers(1, &global_ubo);

	GLbitfield access_flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT;

	glNamedBufferStorage(global_ubo, sizeof(ham_renderer_gl_global_ubo_data), nullptr, access_flags);

	void *global_ubo_map = glMapNamedBufferRange(global_ubo, 0, sizeof(ham_renderer_gl_global_ubo_data), access_flags | GL_MAP_FLUSH_EXPLICIT_BIT);
	if(!global_ubo_map){
		ham::logapierror("Error in glMapNamedBufferRange");
		return nullptr;
	}

	constexpr ham_renderer_gl_global_ubo_data default_ubo_data = {
		.view_proj = ham_mat4_identity(),
		.time      = 0.f,
	};

	memcpy(global_ubo_map, &default_ubo_data, sizeof(default_ubo_data));
	glFlushMappedNamedBufferRange(global_ubo, 0, (GLsizeiptr)sizeof(default_ubo_data));

	const auto scene_info_vert = ham_renderer_gl_load_shader(r, GL_VERTEX_SHADER, HAM_LIT("shaders-gl/scene_info.vert"));
	if(scene_info_vert == (u32)-1){
		ham::logapierror("Error loading scene info vertex shader");
		glDeleteBuffers(1, &global_ubo);
		return nullptr;
	}

	const auto scene_info_frag = ham_renderer_gl_load_shader(r, GL_FRAGMENT_SHADER, HAM_LIT("shaders-gl/scene_info.frag"));
	if(scene_info_frag == (u32)-1){
		ham::logapierror("Error loading scene info fragment shader");
		glDeleteProgram(scene_info_vert);
		glDeleteBuffers(1, &global_ubo);
		return nullptr;
	}

	const auto scene_info_pipeline = ham_renderer_gl_create_pipeline(r, scene_info_vert, scene_info_frag);
	if(scene_info_pipeline == (u32)-1){
		ham::logapierror("Error creating scene info shader pipeline");
		glDeleteProgram(scene_info_vert);
		glDeleteProgram(scene_info_frag);
		glDeleteBuffers(1, &global_ubo);
		return nullptr;
	}

	const auto screen_vert = ham_renderer_gl_load_shader(r, GL_VERTEX_SHADER, HAM_LIT("shaders-gl/screen.vert"));
	if(screen_vert == (u32)-1){
		ham::logapierror("Error loading screen vertex shader");
		glDeleteProgramPipelines(1, &scene_info_pipeline);
		glDeleteProgram(scene_info_vert);
		glDeleteProgram(scene_info_frag);
		glDeleteBuffers(1, &global_ubo);
		return nullptr;
	}

	const auto screen_frag = ham_renderer_gl_load_shader(r, GL_FRAGMENT_SHADER, HAM_LIT("shaders-gl/screen.frag"));
	if(screen_frag == (u32)-1){
		ham::logapierror("Error loading screen fragment shader");
		glDeleteProgramPipelines(1, &scene_info_pipeline);
		glDeleteProgram(scene_info_vert);
		glDeleteProgram(scene_info_frag);
		glDeleteProgram(screen_vert);
		glDeleteBuffers(1, &global_ubo);
		return nullptr;
	}

	r->scene_info_frag_diffuse_tex_loc = glGetUniformLocation(scene_info_frag, "diffuse_tex");

	r->screen_post_frag_depth_loc = glGetUniformLocation(screen_frag, "depth_tex");
	r->screen_post_frag_diffuse_loc = glGetUniformLocation(screen_frag, "diffuse_tex");
	r->screen_post_frag_normal_loc = glGetUniformLocation(screen_frag, "normal_tex");
	r->screen_post_uv_scale_loc = glGetUniformLocation(screen_frag, "uv_scale");

	glProgramUniform1i(scene_info_frag, r->scene_info_frag_diffuse_tex_loc, HAM_DIFFUSE_TEXTURE_UNIT);

	glProgramUniform1i(screen_frag, r->screen_post_frag_depth_loc, HAM_GBO_DEPTH_TEXTURE_UNIT);
	glProgramUniform1i(screen_frag, r->screen_post_frag_diffuse_loc, HAM_GBO_DIFFUSE_TEXTURE_UNIT);
	glProgramUniform1i(screen_frag, r->screen_post_frag_normal_loc, HAM_GBO_NORMAL_TEXTURE_UNIT);
	glProgramUniform2f(screen_frag, r->screen_post_uv_scale_loc, 1.f, 1.f);

	const auto screen_pipeline = ham_renderer_gl_create_pipeline(r, screen_vert, screen_frag);
	if(screen_pipeline == (u32)-1){
		ham::logapierror("Error creating screen shader pipeline");
		glDeleteProgramPipelines(1, &scene_info_pipeline);
		glDeleteProgram(scene_info_vert);
		glDeleteProgram(scene_info_frag);
		glDeleteProgram(screen_vert);
		glDeleteProgram(screen_frag);
		glDeleteBuffers(1, &global_ubo);
		return nullptr;
	}

	GLuint samplers[2];
	glCreateSamplers(std::size(samplers), samplers);

	// Depth/normal sampler
	glSamplerParameteri(samplers[0], GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glSamplerParameteri(samplers[0], GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Diffuse sampler
	glSamplerParameterf(samplers[1], GL_TEXTURE_MAX_ANISOTROPY, 4.f);
	glSamplerParameteri(samplers[1], GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glSamplerParameteri(samplers[1], GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	const auto ret = new(r) ham_renderer_gl;

	ret->fbo = 0;
	memset(ret->fbo_attachments, 0, sizeof(ret->fbo_attachments));

	ret->render_w = 0;
	ret->render_h = 0;

	ret->screen_post_vert = screen_vert;
	ret->screen_post_frag = screen_frag;
	ret->screen_post_pipeline = screen_pipeline;

	ret->scene_info_vert = scene_info_vert;
	ret->scene_info_frag = scene_info_frag;
	ret->scene_info_pipeline = scene_info_pipeline;

	memcpy(ret->samplers, samplers, sizeof(samplers));

	const auto unit_sq = ham_shape_unit_square();
	const ham_image *const null_img = ham_super(ret)->default_img;
	ret->screen_group = (ham_draw_group_gl*)ham_draw_group_create(ham_super(ret), 1, &unit_sq, &null_img); // TODO: check this

	ret->global_ubo = global_ubo;
	ret->global_ubo_writep = global_ubo_map;

	ret->total_time = 0.0;

	return ret;
}

static inline void ham_renderer_gl_dtor(ham_renderer_gl *r){
	glDeleteSamplers(std::size(r->samplers), r->samplers);

	glDeleteProgramPipelines(1, &r->scene_info_pipeline);
	glDeleteProgram(r->scene_info_vert);
	glDeleteProgram(r->scene_info_frag);

	glDeleteProgramPipelines(1, &r->screen_post_pipeline);
	glDeleteProgram(r->screen_post_vert);
	glDeleteProgram(r->screen_post_frag);

	glDeleteBuffers(1, &r->global_ubo);

	glDeleteFramebuffers(1, &r->fbo);
	glDeleteTextures(HAM_RENDERER_GL_FBO_ATTACHMENT_COUNT, r->fbo_attachments);

	std::destroy_at(r);
}

static inline bool ham_renderer_gl_resize(ham_renderer_gl *r, ham_u32 w, ham_u32 h){
	if(r->fbo){
		GLint old_w, old_h;
		glGetTextureLevelParameteriv(r->fbo_attachments[1], 0, GL_TEXTURE_WIDTH, &old_w);
		glGetTextureLevelParameteriv(r->fbo_attachments[1], 0, GL_TEXTURE_HEIGHT, &old_h);

		if(old_w >= w && old_h >= h){
			r->render_w = w;
			r->render_h = h;
			glProgramUniform2f(r->screen_post_frag, r->screen_post_uv_scale_loc, f64(w)/f64(old_w), f64(h)/f64(old_h));
			return true;
		}
	}

	GLuint new_fbo;
	GLuint new_attachments[HAM_RENDERER_GL_FBO_ATTACHMENT_COUNT];

	glCreateFramebuffers(1, &new_fbo);
	glCreateTextures(GL_TEXTURE_2D, HAM_RENDERER_GL_FBO_ATTACHMENT_COUNT, new_attachments);

	GLenum attachment_points[] = {
		GL_DEPTH_STENCIL_ATTACHMENT,
		GL_COLOR_ATTACHMENT0,
		GL_COLOR_ATTACHMENT1,
		GL_COLOR_ATTACHMENT2,
		GL_COLOR_ATTACHMENT3,
		GL_COLOR_ATTACHMENT4,
		GL_COLOR_ATTACHMENT5,
	};

	for(u32 i = 0; i < HAM_RENDERER_GL_FBO_ATTACHMENT_COUNT; i++){
		const auto attachment = static_cast<ham_renderer_gl_fbo_attachment>(i);
		glTextureStorage2D(new_attachments[i], 1, ham_renderer_gl_fbo_attachment_format(attachment), w, h);
		glNamedFramebufferTexture(new_fbo, attachment_points[i], new_attachments[i], 0);
	}

	glNamedFramebufferDrawBuffers(new_fbo, HAM_RENDERER_GL_FBO_ATTACHMENT_COUNT - 1, attachment_points + 1);
	glNamedFramebufferReadBuffer(new_fbo, attachment_points[HAM_RENDERER_GL_FBO_SCENE]);

	GLenum status = glCheckNamedFramebufferStatus(new_fbo, GL_DRAW_FRAMEBUFFER);
	if(status != GL_FRAMEBUFFER_COMPLETE){
		ham::logapierror("Error in glCheckNamedFramebufferStatus: {}", ham_gl_enum_str(status));
		return false;
	}

	if(r->fbo){
		glDeleteFramebuffers(1, &r->fbo);
		glDeleteTextures(HAM_RENDERER_GL_FBO_ATTACHMENT_COUNT, r->fbo_attachments);
	}

	r->fbo = new_fbo;
	memcpy(r->fbo_attachments, new_attachments, sizeof(r->fbo_attachments));

	r->render_w = w;
	r->render_h = h;

	glProgramUniform2f(r->screen_post_frag, r->screen_post_uv_scale_loc, 1.f, 1.f);

	return true;
}

static inline void ham_renderer_gl_frame(ham_renderer_gl *r, ham_f64 dt, const ham_renderer_frame_data *data){
	(void)data;

	const ham::const_camera_view cam = data->common.cam;

	const ham_renderer_gl_global_ubo_data global_data = {
		.view_proj = cam.projection_matrix() * cam.view_matrix(),
		.time      = (f32)r->total_time,
	};

	memcpy(r->global_ubo_writep, &global_data, sizeof(global_data));
	glFlushMappedNamedBufferRange(r->global_ubo, 0, (GLsizeiptr)sizeof(global_data));

	GLint screen_fbo, screen_viewport[4];
	glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &screen_fbo);
	glGetIntegerv(GL_VIEWPORT, screen_viewport);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, r->fbo);
	glViewport(0, 0, r->render_w, r->render_h);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	//glEnable(GL_DEPTH_CLAMP);
	glDepthMask(GL_TRUE);

	glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
	glFrontFace(GL_CCW);
	//glDepthRange(1.0, 0.0);
	//glDepthFunc(GL_LESS);
	glDepthFunc(GL_GEQUAL);

	glClearColor(0.f, 0.f, 0.f, 0.f);
	glClearDepth(0.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	glBindProgramPipeline(r->scene_info_pipeline);
	glBindBufferRange(GL_UNIFORM_BUFFER, 0, r->global_ubo, 0, (GLsizeiptr)sizeof(global_data));

	ham_object_manager_iterate(
		ham_super(r)->draw_groups, [](ham_object *obj, void *user) -> bool{
			const auto group = (const ham_draw_group_gl*)obj;
			if(!ham_super(group)->num_instances) return true;

			const auto r = (const ham_renderer_gl*)ham_super(group)->r;

			glBindTextureUnit(HAM_DIFFUSE_TEXTURE_UNIT, group->diffuse_tex_arr);
			glBindSampler(HAM_DIFFUSE_TEXTURE_UNIT, r->samplers[1]);

			glBindVertexArray(group->vao);
			glBindBufferRange(GL_UNIFORM_BUFFER, 0, r->global_ubo, 0, (GLsizeiptr)sizeof(global_data));
			glBindBuffer(GL_DRAW_INDIRECT_BUFFER, group->bufs[HAM_DRAW_BUFFER_GL_COMMANDS]);
			glMultiDrawElementsIndirect(group->mode, GL_UNSIGNED_INT, nullptr, ham_super(group)->num_shapes, sizeof(ham_gl_draw_elements_indirect_command));
			return true;
		},
		nullptr
	);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDepthMask(GL_FALSE);

	// TODO: do lighting pass

	glBindTextureUnit(HAM_GBO_DEPTH_TEXTURE_UNIT,   r->fbo_attachments[0]);
	glBindTextureUnit(HAM_GBO_DIFFUSE_TEXTURE_UNIT, r->fbo_attachments[1]);
	glBindTextureUnit(HAM_GBO_NORMAL_TEXTURE_UNIT,  r->fbo_attachments[2]);

	glBindSampler(HAM_GBO_DEPTH_TEXTURE_UNIT,   r->samplers[0]);
	glBindSampler(HAM_GBO_DIFFUSE_TEXTURE_UNIT, r->samplers[0]);
	glBindSampler(HAM_GBO_NORMAL_TEXTURE_UNIT,  r->samplers[0]);

	//const ham_mat4 *const view_mat = ham_camera_view_matrix(data->common.cam);
	//const ham_mat4 *const proj_mat = ham_camera_proj_matrix(data->common.cam);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, (GLuint)screen_fbo);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, r->fbo);

	glViewport(0, 0, r->render_w, r->render_h);

	glClearColor(0.f, 0.f, 0.f, 0.f);
	glClear(GL_COLOR_BUFFER_BIT);

	glBindProgramPipeline(r->screen_post_pipeline);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, r->global_ubo);

	glBindVertexArray(r->screen_group->vao);

	glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_INT, nullptr);

	glViewport(screen_viewport[0], screen_viewport[1], screen_viewport[2], screen_viewport[3]);

	r->total_time += dt;
}

ham_define_renderer(ham_renderer_gl, ham_draw_group_gl)

//
// Shaders
//

ham_renderer_gl_api ham_u32 ham_renderer_gl_load_shader(ham_renderer_gl *r, ham_u32 shader_type, ham_str8 filename){
	const auto plugin_dir = ham_plugin_dir(ham_super(r)->plugin);

	ham_path_buffer_utf8 shader_path = { 0 };

	memcpy(shader_path, plugin_dir.ptr, plugin_dir.len);
	shader_path[plugin_dir.len] = '/';

	const auto path_len = plugin_dir.len + filename.len + 1;

	if(path_len >= HAM_PATH_BUFFER_SIZE){
		shader_path[plugin_dir.len] = '\0';
		ham::logapierror(
			"Path to {} shader too long ({}, max {}): {}/{}",
			ham_gl_enum_str(shader_type),
			path_len, HAM_PATH_BUFFER_SIZE,
			plugin_dir.ptr, filename.ptr
		);
		return (u32)-1;
	}

	memcpy(shader_path + plugin_dir.len + 1, filename.ptr, filename.len);
	shader_path[path_len] = '\0';

	const auto file = ham_file_open_utf8(ham::str8(shader_path, path_len), HAM_OPEN_READ);
	if(!file){
		ham::logapierror(
			"Failed to open {} shader: {}",
			ham_gl_enum_str(shader_type),
			shader_path
		);
		return (u32)-1;
	}

	ham_file_info file_info;
	if(!ham_file_get_info(file, &file_info)){
		ham::logapierror(
			"Failed to get file info for {} shader: {}",
			ham_gl_enum_str(shader_type),
			shader_path
		);
		ham_file_close(file);
		return (u32)-1;
	}

	const auto src_buf = (char*)ham_allocator_alloc(ham_super(r)->allocator, alignof(void*), file_info.size);
	if(!src_buf){
		ham::logapierror(
			"Failed to allocate memory for {} shader: {}",
			ham_gl_enum_str(shader_type),
			shader_path
		);
		ham_file_close(file);
		return (u32)-1;
	}

	ham_file_read(file, src_buf, file_info.size);
	ham_file_close(file);

	GLuint shad = glCreateShader((GLenum)shader_type);
	if(shad == 0){
		ham::logapierror("Error in glCreateShader({})", ham_gl_enum_str(shader_type));
		ham_allocator_free(ham_super(r)->allocator, src_buf);
		return (u32)-1;
	}

	if(strcmp(file_info.mime.ptr, HAM_MIME_BINARY) == 0){
		// Only support spir-v binaries currently
		glShaderBinary(1, &shad, GL_SHADER_BINARY_FORMAT_SPIR_V_ARB, src_buf, file_info.size);
		glSpecializeShaderARB(shad, "main", 0, nullptr, nullptr);
	}
	else{
		GLint length = file_info.size;
		glShaderSource(shad, 1, &src_buf, &length);
		glCompileShader(shad);
	}

	ham_allocator_free(ham_super(r)->allocator, src_buf);

	GLint status;

	glGetShaderiv(shad, GL_COMPILE_STATUS, &status);
	if(status != GL_TRUE){
		glGetShaderiv(shad, GL_INFO_LOG_LENGTH, &status);

		ham::str_buffer_utf8 buf;
		buf.resize((usize)status + 1, '\0');

		glGetShaderInfoLog(shad, status, &status, buf.ptr());

		ham::logapierror("Failed to compile shader {}:\n{}", shader_path, buf.c_str());

		glDeleteShader(shad);

		return (u32)-1;
	}

	GLuint prog = glCreateProgram();

	glAttachShader(prog, shad);
	glProgramParameteri(prog, GL_PROGRAM_SEPARABLE, (GLint)GL_TRUE);
	glLinkProgram(prog);
	glDetachShader(prog, shad);
	glDeleteShader(shad);

	glGetProgramiv(prog, GL_LINK_STATUS, &status);
	if(status != GL_TRUE){
		glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &status);

		ham::str_buffer_utf8 buf;
		buf.resize((usize)status + 1, '\0');

		glGetProgramInfoLog(prog, status, &status, buf.ptr());

		ham::logapierror("Failed to link shader {}:\n{}", shader_path, buf.c_str());

		glDeleteProgram(prog);

		return (u32)-1;
	}

	glValidateProgram(prog);

	glGetProgramiv(prog, GL_VALIDATE_STATUS, &status);
	if(status != GL_TRUE){
		glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &status);

		ham::str_buffer_utf8 buf;
		buf.resize((usize)status + 1, '\0');

		glGetProgramInfoLog(prog, status, &status, buf.ptr());

		ham::logapierror("Failed to validate shader {}:\n{}", shader_path, buf.c_str());

		glDeleteProgram(prog);

		return (u32)-1;
	}

	return prog;
}

ham_u32 ham_renderer_gl_create_pipeline(ham_renderer_gl *r, ham_u32 vert_prog, ham_u32 frag_prog){
	if(!ham_check(vert_prog != (u32)-1) || !ham_check(frag_prog != (u32)-1)){
		return (u32)-1;
	}

	GLuint ret;
	glCreateProgramPipelines(1, &ret);

	glUseProgramStages(ret, GL_VERTEX_SHADER_BIT, vert_prog);
	glUseProgramStages(ret, GL_FRAGMENT_SHADER_BIT, frag_prog);

	glValidateProgramPipeline(ret);

	GLint status;

	glGetProgramPipelineiv(ret, GL_VALIDATE_STATUS, &status);
	if(status != GL_TRUE){
		glGetProgramPipelineiv(ret, GL_INFO_LOG_LENGTH, &status);

		ham::str_buffer_utf8 buf;
		buf.resize((usize)status + 1, '\0');

		glGetProgramPipelineInfoLog(ret, status, &status, buf.ptr());

		ham::logapierror("Failed to validate shader pipeline:\n{}", buf.c_str());

		glDeleteProgramPipelines(1, &ret);

		return (u32)-1;
	}

	return ret;
}

HAM_C_API_END
