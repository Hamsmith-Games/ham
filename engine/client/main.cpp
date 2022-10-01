/*
 * The Ham World Engine Client
 * Copyright (C) 2022  Hamsmith Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#define HAM_ENGINE_CLIENT_API_NAME "ham-engine-client"
#define HAM_ENGINE_CLIENT_OBJ_NAME "ham_engine_client"

#include "ham/engine-object.h"
#include "ham/renderer.h"
#include "ham/net.h"
#include "ham/plugin.h"
#include "ham/log.h"

#include "SDL.h"
#include "SDL_vulkan.h"

#include <vulkan/vulkan_core.h>

using namespace ham::typedefs;

static inline bool ham_engine_client_on_load(){
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC) != 0){
		ham_logerrorf(HAM_ENGINE_CLIENT_API_NAME, "Error in SDL_Init: %s", SDL_GetError());
		return false;
	}

	return true;
}

static inline void ham_engine_client_on_unload(){
	SDL_Quit();
}

HAM_PLUGIN(
	ham_engine_client,
	HAM_ENGINE_CLIENT_PLUGIN_UUID,
	HAM_ENGINE_CLIENT_API_NAME,
	HAM_VERSION,
	"Ham World Engine Client",
	"Hamsmith Ltd.",
	"GPLv3+",
	HAM_ENGINE_PLUGIN_CATEGORY,
	"Ham World Engine Client",
	ham_engine_client_on_load,
	ham_engine_client_on_unload
)

struct ham_engine_client{
	ham_derive(ham_engine)

	SDL_Window *window = nullptr;
	VkInstance vk_inst = nullptr;
	VkSurfaceKHR vk_surface = nullptr;

	PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = nullptr;
	PFN_vkCreateInstance vkCreateInstance = nullptr;
	PFN_vkDestroyInstance vkDestroyInstance = nullptr;
	PFN_vkDestroySurfaceKHR vkDestroySurfaceKHR = nullptr;

	ham_renderer *renderer = nullptr;
};

ham_nonnull_args(1)
static inline bool ham_engine_client_init_vulkan(ham_engine_client *engine);

static inline ham_engine_client *ham_engine_client_construct(ham_engine_client *mem, ham_usize nargs, va_list va){
	(void)nargs; (void)va;
	const auto ret = new(mem) ham_engine_client;
	ham_super(ret)->min_dt = 1.0/60.0;
	return ret;
}

static inline void ham_engine_client_destroy(ham_engine_client *engine){
	std::destroy_at(engine);
}

static inline const char *vk_result_to_str(VkResult res){
	switch(res){
	#define HAM_CASE(val_) case (val_): return #val_;

		HAM_CASE(VK_SUCCESS)
		HAM_CASE(VK_NOT_READY)
		HAM_CASE(VK_TIMEOUT)
		HAM_CASE(VK_EVENT_SET)
		HAM_CASE(VK_EVENT_RESET)
		HAM_CASE(VK_INCOMPLETE)
		HAM_CASE(VK_ERROR_OUT_OF_HOST_MEMORY)
		HAM_CASE(VK_ERROR_OUT_OF_DEVICE_MEMORY)
		HAM_CASE(VK_ERROR_INITIALIZATION_FAILED)
		HAM_CASE(VK_ERROR_DEVICE_LOST)
		HAM_CASE(VK_ERROR_MEMORY_MAP_FAILED)
		HAM_CASE(VK_ERROR_LAYER_NOT_PRESENT)
		HAM_CASE(VK_ERROR_EXTENSION_NOT_PRESENT)
		HAM_CASE(VK_ERROR_FEATURE_NOT_PRESENT)
		HAM_CASE(VK_ERROR_INCOMPATIBLE_DRIVER)
		HAM_CASE(VK_ERROR_TOO_MANY_OBJECTS)
		HAM_CASE(VK_ERROR_FORMAT_NOT_SUPPORTED)
		HAM_CASE(VK_ERROR_FRAGMENTED_POOL)
		HAM_CASE(VK_ERROR_UNKNOWN)
		HAM_CASE(VK_ERROR_OUT_OF_POOL_MEMORY)
		HAM_CASE(VK_ERROR_INVALID_EXTERNAL_HANDLE)
		HAM_CASE(VK_ERROR_FRAGMENTATION)
		HAM_CASE(VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS)
		HAM_CASE(VK_PIPELINE_COMPILE_REQUIRED)
		HAM_CASE(VK_ERROR_SURFACE_LOST_KHR)
		HAM_CASE(VK_ERROR_NATIVE_WINDOW_IN_USE_KHR)
		HAM_CASE(VK_SUBOPTIMAL_KHR)
		HAM_CASE(VK_ERROR_OUT_OF_DATE_KHR)
		HAM_CASE(VK_ERROR_INCOMPATIBLE_DISPLAY_KHR)
		HAM_CASE(VK_ERROR_VALIDATION_FAILED_EXT)
		HAM_CASE(VK_ERROR_INVALID_SHADER_NV)

	#ifdef VK_ENABLE_BETA_EXTENSIONS
		HAM_CASE(VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR)
		HAM_CASE(VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR)
		HAM_CASE(VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR)
		HAM_CASE(VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR)
		HAM_CASE(VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR)
		HAM_CASE(VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR)
	#endif

		HAM_CASE(VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT)
		HAM_CASE(VK_ERROR_NOT_PERMITTED_KHR)
		HAM_CASE(VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT)
		HAM_CASE(VK_THREAD_IDLE_KHR)
		HAM_CASE(VK_THREAD_DONE_KHR)
		HAM_CASE(VK_OPERATION_DEFERRED_KHR)
		HAM_CASE(VK_OPERATION_NOT_DEFERRED_KHR)
		HAM_CASE(VK_ERROR_COMPRESSION_EXHAUSTED_EXT)

	#undef HAM_CASE

		default: return "UNKNOWN";
	}
}

bool ham_engine_client_init_vulkan(ham_engine_client *engine){
	ham_u32 vk_ext_count;
	if(!SDL_Vulkan_GetInstanceExtensions(engine->window, &vk_ext_count, nullptr)){
		ham_logapierrorf("Error in SDL_Vulkan_GetInstanceExtensions: %s", SDL_GetError());
		return false;
	}

	const char *enabled_exts[] = {
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME
	};

	const auto vk_ext_strs = (const char**)ham_allocator_alloc(ham_super(engine)->allocator, alignof(void*), sizeof(void*) * (vk_ext_count + std::size(enabled_exts)));
	if(!vk_ext_strs){
		ham_logapierrorf("Error allocating %u Vulkan extension string pointers", vk_ext_count);
		return false;
	}

	if(!SDL_Vulkan_GetInstanceExtensions(engine->window, &vk_ext_count, vk_ext_strs)){
		ham_logapierrorf("Error in SDL_Vulkan_GetInstanceExtensions: %s", SDL_GetError());
		ham_allocator_free(ham_super(engine)->allocator, vk_ext_strs);
		return false;
	}

	memcpy(vk_ext_strs + vk_ext_count, enabled_exts, sizeof(enabled_exts));
	vk_ext_count += (ham_u32)std::size(enabled_exts);

	VkApplicationInfo app_info;

	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pNext = nullptr;
	app_info.pApplicationName = "@APPLICATION_NAME@";
	app_info.applicationVersion = VK_MAKE_VERSION(0, 0, 0);
	app_info.pEngineName   = "Ham World Engine";
	app_info.engineVersion = VK_MAKE_VERSION(HAM_ENGINE_VERSION_MAJOR, HAM_ENGINE_VERSION_MINOR, HAM_ENGINE_VERSION_PATCH);
	app_info.apiVersion = VK_API_VERSION_1_1;

	const char *layers[] = {
		"VK_LAYER_KHRONOS_validation"
	};

	const bool layers_enabled =
	#ifndef NDEBUG
		true;
	#else
		false;
	#endif

	VkInstanceCreateInfo inst_info;

	inst_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	inst_info.pNext = nullptr;
	inst_info.flags = 0;
	inst_info.pApplicationInfo = &app_info;
	inst_info.enabledLayerCount = layers_enabled ? (ham_u32)std::size(layers) : 0;
	inst_info.ppEnabledLayerNames = layers_enabled ? layers : nullptr;
	inst_info.enabledExtensionCount = vk_ext_count;
	inst_info.ppEnabledExtensionNames = vk_ext_strs;

#define ham_impl_load_vk_fn(name) engine->name = (PFN_##name)engine->vkGetInstanceProcAddr(engine->vk_inst, #name); \
	if(!engine->name){ \
		ham_logapierrorf("Error in vkGetInstanceProcAddr(\"" #name "\")"); \
		ham_allocator_free(ham_super(engine)->allocator, vk_ext_strs); \
		if(engine->vk_inst && engine->vkDestroyInstance){ \
			engine->vkDestroyInstance(engine->vk_inst, nullptr); \
			engine->vk_inst = nullptr; \
		} \
		return false; \
	}

	ham_impl_load_vk_fn(vkCreateInstance)

	const auto res = engine->vkCreateInstance(&inst_info, nullptr, &engine->vk_inst);
	if(res != VK_SUCCESS){
		ham_logapierrorf("Error in vkCreateInstance: %s", vk_result_to_str(res));
		ham_allocator_free(ham_super(engine)->allocator, vk_ext_strs);
		return false;
	}

	ham_impl_load_vk_fn(vkDestroyInstance)
	ham_impl_load_vk_fn(vkDestroySurfaceKHR)

#undef ham_impl_load_vk_fn

	if(!SDL_Vulkan_CreateSurface(engine->window, engine->vk_inst, &engine->vk_surface)){
		ham_logapierrorf("Error in SDL_Vulkan_CreateSurface: %s", SDL_GetError());
		ham_allocator_free(ham_super(engine)->allocator, vk_ext_strs);
		engine->vkDestroyInstance(engine->vk_inst, nullptr);
		engine->vk_inst = nullptr;
		return false;
	}

	ham_allocator_free(ham_super(engine)->allocator, vk_ext_strs);
	return true;
}

static inline bool ham_engine_client_init(ham_engine *engine_base){
	const auto engine = (ham_engine_client*)engine_base;

	if(SDL_Vulkan_LoadLibrary(nullptr) != 0){
		ham_logapierrorf("Error in SDL_Vulkan_LoadLibrary: %s", SDL_GetError());
		return false;
	}

	engine->vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)SDL_Vulkan_GetVkGetInstanceProcAddr();
	if(!engine->vkGetInstanceProcAddr){
		ham_logapierrorf("Error in SDL_Vulkan_GetVkGetInstanceProcAddr: %s", SDL_GetError());
		return false;
	}

	engine->window = SDL_CreateWindow(
		HAM_ENGINE_CLIENT_API_NAME,
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		1024, 768,
		SDL_WINDOW_VULKAN | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
	);
	if(!engine->window){
		ham_logapierrorf("Error in SDL_CreateWindow: %s", SDL_GetError());
		return false;
	}

	ham_logapiverbosef("Created window 1024x768");

	if(!ham_engine_client_init_vulkan(engine)){
		ham_logapierrorf("Error initializing Vulkan");
		SDL_DestroyWindow(engine->window);
		return false;
	}

	engine->renderer = ham_renderer_create(
		HAM_RENDERER_VULKAN_PLUGIN_NAME,
		HAM_RENDERER_VULKAN_OBJECT_NAME,
		engine->vk_inst, engine->vk_surface,
		engine->vkGetInstanceProcAddr
	);
	if(!engine->renderer){
		ham_logapierrorf("Error creating renderer");
		engine->vkDestroySurfaceKHR(engine->vk_inst, engine->vk_surface, nullptr);
		engine->vkDestroyInstance(engine->vk_inst, nullptr);
		SDL_DestroyWindow(engine->window);
		return false;
	}

	return true;
}

static inline void ham_engine_client_fini(ham_engine *engine_base){
	const auto engine = (ham_engine_client*)engine_base;

	ham_renderer_destroy(engine->renderer);

	engine->vkDestroySurfaceKHR(engine->vk_inst, engine->vk_surface, nullptr);
	engine->vkDestroyInstance(engine->vk_inst, nullptr);
	SDL_DestroyWindow(engine->window);
}

static inline void ham_engine_client_loop(ham_engine *engine_base, ham_f64 dt){
	const auto engine = (ham_engine_client*)engine_base;

	ham_renderer_loop(engine->renderer, dt);

	SDL_Event ev;
	while(SDL_PollEvent(&ev)){
		if(ev.type == SDL_QUIT){
			ham_engine_request_exit(engine_base);
		}
	}
}

ham_define_object_x(
	2, ham_engine_client,
	1, ham_engine_vtable,
	ham_engine_client_construct,
	ham_engine_client_destroy,
	(
		.init = ham_engine_client_init,
		.fini = ham_engine_client_fini,
		.loop = ham_engine_client_loop,
	)
)

struct ham_engine_client_render_subsys{
	ham_renderer *renderer;
};

class net_subsystem: public ham::engine::subsys_base<net_subsystem>{
	public:
		net_subsystem(ham_engine *engine)
			: subsys_base(engine, "client-net"){}

	protected:
		bool init(ham_engine *engine){
			set_min_dt(1.f/60.f);

			m_net = ham_net_create(HAM_NET_STEAMWORKS_PLUGIN_NAME, HAM_NET_STEAMWORKS_OBJECT_NAME);
			if(!m_net){
				ham_loginfof("ham-client-net", "Error creating ham_net");
				return false;
			}

			ham_loginfof("ham-client-net", "Net subsystem initialized");
			return true;
		}

		void fini(ham_engine *engine) noexcept{
			ham_net_destroy(m_net);
		}

		void loop(ham_engine *engine, f64 dt){
			ham_net_loop(m_net, dt);
		}

	private:
		ham_net *m_net;

		friend class ham::engine::subsys_base<net_subsystem>;
};

int main(int argc, char *argv[]){
	const auto engine = ham_engine_create(HAM_ENGINE_CLIENT_API_NAME, HAM_ENGINE_CLIENT_OBJ_NAME, argc, argv);

	net_subsystem net_subsys{engine};

	return ham_engine_exec(engine);
}
