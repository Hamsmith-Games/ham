/*
 * Ham Runtime Plugins
 * Copyright (C) 2022  Hamsmith Ltd.
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

#ifndef HAM_RENDERER_VULKAN_RENDERER_HPP
#define HAM_RENDERER_VULKAN_RENDERER_HPP 1

#include "ham/renderer-object.h" // IWYU pragma: keep

#include "vulkan.hpp" // IWYU pragma: keep

#include "vk_mem_alloc.h"

HAM_C_API_BEGIN

struct ham_vertex_uniform_data{
	ham_mat4 view_proj;
};

struct ham_fragment_uniform_data{
	float time;
};

typedef struct ham_renderer_vulkan ham_renderer_vulkan;
typedef struct ham_draw_group_vulkan ham_draw_group_vulkan;

constexpr ham_usize ham_impl_max_queued_frames = 3;

struct ham_renderer_vulkan{
	ham_derive(ham_renderer)

	bool swapchain_dirty;

	// TODO: global uniform buffers

	ham_draw_group_vulkan *screen_draw_group;

	VkInstance vk_inst;
	VkSurfaceKHR vk_surface;
	VkPhysicalDevice vk_phys_dev;
	VkDevice vk_dev;

	ham_u32 vk_graphics_family, vk_present_family;

	VkQueue vk_graphics_queue, vk_present_queue;

	VkSwapchainKHR vk_swapchain;
	VkFormat vk_swapchain_format;
	VkExtent2D vk_swapchain_extent;

	ham_u32 vk_num_swapchain_images;
	VkImage *vk_swapchain_images;
	VkImage vk_swapchain_depth;
	VkImageView vk_swapchain_depth_view;
	VmaAllocation vk_swapchain_depth_alloc;
	VkImageView *vk_swapchain_views;
	VkFramebuffer *vk_swapchain_framebuffers;

	VkPipelineLayout vk_pipeline_layout;
	VkDescriptorSetLayout vk_descriptor_set_layout;
	VkRenderPass vk_render_pass_data;
	VkRenderPass vk_render_pass_screen;
	VkPipeline vk_pipeline;

	VkCommandPool vk_cmd_pool;

	VkCommandBuffer vk_cmd_bufs[ham_impl_max_queued_frames];
	VkSemaphore vk_img_sems[ham_impl_max_queued_frames];
	VkSemaphore vk_render_sems[ham_impl_max_queued_frames];
	VkFence vk_frame_fences[ham_impl_max_queued_frames];

	ham_vk_fns vk_fns;

	VkSurfaceCapabilitiesKHR vk_surface_caps;
	VkSurfaceFormatKHR vk_surface_fmt;
	VkPresentModeKHR vk_present_mode;

	VmaVulkanFunctions vma_vk_fns;
	VmaAllocator vma_allocator;

	ham_u32 frame_counter;
};

struct ham_draw_group_vulkan{
	ham_derive(ham_draw_group)

	// TODO: group uniform buffer
	// TODO: instance attribute buffer

	VkBuffer vbo, ibo, cbo;
	VmaAllocation vbo_alloc, ibo_alloc, cbo_alloc;
};

ham_private ham_draw_group_vulkan *ham_draw_group_vulkan_ctor(ham_draw_group_vulkan *mem, ham_u32 nargs, va_list va);

ham_private void ham_draw_group_vulkan_dtor(ham_draw_group_vulkan *group);

ham_private bool ham_draw_group_vulkan_init(
	ham_draw_group_vulkan *group,
	ham_renderer_vulkan *r,
	ham_usize num_shapes, const ham_shape *const *shapes
);

ham_private void ham_draw_group_vulkan_fini(ham_draw_group_vulkan *group);

HAM_C_API_END

#endif // !HAM_RENDERER_VULKAN_RENDERER_HPP
