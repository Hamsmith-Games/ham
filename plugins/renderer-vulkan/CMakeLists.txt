#
# Hamsmith Runtime Plugins
# Copyright (C) 2022  Hamsmith Ltd.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
#

find_package(Vulkan REQUIRED COMPONENTS glslc)

set(VMA_STATIC_VULKAN_FUNCTIONS OFF CACHE BOOL "" FORCE)
set(VMA_DYNAMIC_VULKAN_FUNCTIONS ON CACHE BOOL "" FORCE)

FetchContent_Declare(
	VulkanMemoryAllocator_content
	GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
	GIT_TAG        a6bfc237255a6bac1513f7c1ebde6d8aed6b5191
)

FetchContent_MakeAvailable(VulkanMemoryAllocator_content)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
	target_compile_options(VulkanMemoryAllocator PRIVATE "-fPIC")
endif()

ham_compile_shader_vk(${CMAKE_CURRENT_LIST_DIR}/screen.vert ${HAM_PLUGIN_BINARY_DIR}/shaders/screen.vert.spv)
ham_compile_shader_vk(${CMAKE_CURRENT_LIST_DIR}/screen.frag ${HAM_PLUGIN_BINARY_DIR}/shaders/screen.frag.spv)

set(
	HAM_RENDERER_VULKAN_SHADERS
	${HAM_PLUGIN_BINARY_DIR}/shaders/screen.vert.spv
	${HAM_PLUGIN_BINARY_DIR}/shaders/screen.frag.spv
)

ham_add_plugin(
	ham-renderer-vulkan
	${HAM_RENDERER_VULKAN_SHADERS}
	renderer.hpp
	renderer.cpp
	shader_vulkan.cpp
	draw_group.cpp
	light_group_vulkan.cpp
)

target_link_libraries(ham-renderer-vulkan PRIVATE VulkanMemoryAllocator)
