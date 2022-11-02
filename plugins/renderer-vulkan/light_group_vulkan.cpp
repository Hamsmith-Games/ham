#include "renderer.hpp"

using namespace ham::typedefs;

HAM_C_API_BEGIN

ham_def_ctor(ham_light_group_vulkan, nargs, va){
	self->inst_cap = 0;
	self->inst_map = nullptr;
	return self;
}

ham_def_dtor(ham_light_group_vulkan){}

static inline bool ham_light_group_vulkan_set_num_instances(ham_light_group_vulkan *self, ham_u32 n){
	if(n <= self->inst_cap) return true;
	else return false;
}

static inline ham_light *ham_light_group_vulkan_instance_data(ham_light_group_vulkan *self){
	return (ham_light*)self->inst_map;
}

ham_define_light_group(ham_light_group_vulkan)

HAM_C_API_END
