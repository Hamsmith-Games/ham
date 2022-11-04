#
# Hamsmith Runtime Plugins
# Copyright (C) 2022 Hamsmith Ltd.
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

##
# Dependencies
##

find_package(Vulkan REQUIRED COMPONENTS glslc)

add_library(glad OBJECT glad/src/glad.c)
add_library(glad::glad ALIAS glad)

target_include_directories(glad PUBLIC glad/include)

#FetchContent_Declare(
#	glbinding_content
#	GIT_REPOSITORY https://github.com/cginternals/glbinding.git
#	GIT_TAG        28d32d9bbc72aedf815f18113b0bd3aa7b354108
#)

#FetchContent_GetProperties(glbinding_content)
#if(NOT glbinding_content_POPULATED)
#	FetchContent_Populate(glbinding_content)

#	set(BUILD_SHARED_LIBS_OLD ${BUILD_SHARED_LIBS})

#	set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
#	set(OPTION_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
#	set(OPTION_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

#	add_subdirectory(${glbinding_content_SOURCE_DIR} ${glbinding_content_BINARY_DIR} EXCLUDE_FROM_ALL)

#	if(BUILD_SHARED_LIBS_OLD)
#		set(BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)
#	endif()
#endif()

##
# Targets
##

ham_compile_shader_gl(${CMAKE_CURRENT_LIST_DIR}/light.vert ${HAM_PLUGIN_BINARY_DIR}/shaders-gl/light.vert.spv)
ham_compile_shader_gl(${CMAKE_CURRENT_LIST_DIR}/light.frag ${HAM_PLUGIN_BINARY_DIR}/shaders-gl/light.frag.spv)
ham_compile_shader_gl(${CMAKE_CURRENT_LIST_DIR}/screen.vert ${HAM_PLUGIN_BINARY_DIR}/shaders-gl/screen.vert.spv)
ham_compile_shader_gl(${CMAKE_CURRENT_LIST_DIR}/screen.frag ${HAM_PLUGIN_BINARY_DIR}/shaders-gl/screen.frag.spv)
ham_compile_shader_gl(${CMAKE_CURRENT_LIST_DIR}/scene_info.vert ${HAM_PLUGIN_BINARY_DIR}/shaders-gl/scene_info.vert.spv)
ham_compile_shader_gl(${CMAKE_CURRENT_LIST_DIR}/scene_info.frag ${HAM_PLUGIN_BINARY_DIR}/shaders-gl/scene_info.frag.spv)

function(ham_ensure_file INPUT OUTPUT)
	add_custom_command(
		OUTPUT ${OUTPUT}
		COMMAND ${CMAKE_COMMAND} -E copy ${INPUT} ${OUTPUT}
		MAIN_DEPENDENCY ${INPUT}
	)
endfunction()

ham_ensure_file(${CMAKE_CURRENT_LIST_DIR}/light.vert ${HAM_PLUGIN_BINARY_DIR}/shaders-gl/light.vert)
ham_ensure_file(${CMAKE_CURRENT_LIST_DIR}/light.frag ${HAM_PLUGIN_BINARY_DIR}/shaders-gl/light.frag)
ham_ensure_file(${CMAKE_CURRENT_LIST_DIR}/screen.vert ${HAM_PLUGIN_BINARY_DIR}/shaders-gl/screen.vert)
ham_ensure_file(${CMAKE_CURRENT_LIST_DIR}/screen.frag ${HAM_PLUGIN_BINARY_DIR}/shaders-gl/screen.frag)
ham_ensure_file(${CMAKE_CURRENT_LIST_DIR}/scene_info.vert ${HAM_PLUGIN_BINARY_DIR}/shaders-gl/scene_info.vert)
ham_ensure_file(${CMAKE_CURRENT_LIST_DIR}/scene_info.frag ${HAM_PLUGIN_BINARY_DIR}/shaders-gl/scene_info.frag)

set(
	HAM_RENDERER_GL_SHADERS

	${HAM_PLUGIN_BINARY_DIR}/shaders-gl/bone_skin.vert.spv
	${HAM_PLUGIN_BINARY_DIR}/shaders-gl/light.vert.spv
	${HAM_PLUGIN_BINARY_DIR}/shaders-gl/light.frag.spv
	${HAM_PLUGIN_BINARY_DIR}/shaders-gl/screen.vert.spv
	${HAM_PLUGIN_BINARY_DIR}/shaders-gl/screen.frag.spv
	${HAM_PLUGIN_BINARY_DIR}/shaders-gl/scene_info.vert.spv
	${HAM_PLUGIN_BINARY_DIR}/shaders-gl/scene_info.frag.spv

	${HAM_PLUGIN_BINARY_DIR}/shaders-gl/light.vert
	${HAM_PLUGIN_BINARY_DIR}/shaders-gl/light.frag
	${HAM_PLUGIN_BINARY_DIR}/shaders-gl/screen.vert
	${HAM_PLUGIN_BINARY_DIR}/shaders-gl/screen.frag
	${HAM_PLUGIN_BINARY_DIR}/shaders-gl/scene_info.vert
	${HAM_PLUGIN_BINARY_DIR}/shaders-gl/scene_info.frag
)

ham_add_plugin(
	ham-renderer-gl
	${HAM_RENDERER_GL_SHADERS}
	renderer-gl.hpp
	renderer-gl.cpp
	draw-group-gl.cpp
	light-group-gl.cpp
)

target_compile_definitions(ham-renderer-gl PRIVATE HAM_RENDERER_GL_IMPLEMENTATION)

target_link_libraries(ham-renderer-gl PRIVATE robin_hood::robin_hood glad::glad)
