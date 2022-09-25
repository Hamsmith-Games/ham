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

find_package(Vulkan REQUIRED)

FetchContent_Declare(
	VulkanMemoryAllocator_content
	GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
	GIT_TAG        a6bfc237255a6bac1513f7c1ebde6d8aed6b5191
)

FetchContent_MakeAvailable(VulkanMemoryAllocator_content)

ham_add_plugin(ham-renderer-vulkan renderer.cpp)

target_link_libraries(ham-renderer-vulkan PRIVATE Vulkan::Vulkan VulkanMemoryAllocator)
