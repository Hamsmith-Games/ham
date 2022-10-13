#ifndef HAM_ENGINE_CLIENT_H
#define HAM_ENGINE_CLIENT_H 1

#include "SDL.h"
#include "SDL_vulkan.h"

#include "ham/engine.h"
#include "ham/log.h"
#include "ham/net.h"
#include "ham/renderer.h"

#ifdef HAM_ENGINE_CLIENT_IMPLEMENTATION
#	define ham_engine_client_api ham_private ham_export
#else
#	define ham_engine_client_api ham_private ham_import
#endif

#include "vk_mem_alloc.h"

HAM_C_API_BEGIN

ham_engine_client_api bool ham_engine_client_show_message(ham_log_level level, const char *msg);

ham_engine_client_api VkInstance ham_engine_client_init_vk_inst(SDL_Window *window, const ham_engine_app *app, ham_vk_fns *fns);

ham_engine_client_api VkPhysicalDevice ham_engine_client_init_vk_phys(
	VkInstance vk_inst, VkSurfaceKHR vk_surface, ham_vk_fns *fns,
	ham_u32 *graphics_family, ham_u32 *present_family
);

ham_engine_client_api VkDevice ham_engine_client_init_vk_device(
	VkPhysicalDevice vk_phys, ham_vk_fns *fns,
	ham_u32 graphics_family, ham_u32 present_family,
	VkQueue *graphics_queue_ret, VkQueue *present_queue_ret
);

ham_engine_client_api VmaAllocator ham_engine_client_init_vma(VkInstance vk_inst, VkPhysicalDevice vk_phys, VkDevice vk_dev);

ham_engine_client_api VkSwapchainKHR ham_engine_client_init_vk_swapchain(
	VkPhysicalDevice vk_phys, VkSurfaceKHR vk_surface, VkDevice vk_dev, const ham_vk_fns *fns,
	ham_u32 graphics_family, ham_u32 present_family,
	VkSurfaceCapabilitiesKHR *surface_caps_ret,
	VkSurfaceFormatKHR *surface_format_ret, VkPresentModeKHR *present_mode_ret,
	ham_u32 *image_count_ret
);

ham_engine_client_api bool ham_engine_client_init_vk_swapchain_images(
	VkDevice vk_dev, VmaAllocator vma, VkSwapchainKHR vk_swapchain, const ham_vk_fns *fns,
	const VkSurfaceCapabilitiesKHR *surface_caps, VkFormat surface_format,
	ham_u32 num_swapchain_images,
	VkImage *const images_ret,
	VkImageView *const image_views_ret,
	VkImage *const depth_ret,
	VkImageView *const depth_view_ret,
	VmaAllocation *const depth_alloc_ret
);

ham_engine_client_api bool ham_engine_client_init_vk_sync_objects(
	VkDevice vk_dev, const ham_vk_fns *fns,
	ham_u32 max_queued_frames,
	VkSemaphore *const frame_img_sems_ret,
	VkSemaphore *const frame_render_sems_ret,
	VkFence *const frame_fences_ret
);

ham_engine_client_api VkCommandPool ham_engine_client_init_vk_command_pool(VkDevice vk_dev, const ham_vk_fns *fns, ham_u32 graphics_family);

ham_engine_client_api bool ham_engine_client_init_vk_command_buffers(
	VkDevice vk_dev, const ham_vk_fns *fns,
	VkCommandPool vk_cmd_pool, ham_u32 max_queued_frames,
	VkCommandBuffer *const ret
);

HAM_C_API_END

#ifdef __cplusplus

#include "ham/buffer.h"

namespace ham::engine{
	using vulkan_present_fn = void(*)(VkCommandBuffer cmd_buf, void *user);

	class ham_engine_client_api client_window{
		public:
			virtual ~client_window();

			SDL_Window *window_handle() const noexcept{ return m_win; }

			virtual void present(f64 dt, ham_renderer *r) const = 0;

		protected:
			explicit client_window(const ham_engine_app *app, Uint32 flags = 0);

		private:
			SDL_Window *m_win;
	};

	class ham_engine_client_api client_window_gl: public client_window{
		public:
			client_window_gl(const ham_engine_app *app, Uint32 flags = 0);
			~client_window_gl();

			SDL_GLContext context_handle() const noexcept{ return m_ctx; }

			void present(f64 dt, ham_renderer *r) const override;

		private:
			SDL_GLContext m_ctx;
			mutable ham_renderer_frame_data m_frame_data;
	};

	class ham_engine_client_api client_window_vulkan: public client_window{
		public:
			explicit client_window_vulkan(const ham_engine_app *app, Uint32 flags = 0);
			~client_window_vulkan();

			VkInstance vk_instance() const noexcept{ return m_vk_inst; }
			VkSurfaceKHR vk_surface() const noexcept{ return m_vk_surface; }
			VkPhysicalDevice vk_physical_device() const noexcept{ return m_vk_phys; }
			VkDevice vk_device() const noexcept{ return m_vk_dev; }
			const ham_vk_fns *vk_fns() const noexcept{ return &m_vk_fns; }

			u32 num_swap_images() const noexcept{ return m_num_swapchain_imgs; }

			void present(f64 dt, ham_renderer *r) const override;

		private:
			ham_vk_fns m_vk_fns;

			VkInstance m_vk_inst;
			VkSurfaceKHR m_vk_surface;
			VkPhysicalDevice m_vk_phys;
			u32 m_vk_graphics_family, m_vk_present_family;
			VkDevice m_vk_dev;

			VmaAllocator m_vma_allocator;

			VkQueue m_vk_graphics_queue, m_vk_present_queue;

			VkSurfaceCapabilitiesKHR m_vk_surface_caps;
			VkSurfaceFormatKHR m_vk_surface_format;
			VkPresentModeKHR m_vk_present_mode;
			u32 m_num_swapchain_imgs;
			VkSwapchainKHR m_vk_swapchain;

			basic_buffer<VkImage> m_vk_swapchain_imgs;
			basic_buffer<VkImageView> m_vk_swapchain_img_views;
			VkImage m_vk_depth_img;
			VmaAllocation m_vk_depth_alloc;
			VkImageView m_vk_depth_view;

			VkCommandPool m_vk_cmd_pool;

			basic_buffer<VkSemaphore> m_vk_frame_img_sems;
			basic_buffer<VkSemaphore> m_vk_frame_render_sems;
			basic_buffer<VkFence> m_vk_frame_fences;
			basic_buffer<VkCommandBuffer> m_vk_frame_cmd_bufs;

			u32 m_max_queued_frames = 2;
			bool m_swapchain_dirty = false;

			mutable u64 m_cur_frame = 0;
			mutable ham_renderer_frame_data m_r_frame_data;
	};

	class ham_engine_client_api client_net_subsystem: public subsystem_base<client_net_subsystem>{
		public:
			explicit client_net_subsystem(ham_engine *engine): subsystem_base(engine){}

			static str8 name() noexcept{ return "client-net"; }

		private:
			bool init(ham_engine *engine);
			void fini(ham_engine *engine);
			void loop(ham_engine *engine, f64 dt);

			ham_net *m_net;
			ham_net_socket *m_serv_sock;

			friend class subsystem_base<client_net_subsystem>;
	};

	class ham_engine_client_api client_video_subsystem: public subsystem_base<client_video_subsystem>{
		public:
			explicit client_video_subsystem(ham_engine *engine): subsystem_base(engine){}

			static str8 name() noexcept{ return "video"; }

		private:
			bool init(ham_engine *engine);
			void fini(ham_engine *engine);
			void loop(ham_engine *engine, f64 dt);

			owned_ptr<client_window> m_win = nullptr;
			ham_renderer *m_r = nullptr;

			friend class subsystem_base<client_video_subsystem>;
	};
}

#endif // __cplusplus

#endif // !HAM_ENGINE_CLIENT_H
