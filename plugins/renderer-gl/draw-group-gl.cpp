#include "renderer-gl.hpp"

#include "ham/log.h"

using namespace ham::typedefs;

HAM_C_API_BEGIN

static inline ham_draw_group_gl *ham_draw_group_gl_ctor(ham_draw_group_gl *group, u32 nargs, va_list va){
	if(nargs != 2){
		ham::logapierror("Wrong number of arguments passed: {}, expected 2 (ham_usize num_shapes, const ham_shape *const *shapes)", nargs);
		return nullptr;
	}

	const ham_usize num_shapes = va_arg(va, ham_usize);
	const ham_shape *const *shapes = va_arg(va, const ham_shape* const*);

	const auto ret = new(group) ham_draw_group_gl;

	return ret;
}

ham_nothrow static inline void ham_draw_group_gl_dtor(ham_draw_group_gl *group){
	std::destroy_at(group);
}

HAM_C_API_END

ham_define_draw_group(ham_draw_group_gl)
