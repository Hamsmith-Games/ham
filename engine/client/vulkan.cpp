/*
 * The Ham World Engine Client
 * Copyright (C) 2022 Keith Hammond
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

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN 1
#define VK_USE_PLATFORM_WIN32_KHR 1
#endif

#include "client.h"

#include "ham/log.h"
#include "ham/buffer.h"

#define VMA_IMPLEMENTATION

using namespace ham::typedefs;

namespace engine = ham::engine;

HAM_C_API_BEGIN

VkInstance ham_engine_client_init_vk_inst(SDL_Window *window, const ham_engine_app *app, PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr){
	ham_u32 ext_count;
	if(!SDL_Vulkan_GetInstanceExtensions(window, &ext_count, nullptr)){
		ham::logapierror("Error in SDL_Vulkan_GetInstanceExtensions: {}", SDL_GetError());
		return VK_NULL_HANDLE;
	}

	const char *enabled_exts[] = {
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME
	};

	ham::basic_buffer<const char*> exts;

	if(!exts.resize(ext_count + std::size(enabled_exts))){
		ham::logapierror("Error allocating {} Vulkan extension string pointers", ext_count);
		return VK_NULL_HANDLE;
	}

	if(!SDL_Vulkan_GetInstanceExtensions(window, &ext_count, exts.data())){
		ham::logapierror("Error in SDL_Vulkan_GetInstanceExtensions: {}", SDL_GetError());
		return VK_NULL_HANDLE;
	}

	for(usize i = 0; i < std::size(enabled_exts); i++){
		exts[ext_count + i] = enabled_exts[i];
	}

	ext_count += (u32)std::size(enabled_exts);

	const char *layers[] = {
		"VK_LAYER_KHRONOS_validation"
	};

	constexpr bool layers_enabled =
	#ifndef NDEBUG
		true;
	#else
		false;
	#endif

	const ham::str_buffer_utf8 name_buf = app->name;

	VkApplicationInfo app_info{
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pNext = nullptr,
		.pApplicationName = name_buf.c_str(),
		.applicationVersion = VK_MAKE_VERSION(app->version.major, app->version.minor, app->version.patch),
		.pEngineName = "ham-engine",
		.engineVersion = VK_MAKE_VERSION(HAM_ENGINE_VERSION_MAJOR, HAM_ENGINE_VERSION_MINOR, HAM_ENGINE_VERSION_PATCH),
		.apiVersion = VK_API_VERSION_1_2,
	};

	VkInstanceCreateInfo inst_info{
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.pApplicationInfo = &app_info,
		.enabledLayerCount = layers_enabled ? (ham_u32)std::size(layers) : 0,
		.ppEnabledLayerNames = layers_enabled ? layers : nullptr,
		.enabledExtensionCount = ext_count,
		.ppEnabledExtensionNames = exts.data(),
	};

	VkInstance ret = VK_NULL_HANDLE;

	const auto vkCreateInstance = (PFN_vkCreateInstance)vkGetInstanceProcAddr(nullptr, "vkCreateInstance");
	if(!vkCreateInstance){
		ham::logapierror("Failed to load function 'vkCreateInstance'");
		return VK_NULL_HANDLE;
	}

	VkResult res = vkCreateInstance(&inst_info, nullptr, &ret);
	if(res != VK_SUCCESS){
		ham::logapierror("Error in vkCreateInstance: {}", ham_vk_result_str(res));
		return VK_NULL_HANDLE;
	}

	return ret;
}


//
// Vulkan window
//

engine::client_window_vulkan::client_window_vulkan(const ham_engine_app *app, Uint32 flags)
	: client_window(app, flags | SDL_WINDOW_VULKAN)
{
	m_vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)SDL_Vulkan_GetVkGetInstanceProcAddr();

	m_vk_inst = ham_engine_client_init_vk_inst(window_handle(), app, m_vkGetInstanceProcAddr);
	if(m_vk_inst == VK_NULL_HANDLE){
		throw std::runtime_error("Error in ham_engine_client_init_vk_inst");
	}

	if(!SDL_Vulkan_CreateSurface(window_handle(), m_vk_inst, &m_vk_surface)){
		const auto vkDestroyInstance = (PFN_vkDestroyInstance)m_vkGetInstanceProcAddr(m_vk_inst, "vkDestroyInstance");
		vkDestroyInstance(m_vk_inst, nullptr);
		throw std::runtime_error(fmt::format("Error in SDL_Vulkan_CreateSurface: {}", SDL_GetError()));
	}

	m_r_frame_data.common.current_frame = 0;
}

engine::client_window_vulkan::~client_window_vulkan(){
	if(!m_vk_inst) return;

	const auto vkDestroyInstance = (PFN_vkDestroyInstance)m_vkGetInstanceProcAddr(m_vk_inst, "vkDestroyInstance");

	vkDestroyInstance(m_vk_inst, nullptr);
}

void engine::client_window_vulkan::present(f64 dt, ham_renderer *r) const{
	ham_renderer_frame(r, dt, &m_r_frame_data);
	++m_r_frame_data.common.current_frame;
}

HAM_C_API_END
