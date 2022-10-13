/*
 * The Ham World Engine Client
 * Copyright (C) 2022 Hamsmith Ltd.
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

VkInstance ham_engine_client_init_vk_inst(SDL_Window *window, const ham_engine_app *app, ham_vk_fns *fns){
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

	fns->vkCreateInstance = (PFN_vkCreateInstance)fns->vkGetInstanceProcAddr(nullptr, "vkCreateInstance");
	if(!fns->vkCreateInstance){
		ham::logapierror("Failed to load function 'vkCreateInstance'");
		return VK_NULL_HANDLE;
	}

	fns->vkDestroyInstance = (PFN_vkDestroyInstance)fns->vkGetInstanceProcAddr(nullptr, "vkDestroyInstance");
	if(!fns->vkDestroyInstance){
		ham::logapierror("Failed to load function 'vkDestroyInstance'");
		return VK_NULL_HANDLE;
	}

	VkResult res = fns->vkCreateInstance(&inst_info, nullptr, &ret);
	if(res != VK_SUCCESS){
		ham::logapierror("Error in vkCreateInstance: {}", ham_vk_result_str(res));
		return VK_NULL_HANDLE;
	}

#define LOAD_INSTANCE_FN(name) \
	fns->name = (PFN_##name)fns->vkGetInstanceProcAddr(ret, #name);\
	if(!fns->name){\
		ham::logapierror("Failed to load function '" #name "'"); \
		return VK_NULL_HANDLE; \
	}

	// TODO: load physical device functions

	LOAD_INSTANCE_FN(vkGetDeviceProcAddr)

	LOAD_INSTANCE_FN(vkEnumeratePhysicalDevices)
	LOAD_INSTANCE_FN(vkGetPhysicalDeviceProperties)
	LOAD_INSTANCE_FN(vkGetPhysicalDeviceFeatures)
	LOAD_INSTANCE_FN(vkGetPhysicalDeviceQueueFamilyProperties)
	LOAD_INSTANCE_FN(vkGetPhysicalDeviceSurfaceCapabilitiesKHR)
	LOAD_INSTANCE_FN(vkGetPhysicalDeviceSurfaceFormatsKHR)
	LOAD_INSTANCE_FN(vkGetPhysicalDeviceSurfacePresentModesKHR)
	LOAD_INSTANCE_FN(vkGetPhysicalDeviceSurfaceSupportKHR)

	LOAD_INSTANCE_FN(vkCreateDevice)
	LOAD_INSTANCE_FN(vkDestroyDevice)

#undef LOAD_INSTANCE_FN

	return ret;
}

VkPhysicalDevice ham_engine_client_init_vk_phys(
	VkInstance vk_inst, VkSurfaceKHR vk_surface, ham_vk_fns *fns,
	ham_u32 *ret_graphics_family, ham_u32 *ret_present_family
){
	u32 num_devs = 0;

	VkResult res = fns->vkEnumeratePhysicalDevices(vk_inst, &num_devs, nullptr);
	if(res != VK_SUCCESS){
		ham::logapierror("Error in vkEnumeratePhysicalDevices: {}", ham_vk_result_str(res));
		return VK_NULL_HANDLE;
	}
	else if(num_devs == 0){
		ham::logapierror("No devices found");
		return VK_NULL_HANDLE;
	}

	ham::basic_buffer<VkPhysicalDevice> devices;
	devices.resize(num_devs);

	res = fns->vkEnumeratePhysicalDevices(vk_inst, &num_devs, devices.data());
	if(res != VK_SUCCESS){
		ham::logapierror("Error in vkEnumeratePhysicalDevices: {}", ham_vk_result_str(res));
		return VK_NULL_HANDLE;
	}

	VkPhysicalDeviceProperties selected_props{}, props{};
	VkPhysicalDeviceFeatures selected_feats{}, feats{};

	VkPhysicalDevice selected = nullptr;
	u32 selected_graphics = 0;
	u32 selected_present = 0;

	for(u32 i = 0; i < num_devs; i++){
		const auto dev = devices[i];

		fns->vkGetPhysicalDeviceProperties(dev, &props);
		fns->vkGetPhysicalDeviceFeatures(dev, &feats);

		if(selected && selected_props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU){
			if(props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU){
				// TODO: not just compare framebuffer total size

				const ham_usize largest_fb =
					props.limits.maxFramebufferWidth *
					props.limits.maxFramebufferHeight *
					props.limits.maxFramebufferLayers;

				const ham_usize best_large_fb =
					selected_props.limits.maxFramebufferWidth *
					selected_props.limits.maxFramebufferHeight *
					selected_props.limits.maxFramebufferLayers;

				if(largest_fb < best_large_fb) continue;
			}
			else if(props.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU){
				// try to be discrete
				continue;
			}
		}

		ham_u32 num_queues = 0;
		fns->vkGetPhysicalDeviceQueueFamilyProperties(dev, &num_queues, nullptr);

		if(num_queues == 0){
			continue;
		}

		VkSurfaceCapabilitiesKHR caps;
		fns->vkGetPhysicalDeviceSurfaceCapabilitiesKHR(dev, vk_surface, &caps);

		ham::basic_buffer<VkQueueFamilyProperties> family_props(num_queues);
		fns->vkGetPhysicalDeviceQueueFamilyProperties(dev, &num_queues, family_props.data());

		ham_u32 graphics_family = (ham_u32)-1;
		ham_u32 present_family = (ham_u32)-1;

		for(ham_u32 i = 0; i < num_queues; i++){
			const auto &props = family_props[i];
			if(props.queueFlags & VK_QUEUE_GRAPHICS_BIT){
				graphics_family = i;

				VkBool32 present_support = false;
				fns->vkGetPhysicalDeviceSurfaceSupportKHR(dev, i, vk_surface, &present_support);

				if(present_support){
					present_family = i;
				}
			}
		}

		if(graphics_family == (ham_u32)-1){
			ham::logapidebug("No graphics support found for device: {}", props.deviceName);
			continue;
		}
		else if(present_family == (ham_u32)-1){
			ham::logapidebug("No present support found for device: {}", props.deviceName);
			continue;
		}

		selected = dev;
		selected_props = props;
		selected_feats = feats;
		selected_graphics = graphics_family;
		selected_present = present_family;
	}

	if(!selected){
		ham::logapierror("No physical device with VK_QUEUE_GRAPHICS_BIT and present support found");
		return VK_NULL_HANDLE;
	}

	*ret_graphics_family = selected_graphics;
	*ret_present_family = selected_present;

	return selected;
}

VkDevice ham_engine_client_init_vk_device(
	VkPhysicalDevice vk_phys, ham_vk_fns *fns,
	ham_u32 graphics_family, ham_u32 present_family,
	VkQueue *graphics_queue_ret, VkQueue *present_queue_ret
){
	const float graphics_priority = 1.f;

	VkDeviceQueueCreateInfo queue_infos[2];

	auto &&graphics_queue_info = queue_infos[0];
	graphics_queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	graphics_queue_info.pNext = nullptr;
	graphics_queue_info.flags = 0;
	graphics_queue_info.queueFamilyIndex = graphics_family;
	graphics_queue_info.queueCount = 1;
	graphics_queue_info.pQueuePriorities = &graphics_priority;

	VkDeviceQueueCreateInfo present_queue_info;
	if(graphics_family != present_family){
		auto &&present_queue_info = queue_infos[1];
		present_queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		present_queue_info.pNext = nullptr;
		present_queue_info.queueFamilyIndex = present_family;
		present_queue_info.queueCount = 1;
		present_queue_info.pQueuePriorities = &graphics_priority;
	}

	VkPhysicalDeviceExtendedDynamicStateFeaturesEXT dyn_state_features{
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT,
		.pNext = nullptr,
		.extendedDynamicState = VK_TRUE,
	};

	VkPhysicalDeviceFeatures2 features;
	memset(&features, 0, sizeof(features));

	features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	features.pNext = &dyn_state_features;

	const char *dev_exts[] = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	VkDeviceCreateInfo dev_info;
	dev_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	dev_info.pNext = &features;
	dev_info.flags = 0;
	dev_info.queueCreateInfoCount = 1 + (graphics_family != present_family);
	dev_info.pQueueCreateInfos = queue_infos;
	dev_info.enabledLayerCount = 0;
	dev_info.ppEnabledLayerNames = nullptr;
	dev_info.enabledExtensionCount = (ham_u32)std::size(dev_exts);
	dev_info.ppEnabledExtensionNames = dev_exts;
	dev_info.pEnabledFeatures = nullptr;

	VkDevice ret = nullptr;

	const auto res = fns->vkCreateDevice(vk_phys, &dev_info, nullptr, &ret);
	if(res != VK_SUCCESS){
		ham::logapierror("Error in vkCreateDevice: {}", ham_vk_result_str(res));
		return VK_NULL_HANDLE;
	}

#define LOAD_DEVICE_FN(name) \
	fns->name = (PFN_##name)fns->vkGetDeviceProcAddr(ret, #name); \
	if(!fns->name){ \
		ham::logapierror("Failed to get device function '" #name "'"); \
		fns->vkDestroyDevice(ret, nullptr); \
		return VK_NULL_HANDLE; \
	}

	// TODO: load device fns

	LOAD_DEVICE_FN(vkDeviceWaitIdle)
	LOAD_DEVICE_FN(vkGetDeviceQueue)
	LOAD_DEVICE_FN(vkQueueSubmit)
	LOAD_DEVICE_FN(vkQueuePresentKHR)

	LOAD_DEVICE_FN(vkCreateSwapchainKHR)
	LOAD_DEVICE_FN(vkDestroySwapchainKHR)
	LOAD_DEVICE_FN(vkGetSwapchainImagesKHR)
	LOAD_DEVICE_FN(vkAcquireNextImageKHR)

	LOAD_DEVICE_FN(vkCreateImageView)
	LOAD_DEVICE_FN(vkDestroyImageView)

	LOAD_DEVICE_FN(vkCreateShaderModule)
	LOAD_DEVICE_FN(vkDestroyShaderModule)

	LOAD_DEVICE_FN(vkCreateRenderPass)
	LOAD_DEVICE_FN(vkDestroyRenderPass)

	LOAD_DEVICE_FN(vkCreatePipelineLayout)
	LOAD_DEVICE_FN(vkDestroyPipelineLayout)
	LOAD_DEVICE_FN(vkCreateGraphicsPipelines)
	LOAD_DEVICE_FN(vkDestroyPipeline)

	LOAD_DEVICE_FN(vkCreateDescriptorSetLayout)
	LOAD_DEVICE_FN(vkDestroyDescriptorSetLayout)

	LOAD_DEVICE_FN(vkCreateFramebuffer)
	LOAD_DEVICE_FN(vkDestroyFramebuffer)

	LOAD_DEVICE_FN(vkCreateCommandPool)
	LOAD_DEVICE_FN(vkDestroyCommandPool)

	LOAD_DEVICE_FN(vkAllocateCommandBuffers)
	LOAD_DEVICE_FN(vkFreeCommandBuffers)
	LOAD_DEVICE_FN(vkBeginCommandBuffer)
	LOAD_DEVICE_FN(vkEndCommandBuffer)
	LOAD_DEVICE_FN(vkResetCommandBuffer)

	LOAD_DEVICE_FN(vkCmdPipelineBarrier)
	LOAD_DEVICE_FN(vkCmdBeginRenderPass)
	LOAD_DEVICE_FN(vkCmdEndRenderPass)
	LOAD_DEVICE_FN(vkCmdBindPipeline)
	LOAD_DEVICE_FN(vkCmdBindVertexBuffers)
	LOAD_DEVICE_FN(vkCmdBindIndexBuffer)
	LOAD_DEVICE_FN(vkCmdSetViewport)
	LOAD_DEVICE_FN(vkCmdSetScissor)
	LOAD_DEVICE_FN(vkCmdDraw)
	LOAD_DEVICE_FN(vkCmdDrawIndexed)
	LOAD_DEVICE_FN(vkCmdDrawIndexedIndirect)

	LOAD_DEVICE_FN(vkCreateSemaphore)
	LOAD_DEVICE_FN(vkDestroySemaphore)
	LOAD_DEVICE_FN(vkCreateFence)
	LOAD_DEVICE_FN(vkDestroyFence)

	LOAD_DEVICE_FN(vkWaitForFences)
	LOAD_DEVICE_FN(vkResetFences)

#undef LOAD_DEVICE_FN

	fns->vkGetDeviceQueue(ret, graphics_family, 0, graphics_queue_ret);
	fns->vkGetDeviceQueue(ret, present_family, 0, present_queue_ret);

	return ret;
}

VmaAllocator ham_engine_client_init_vma(VkInstance vk_inst, VkPhysicalDevice vk_phys, VkDevice vk_dev){
	VmaAllocatorCreateInfo allocator_info{};
	allocator_info.physicalDevice = vk_phys;
	allocator_info.device = vk_dev;
	allocator_info.instance = vk_inst;
	allocator_info.vulkanApiVersion = VK_API_VERSION_1_2;

	VmaAllocator ret = VK_NULL_HANDLE;

	VkResult res = vmaCreateAllocator(&allocator_info, &ret);
	if(res != VK_SUCCESS){
		ham::logapierror("Error in vmaCreateAllocator: {}", ham_vk_result_str(res));
		return VK_NULL_HANDLE;
	}

	return ret;
}

VkSwapchainKHR ham_engine_client_init_vk_swapchain(
	VkPhysicalDevice vk_phys, VkSurfaceKHR vk_surface, VkDevice vk_dev, const ham_vk_fns *fns,
	ham_u32 graphics_family, ham_u32 present_family,
	VkSurfaceCapabilitiesKHR *surface_caps_ret,
	VkSurfaceFormatKHR *surface_format_ret, VkPresentModeKHR *present_mode_ret,
	ham_u32 *image_count_ret
){
	ham::basic_buffer<VkPresentModeKHR> present_modes;
	ham::basic_buffer<VkSurfaceFormatKHR> surface_formats;

	VkResult res = fns->vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vk_phys, vk_surface, surface_caps_ret);
	if(res != VK_SUCCESS){
		ham::logapierror("Error in vkGetPhysicalDeviceSurfaceCapabilitiesKHR: {}", ham_vk_result_str(res));
		return VK_NULL_HANDLE;
	}

	u32 num_formats = 0;
	res = fns->vkGetPhysicalDeviceSurfaceFormatsKHR(vk_phys, vk_surface, &num_formats, nullptr);
	if(res != VK_SUCCESS){
		ham::logapierror("Error in vkGetPhysicalDeviceSurfaceFormatsKHR: {}", ham_vk_result_str(res));
		return VK_NULL_HANDLE;
	}
	else if(num_formats == 0){
		ham::logapierror("No device vk_surface formats found");
		return VK_NULL_HANDLE;
	}

	surface_formats.resize(num_formats);

	res = fns->vkGetPhysicalDeviceSurfaceFormatsKHR(vk_phys, vk_surface, &num_formats, surface_formats.data());
	if(res != VK_SUCCESS){
		ham::logapierror("Error in vkGetPhysicalDeviceSurfaceFormatsKHR: {}", ham_vk_result_str(res));
		return VK_NULL_HANDLE;
	}

	u32 num_modes = 0;
	res = fns->vkGetPhysicalDeviceSurfacePresentModesKHR(vk_phys, vk_surface, &num_modes, nullptr);
	if(res != VK_SUCCESS){
		ham::logapierror("Error in vkGetPhysicalDeviceSurfacePresentModesKHR: {}", ham_vk_result_str(res));
		return VK_NULL_HANDLE;
	}
	else if(num_modes == 0){
		ham::logapierror("No device surface present modes found");
		return VK_NULL_HANDLE;
	}

	present_modes.resize(num_modes);

	res = fns->vkGetPhysicalDeviceSurfacePresentModesKHR(vk_phys, vk_surface, &num_modes, present_modes.data());
	if(res != VK_SUCCESS){
		ham::logapierror("Error in vkGetPhysicalDeviceSurfacePresentModesKHR: {}", ham_vk_result_str(res));
		return VK_NULL_HANDLE;
	}

	// Now look for best format

	VkSurfaceFormatKHR desired_format;
	desired_format.format = VK_FORMAT_A2R10G10B10_UNORM_PACK32;
	desired_format.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

	VkSurfaceFormatKHR best_format;
	best_format.format = VK_FORMAT_B8G8R8A8_SRGB;
	best_format.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

	for(const auto &format : surface_formats){
		ham::logapidebug("Found swapchain format: {} {}", ham_vk_format_str(format.format), ham_vk_color_space_str(format.colorSpace));

		if(format.format == desired_format.format && format.colorSpace == desired_format.colorSpace){
			best_format = desired_format;
			break;
		}
	}

	VkPresentModeKHR desired_mode = VK_PRESENT_MODE_FIFO_RELAXED_KHR;
	VkPresentModeKHR best_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;

	for(auto mode : present_modes){
		if(mode == desired_mode){
			best_mode = desired_mode;
			break;
		}
		else if(mode > best_mode && mode <= desired_mode){
			best_mode = mode;
		}
	}

	const ham_u32 image_count = (surface_caps_ret->maxImageCount > 0) ?
		ham_min(surface_caps_ret->minImageCount + 1, surface_caps_ret->maxImageCount) :
		surface_caps_ret->minImageCount + 1;

	const ham_u32 queue_fams[] = { graphics_family, present_family };

	VkSwapchainCreateInfoKHR create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	create_info.pNext = nullptr;
	create_info.flags = 0;
	create_info.surface = vk_surface;
	create_info.minImageCount = image_count;
	create_info.imageFormat = best_format.format;
	create_info.imageColorSpace = best_format.colorSpace;
	create_info.imageExtent = surface_caps_ret->currentExtent;
	create_info.imageArrayLayers = 1;
	create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	create_info.queueFamilyIndexCount = 1 + (graphics_family != present_family);
	create_info.pQueueFamilyIndices = queue_fams;
	create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	create_info.compositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
	create_info.presentMode = best_mode;
	create_info.clipped = VK_TRUE;
	create_info.oldSwapchain = VK_NULL_HANDLE;

	VkSwapchainKHR ret = VK_NULL_HANDLE;

	res = fns->vkCreateSwapchainKHR(vk_dev, &create_info, nullptr, &ret);
	if(res != VK_SUCCESS){
		ham::logapierror("Error in vkCreateSwapchainKHR: {}", ham_vk_result_str(res));
		return VK_NULL_HANDLE;
	}

	res = fns->vkGetSwapchainImagesKHR(vk_dev, ret, image_count_ret, nullptr);
	if(res != VK_SUCCESS){
		ham::logapierror("Error in vkGetSwapchainImagesKHR: {}", ham_vk_result_str(res));
		fns->vkDestroySwapchainKHR(vk_dev, ret, nullptr);
		return VK_NULL_HANDLE;
	}

	*surface_format_ret = best_format;
	*present_mode_ret = best_mode;

	return ret;
}

bool ham_engine_client_init_vk_swapchain_images(
	VkDevice vk_dev, VmaAllocator vma, VkSwapchainKHR vk_swapchain, const ham_vk_fns *fns,
	const VkSurfaceCapabilitiesKHR *surface_caps, VkFormat surface_format,
	ham_u32 num_swapchain_images,
	VkImage *const images_ret,
	VkImageView *const image_views_ret,
	VkImage *const depth_ret,
	VkImageView *const depth_view_ret,
	VmaAllocation *const depth_alloc_ret
){
	ham::basic_buffer<VkImage> images(num_swapchain_images);
	ham::basic_buffer<VkImageView> image_views(num_swapchain_images);

	VkResult res = fns->vkGetSwapchainImagesKHR(vk_dev, vk_swapchain, nullptr, images.data());
	if(res != VK_SUCCESS){
		ham::logapierror("Error in vkGetSwapchainImagesKHR: {}", ham_vk_result_str(res));
		return false;
	}

	VkImageCreateInfo depth_info{
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.imageType = VK_IMAGE_TYPE_2D,
		.format = VK_FORMAT_D32_SFLOAT_S8_UINT,
		.extent = VkExtent3D{ .width = surface_caps->currentExtent.width, .height = surface_caps->currentExtent.height, .depth = 1 },
		.mipLevels = 1,
		.arrayLayers = 1,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = 0,
		.pQueueFamilyIndices = nullptr,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
	};

	VmaAllocationCreateInfo alloc_info{
		.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
		.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
		.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
	};

	VkImage depth_img = VK_NULL_HANDLE;
	VmaAllocation depth_alloc = VK_NULL_HANDLE;

	res = vmaCreateImage(vma, &depth_info, &alloc_info, &depth_img, &depth_alloc, nullptr);
	if(res != VK_SUCCESS){
		ham::logapierror("Error in vmaCreateImage: {}", ham_vk_result_str(res));
		return false;
	}

	VkImageViewCreateInfo view_create_info = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, nullptr, 0 };

	view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	view_create_info.components = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY };

	view_create_info.subresourceRange.baseMipLevel = 0;
	view_create_info.subresourceRange.levelCount = 1;
	view_create_info.subresourceRange.baseArrayLayer = 0;
	view_create_info.subresourceRange.layerCount = 1;

	view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	view_create_info.image = depth_img;
	view_create_info.format = VK_FORMAT_D32_SFLOAT_S8_UINT;

	VkImageView depth_view;

	res = fns->vkCreateImageView(vk_dev, &view_create_info, nullptr, &depth_view);
	if(res != VK_SUCCESS){
		ham::logapierror("Error in vkCreateImageView: {}", ham_vk_result_str(res));
		vmaDestroyImage(vma, depth_img, depth_alloc);
		return false;
	}

	view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

	for(ham_u32 i = 0; i < num_swapchain_images; i++){
		view_create_info.image = images[i];
		view_create_info.format = surface_format;

		res = fns->vkCreateImageView(vk_dev, &view_create_info, nullptr, &image_views[i]);
		if(res != VK_SUCCESS){
			ham::logapierror("Error in vkCreateImageView: {}", ham_vk_result_str(res));
			for(ham_i64 j = 0; j < i; j++){
				fns->vkDestroyImageView(vk_dev, image_views[j], nullptr);
			}

			fns->vkDestroyImageView(vk_dev, depth_view, nullptr);

			vmaDestroyImage(vma, depth_img, depth_alloc);
			return false;
		}
	}

	memcpy(images_ret, images.data(), sizeof(VkImage) * images.size());
	memcpy(image_views_ret, image_views.data(), sizeof(VkImageView) * images.size());

	*depth_ret = depth_img;
	*depth_alloc_ret = depth_alloc;
	*depth_view_ret = depth_view;

	return true;
}

bool ham_engine_client_init_vk_sync_objects(
	VkDevice vk_dev, const ham_vk_fns *fns,
	ham_u32 max_queued_frames,
	VkSemaphore *const frame_img_sems_ret,
	VkSemaphore *const frame_render_sems_ret,
	VkFence *const frame_fences_ret
){
	static const VkSemaphoreCreateInfo sem_create_info{
		VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, nullptr, 0
	};

	static const VkFenceCreateInfo fence_create_info{
		VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, nullptr, VK_FENCE_CREATE_SIGNALED_BIT
	};

	for(ham_u32 i = 0; i < max_queued_frames; i++){
		const VkResult res = fns->vkCreateSemaphore(vk_dev, &sem_create_info, nullptr, frame_img_sems_ret + i);
		if(res != VK_SUCCESS){
			ham::logapierror("Error in vkCreateSemaphore: {}", ham_vk_result_str(res));

			for(ham_u32 j = 0; j < i; j++){
				fns->vkDestroySemaphore(vk_dev, frame_img_sems_ret[j], nullptr);
			}

			return false;
		}
	}

	for(ham_u32 i = 0; i < max_queued_frames; i++){
		const VkResult res = fns->vkCreateSemaphore(vk_dev, &sem_create_info, nullptr, frame_render_sems_ret + i);
		if(res != VK_SUCCESS){
			ham::logapierror("Error in vkCreateSemaphore: {}", ham_vk_result_str(res));

			for(ham_u32 j = 0; j < i; j++){
				fns->vkDestroySemaphore(vk_dev, frame_render_sems_ret[j], nullptr);
			}

			for(ham_u32 j = 0; j < max_queued_frames; j++){
				fns->vkDestroySemaphore(vk_dev, frame_img_sems_ret[j], nullptr);
			}

			return false;
		}
	}

	for(ham_u32 i = 0; i < max_queued_frames; i++){
		const VkResult res = fns->vkCreateFence(vk_dev, &fence_create_info, nullptr, frame_fences_ret + i);
		if(res != VK_SUCCESS){
			ham::logapierror("Error in vkCreateFence: {}", ham_vk_result_str(res));

			for(ham_u32 j = 0; j < i; j++){
				fns->vkDestroyFence(vk_dev, frame_fences_ret[j], nullptr);
			}

			for(ham_u32 j = 0; j < max_queued_frames; j++){
				fns->vkDestroySemaphore(vk_dev, frame_render_sems_ret[j], nullptr);
				fns->vkDestroySemaphore(vk_dev, frame_img_sems_ret[j], nullptr);
			}

			return false;
		}
	}

	return true;
}

VkCommandPool ham_engine_client_init_vk_command_pool(VkDevice vk_dev, const ham_vk_fns *fns, ham_u32 graphics_family){
	const VkCommandPoolCreateInfo create_info{
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.pNext = nullptr,
		.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		.queueFamilyIndex = graphics_family,
	};

	VkCommandPool ret = VK_NULL_HANDLE;

	const VkResult res = fns->vkCreateCommandPool(vk_dev, &create_info, nullptr, &ret);
	if(res != VK_SUCCESS){
		ham::logapierror("Error in vkCreateCommandPool: {}", ham_vk_result_str(res));
		return VK_NULL_HANDLE;
	}

	return ret;
}

bool ham_engine_client_init_vk_command_buffers(
	VkDevice vk_dev, const ham_vk_fns *fns,
	VkCommandPool vk_cmd_pool, ham_u32 max_queued_frames,
	VkCommandBuffer *const ret
){
	const VkCommandBufferAllocateInfo alloc_info{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.pNext = nullptr,
		.commandPool = vk_cmd_pool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = max_queued_frames,
	};

	const VkResult res = fns->vkAllocateCommandBuffers(vk_dev, &alloc_info, ret);
	if(res != VK_SUCCESS){
		ham::logapierror("Error in vkAllocateCommandBuffers: {}", ham_vk_result_str(res));
		return false;
	}

	return true;
}

//
// Vulkan window
//

engine::client_window_vulkan::client_window_vulkan(const ham_engine_app *app, Uint32 flags)
	: client_window(app, flags | SDL_WINDOW_VULKAN)
{
	m_vk_fns.vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)SDL_Vulkan_GetVkGetInstanceProcAddr();

	m_vk_inst = ham_engine_client_init_vk_inst(window_handle(), app, &m_vk_fns);
	if(m_vk_inst == VK_NULL_HANDLE){
		throw std::runtime_error("Error in ham_engine_client_init_vk_inst");
	}

	if(!SDL_Vulkan_CreateSurface(window_handle(), m_vk_inst, &m_vk_surface)){
		m_vk_fns.vkDestroyInstance(m_vk_inst, nullptr);
		throw std::runtime_error(fmt::format("Error in SDL_Vulkan_CreateSurface: {}", SDL_GetError()));
	}

	m_vk_phys = ham_engine_client_init_vk_phys(m_vk_inst, m_vk_surface, &m_vk_fns, &m_vk_graphics_family, &m_vk_present_family);
	if(m_vk_phys == VK_NULL_HANDLE){
		m_vk_fns.vkDestroySurfaceKHR(m_vk_inst, m_vk_surface, nullptr);
		m_vk_fns.vkDestroyInstance(m_vk_inst, nullptr);
		throw std::runtime_error("Error in ham_engine_client_init_vk_phys");
	}

	m_vk_dev = ham_engine_client_init_vk_device(m_vk_phys, &m_vk_fns, m_vk_graphics_family, m_vk_present_family, &m_vk_graphics_queue, &m_vk_present_queue);
	if(m_vk_dev == VK_NULL_HANDLE){
		m_vk_fns.vkDestroySurfaceKHR(m_vk_inst, m_vk_surface, nullptr);
		m_vk_fns.vkDestroyInstance(m_vk_inst, nullptr);
		throw std::runtime_error("Error in ham_engine_client_init_vk_device");
	}

	m_vma_allocator = ham_engine_client_init_vma(m_vk_inst, m_vk_phys, m_vk_dev);
	if(m_vma_allocator == VK_NULL_HANDLE){
		m_vk_fns.vkDestroyDevice(m_vk_dev, nullptr);
		m_vk_fns.vkDestroySurfaceKHR(m_vk_inst, m_vk_surface, nullptr);
		m_vk_fns.vkDestroyInstance(m_vk_inst, nullptr);
		throw std::runtime_error("Error in ham_engine_client_init_vma");
	}

	m_vk_swapchain = ham_engine_client_init_vk_swapchain(
		m_vk_phys, m_vk_surface, m_vk_dev, &m_vk_fns,
		m_vk_graphics_family, m_vk_present_family,
		&m_vk_surface_caps, &m_vk_surface_format, &m_vk_present_mode,
		&m_num_swapchain_imgs
	);
	if(m_vk_swapchain == VK_NULL_HANDLE){
		vmaDestroyAllocator(m_vma_allocator);
		m_vk_fns.vkDestroyDevice(m_vk_dev, nullptr);
		m_vk_fns.vkDestroySurfaceKHR(m_vk_inst, m_vk_surface, nullptr);
		m_vk_fns.vkDestroyInstance(m_vk_inst, nullptr);
		throw std::runtime_error("Error in ham_engine_client_init_vk_swapchain");
	}

	m_vk_swapchain_imgs.resize(m_num_swapchain_imgs);
	m_vk_swapchain_img_views.resize(m_num_swapchain_imgs);

	if(
		!ham_engine_client_init_vk_swapchain_images(
			m_vk_dev, m_vma_allocator, m_vk_swapchain, &m_vk_fns,
			&m_vk_surface_caps, m_vk_surface_format.format,
			m_num_swapchain_imgs,
			m_vk_swapchain_imgs.data(),
			m_vk_swapchain_img_views.data(),
			&m_vk_depth_img,
			&m_vk_depth_view,
			&m_vk_depth_alloc
		)
	)
	{
		m_vk_fns.vkDestroySwapchainKHR(m_vk_dev, m_vk_swapchain, nullptr);

		vmaDestroyAllocator(m_vma_allocator);
		m_vk_fns.vkDestroyDevice(m_vk_dev, nullptr);
		m_vk_fns.vkDestroySurfaceKHR(m_vk_inst, m_vk_surface, nullptr);
		m_vk_fns.vkDestroyInstance(m_vk_inst, nullptr);
		throw std::runtime_error("Error in ham_engine_client_init_vk_swapchain_images");
	}

	m_max_queued_frames = 3;

	m_vk_frame_cmd_bufs.resize(m_max_queued_frames);

	m_vk_cmd_pool = ham_engine_client_init_vk_command_pool(m_vk_dev, &m_vk_fns, m_vk_graphics_family);
	if(m_vk_cmd_pool == VK_NULL_HANDLE){
		for(u32 i = 0; i < m_num_swapchain_imgs; i++){
			const auto view = m_vk_swapchain_img_views[i];
			m_vk_fns.vkDestroyImageView(m_vk_dev, view, nullptr);
		}

		m_vk_fns.vkDestroyImageView(m_vk_dev, m_vk_depth_view, nullptr);

		vmaDestroyImage(m_vma_allocator, m_vk_depth_img, m_vk_depth_alloc);

		m_vk_fns.vkDestroySwapchainKHR(m_vk_dev, m_vk_swapchain, nullptr);

		vmaDestroyAllocator(m_vma_allocator);
		m_vk_fns.vkDestroyDevice(m_vk_dev, nullptr);
		m_vk_fns.vkDestroySurfaceKHR(m_vk_inst, m_vk_surface, nullptr);
		m_vk_fns.vkDestroyInstance(m_vk_inst, nullptr);
		throw std::runtime_error("Error in ham_engine_client_init_vk_command_pool");
	}

	if(!ham_engine_client_init_vk_command_buffers(m_vk_dev, &m_vk_fns, m_vk_cmd_pool, m_max_queued_frames, m_vk_frame_cmd_bufs.data())){
		m_vk_fns.vkDestroyCommandPool(m_vk_dev, m_vk_cmd_pool, nullptr);

		for(u32 i = 0; i < m_num_swapchain_imgs; i++){
			const auto view = m_vk_swapchain_img_views[i];
			m_vk_fns.vkDestroyImageView(m_vk_dev, view, nullptr);
		}

		m_vk_fns.vkDestroyImageView(m_vk_dev, m_vk_depth_view, nullptr);

		vmaDestroyImage(m_vma_allocator, m_vk_depth_img, m_vk_depth_alloc);

		m_vk_fns.vkDestroySwapchainKHR(m_vk_dev, m_vk_swapchain, nullptr);

		vmaDestroyAllocator(m_vma_allocator);
		m_vk_fns.vkDestroyDevice(m_vk_dev, nullptr);
		m_vk_fns.vkDestroySurfaceKHR(m_vk_inst, m_vk_surface, nullptr);
		m_vk_fns.vkDestroyInstance(m_vk_inst, nullptr);
		throw std::runtime_error("Error in ham_engine_client_init_vk_command_pool");
	}

	m_vk_frame_img_sems.resize(m_max_queued_frames);
	m_vk_frame_render_sems.resize(m_max_queued_frames);
	m_vk_frame_fences.resize(m_max_queued_frames);

	if(
		!ham_engine_client_init_vk_sync_objects(
			m_vk_dev, &m_vk_fns,
			m_max_queued_frames,
			m_vk_frame_img_sems.data(),
			m_vk_frame_render_sems.data(),
			m_vk_frame_fences.data()
		)
	){
		m_vk_fns.vkFreeCommandBuffers(m_vk_dev, m_vk_cmd_pool, m_max_queued_frames, m_vk_frame_cmd_bufs.data());

		m_vk_fns.vkDestroyCommandPool(m_vk_dev, m_vk_cmd_pool, nullptr);

		for(u32 i = 0; i < m_num_swapchain_imgs; i++){
			const auto view = m_vk_swapchain_img_views[i];
			m_vk_fns.vkDestroyImageView(m_vk_dev, view, nullptr);
		}

		m_vk_fns.vkDestroyImageView(m_vk_dev, m_vk_depth_view, nullptr);

		vmaDestroyImage(m_vma_allocator, m_vk_depth_img, m_vk_depth_alloc);

		m_vk_fns.vkDestroySwapchainKHR(m_vk_dev, m_vk_swapchain, nullptr);

		vmaDestroyAllocator(m_vma_allocator);
		m_vk_fns.vkDestroyDevice(m_vk_dev, nullptr);
		m_vk_fns.vkDestroySurfaceKHR(m_vk_inst, m_vk_surface, nullptr);
		m_vk_fns.vkDestroyInstance(m_vk_inst, nullptr);
		throw std::runtime_error("Error in ham_engine_client_init_vk_swapchain_images");
	}

	m_r_frame_data.vulkan.current_frame = 0;
}

engine::client_window_vulkan::~client_window_vulkan(){
	if(!m_vk_inst) return;

	m_vk_fns.vkDeviceWaitIdle(m_vk_dev);

	m_vk_fns.vkFreeCommandBuffers(m_vk_dev, m_vk_cmd_pool, m_max_queued_frames, m_vk_frame_cmd_bufs.data());

	m_vk_fns.vkDestroyCommandPool(m_vk_dev, m_vk_cmd_pool, nullptr);

	for(u32 i = 0; i < m_max_queued_frames; i++){
		const auto img_sem = m_vk_frame_img_sems[i];
		const auto render_sem = m_vk_frame_render_sems[i];
		const auto frame_fence = m_vk_frame_fences[i];

		m_vk_fns.vkDestroySemaphore(m_vk_dev, img_sem, nullptr);
		m_vk_fns.vkDestroySemaphore(m_vk_dev, render_sem, nullptr);
		m_vk_fns.vkDestroyFence(m_vk_dev, frame_fence, nullptr);
	}

	for(u32 i = 0; i < m_num_swapchain_imgs; i++){
		const auto view = m_vk_swapchain_img_views[i];
		m_vk_fns.vkDestroyImageView(m_vk_dev, view, nullptr);
	}

	m_vk_fns.vkDestroyImageView(m_vk_dev, m_vk_depth_view, nullptr);

	vmaDestroyImage(m_vma_allocator, m_vk_depth_img, m_vk_depth_alloc);

	m_vk_fns.vkDestroySwapchainKHR(m_vk_dev, m_vk_swapchain, nullptr);

	vmaDestroyAllocator(m_vma_allocator);

	m_vk_fns.vkDestroyDevice(m_vk_dev, nullptr);
	m_vk_fns.vkDestroySurfaceKHR(m_vk_inst, m_vk_surface, nullptr);
	m_vk_fns.vkDestroyInstance(m_vk_inst, nullptr);
}

void engine::client_window_vulkan::present(f64 dt, ham_renderer *r) const{
	vmaSetCurrentFrameIndex(m_vma_allocator, (u32)m_cur_frame);

	const u32 current_frame_idx = m_cur_frame % m_max_queued_frames;

	// wait for previous frame to finish
	VkResult res = m_vk_fns.vkWaitForFences(m_vk_dev, 1, &m_vk_frame_fences[current_frame_idx], VK_TRUE, UINT64_MAX);
	if(res != VK_SUCCESS){
		ham::logapierror("Error in vkWaitForFences: {}", ham_vk_result_str(res));
		return;
	}

	// acquire swapchain image
	u32 swap_idx;
	res = m_vk_fns.vkAcquireNextImageKHR(m_vk_dev, m_vk_swapchain, UINT64_MAX, m_vk_frame_img_sems[current_frame_idx], VK_NULL_HANDLE, &swap_idx);
	if(res == VK_ERROR_OUT_OF_DATE_KHR){
		//recreate_swapchain();
		return;
	}
	else if(res != VK_SUCCESS && res != VK_SUBOPTIMAL_KHR){
		ham::logapierror("Error in vkAcquireNextImageKHR: {}", ham_vk_result_str(res));
		return;
	}

	// now we have an image we can wait to actually use it
	res = m_vk_fns.vkResetFences(m_vk_dev, 1, &m_vk_frame_fences[current_frame_idx]);
	if(res != VK_SUCCESS){
		ham::logapierror("Error in vkResetFences: {}", ham_vk_result_str(res));
		return;
	}

	// reset the frame command buffer
	res = m_vk_fns.vkResetCommandBuffer(m_vk_frame_cmd_bufs[current_frame_idx], 0);
	if(res != VK_SUCCESS){
		ham::logapierror("Error in vkResetCommandBuffer: {}", ham_vk_result_str(res));
		return;
	}

	m_r_frame_data.vulkan.current_frame = m_cur_frame;
	m_r_frame_data.vulkan.command_buffer = m_vk_frame_cmd_bufs[current_frame_idx];

	ham_renderer_frame(r, dt, &m_r_frame_data);

	constexpr VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	VkSubmitInfo submit_info{
		VK_STRUCTURE_TYPE_SUBMIT_INFO, nullptr,
		1, &m_vk_frame_img_sems[current_frame_idx],
		wait_stages,
		1, &m_vk_frame_cmd_bufs[current_frame_idx],
		1, &m_vk_frame_render_sems[current_frame_idx],
	};

	res = m_vk_fns.vkQueueSubmit(m_vk_graphics_queue, 1, &submit_info, m_vk_frame_fences[current_frame_idx]);
	if(res != VK_SUCCESS){
		ham::logapierror("Error in vkQueueSubmit: {}", ham_vk_result_str(res));
		return;
	}

	VkPresentInfoKHR present_info{
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.pNext = nullptr,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &m_vk_frame_render_sems[current_frame_idx],
		.swapchainCount = 1,
		.pSwapchains = &m_vk_swapchain,
		.pImageIndices = &swap_idx,
		.pResults = nullptr,
	};

	res = m_vk_fns.vkQueuePresentKHR(m_vk_present_queue, &present_info);
	if(res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR || m_swapchain_dirty){
		//recreate_swapchain();
	}
	else if(res != VK_SUCCESS){
		ham::logapierror("Error in vkQueuePresentKHR: {}", ham_vk_result_str(res));
		return;
	}

	++m_cur_frame;
}

HAM_C_API_END
