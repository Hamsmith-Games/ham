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

ham_engine_client_api VkInstance ham_engine_client_init_vk_inst(SDL_Window *window, const ham_engine_app *app, PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr);

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
			PFN_vkGetInstanceProcAddr pfn_vkGetInstanceProcAddr() const noexcept{ return m_vkGetInstanceProcAddr; }

			void present(f64 dt, ham_renderer *r) const override;

		private:
			VkInstance m_vk_inst;
			VkSurfaceKHR m_vk_surface;
			PFN_vkGetInstanceProcAddr m_vkGetInstanceProcAddr;

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
