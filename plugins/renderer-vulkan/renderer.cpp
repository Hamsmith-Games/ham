#include "ham/renderer-object.h"
#include "ham/plugin.h"

HAM_C_API_BEGIN

static inline bool ham_renderer_on_load_vulkan(){ return true; }
static inline void ham_renderer_on_unload_vulkan(){}

HAM_PLUGIN(
	ham_renderer_vulkan,
	HAM_RENDERER_VULKAN_PLUGIN_UUID,
	HAM_RENDERER_VULKAN_PLUGIN_NAME,
	HAM_VERSION,
	"Vulkan Rendering",
	"Hamsmith Ltd.",
	"LGPLv3+",
	HAM_RENDERER_PLUGIN_CATEGORY,
	"Rendering using Vulkan 1.2",

	ham_renderer_on_load_vulkan,
	ham_renderer_on_unload_vulkan
)

struct ham_renderer_vulkan{
	ham_derive(ham_renderer)
};

static inline ham_renderer_vulkan *ham_renderer_vulkan_construct(ham_renderer_vulkan *mem, va_list va){
	(void)va;
	return new(mem) ham_renderer_vulkan;
}

static inline void ham_renderer_vulkan_destroy(ham_renderer_vulkan *renderer){
	std::destroy_at(renderer);
}

static inline bool ham_renderer_vulkan_init(ham_renderer *renderer){
	(void)renderer;
	return true;
}

static inline void ham_renderer_vulkan_fini(ham_renderer *renderer){
	(void)renderer;
}

static inline void ham_renderer_vulkan_loop(ham_renderer *renderer, ham_f64 dt){
	(void)renderer; (void)dt;
}

ham_define_object_x(
	2, ham_renderer_vulkan, 1, ham_renderer_vtable,
	ham_renderer_vulkan_construct,
	ham_renderer_vulkan_destroy,
	(
		.init = ham_renderer_vulkan_init,
		.fini = ham_renderer_vulkan_fini,
		.loop = ham_renderer_vulkan_loop,
	)
)

HAM_C_API_END
