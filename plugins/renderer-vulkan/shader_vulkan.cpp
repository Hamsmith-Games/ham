#include "renderer.hpp"

HAM_C_API_BEGIN

ham_def_ctor(ham_shader_vulkan, nargs, args){
	(void)nargs; (void)args;
	ham::logapierror("UNIMPLEMENTED");
	return nullptr;
}

ham_def_dtor(ham_shader_vulkan){}

static inline bool ham_def_method(ham_shader_vulkan, set_source, ham_shader_source_kind kind, ham_str8 src){
	(void)kind; (void)src;
	ham::logapierror("UNIMPLEMENTED");
	return false;
}

static inline bool ham_def_method(ham_shader_vulkan, compile){
	ham::logapierror("UNIMPLEMENTED");
	return false;
}

static inline ham_u32 ham_def_cmethod(ham_shader_vulkan, num_uniforms){ return self->num_uniforms; }
static inline const ham_shader_uniform *ham_def_cmethod(ham_shader_vulkan, uniforms){ return self->uniforms; }

ham_define_shader(ham_shader_vulkan)

HAM_C_API_END
