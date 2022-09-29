#ifndef HAM_RENDERER_VULKAN_RENDERER_HPP
#define HAM_RENDERER_VULKAN_RENDERER_HPP 1

#include "ham/renderer-object.h"

#include "vulkan.hpp"

#include "vk_mem_alloc.h"

HAM_C_API_BEGIN

typedef struct ham_renderer_vulkan ham_renderer_vulkan;
typedef struct ham_draw_group_vulkan ham_draw_group_vulkan;

constexpr ham_usize ham_impl_max_queued_frames = 3;

struct ham_renderer_vulkan{
	ham_derive(ham_renderer)

	bool swapchain_dirty = false;

	VkInstance vk_inst;
	VkSurfaceKHR vk_surface;

	ham_u32 vk_graphics_family, vk_present_family;
	VkPhysicalDevice vk_phys_dev;
	VkDevice vk_dev;

	VkQueue vk_graphics_queue, vk_present_queue;

	VkSwapchainKHR vk_swapchain;
	VkFormat vk_swapchain_format;
	VkExtent2D vk_swapchain_extent;
	ham::std_vector<VkImage> vk_swapchain_images;
	ham::std_vector<VkImageView> vk_swapchain_image_views;
	ham::std_vector<VkFramebuffer> vk_swapchain_framebuffers;

	VkPipelineLayout vk_pipeline_layout;
	VkRenderPass vk_render_pass;
	VkPipeline vk_pipeline;

	VkCommandPool vk_cmd_pool;

	VkCommandBuffer vk_cmd_bufs[ham_impl_max_queued_frames];
	VkSemaphore vk_img_sems[ham_impl_max_queued_frames];
	VkSemaphore vk_render_sems[ham_impl_max_queued_frames];
	VkFence vk_frame_fences[ham_impl_max_queued_frames];

	ham_vk_fns vk_fns;
	ham::vk::swapchain_info vk_swapchain_info;

	VmaVulkanFunctions vma_vk_fns;
	VmaAllocator vma_allocator;

	ham_u32 frame_counter = 0;
};

struct ham_draw_group_vulkan{
	ham_derive(ham_draw_group)

	VkBuffer vbo, ibo;
	VmaAllocation vbo_alloc, ibo_alloc;
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
