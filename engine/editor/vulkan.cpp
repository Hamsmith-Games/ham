/*
 * Ham World Engine Editor
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

#include "world_view.hpp"

namespace editor = ham::engine::editor;

//
// Renderer view vulkan
//

void editor::detail::vulkan_renderer::initResources(){
	if(m_r) return;

	/*
	VkInstance instance;
	VkPhysicalDevice physical_device;
	VkDevice device;
	PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr;
	PFN_vkGetDeviceProcAddr vkGetDeviceProcAddr;
	*/

	const auto qvk_inst = m_window->vulkanInstance();

	//qvk_inst->getInstanceProcAddr();
	//const auto qvk_dev_fns = qvk_inst->deviceFunctions(m_window->device());

	ham_renderer_create_args create_args;
	create_args.vulkan = {
		.instance = qvk_inst->vkInstance(),
		.physical_device = m_window->physicalDevice(),
		.device = m_window->device(),
	};

	m_r = ham_renderer_create(HAM_RENDERER_VULKAN_PLUGIN_NAME, HAM_RENDERER_VULKAN_OBJECT_NAME, &create_args);

	ham_ticker_reset(&m_ticker);
}

void editor::detail::vulkan_renderer::releaseResources(){
	ham_renderer_destroy(m_r);
	m_r = nullptr;
}

void editor::detail::vulkan_renderer::startNextFrame(){
	const f64 target_dt = 1.0/60.0;

	f64 dt = 0.0;
	do{
		dt += ham_ticker_tick(&m_ticker, target_dt - dt);
	} while(dt < target_dt);

	m_frame_data.vulkan.command_buffer = m_window->currentCommandBuffer();
	m_frame_data.vulkan.framebuffer = m_window->currentFramebuffer();

	ham_renderer_frame(m_r, dt, &m_frame_data);

	m_window->frameReady();

	++m_frame_data.current_frame;
}
