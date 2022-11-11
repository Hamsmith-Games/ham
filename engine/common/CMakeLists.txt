#
# Ham World Engine Runtime
# Copyright (C) 2022 Hamsmith Ltd.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
#

include(CMakePackageConfigHelpers)

FetchContent_Declare(
	assimp_content
	GIT_REPOSITORY https://github.com/assimp/assimp.git
	GIT_TAG        c2967cf79acdc4cd48ecb0729e2733bf45b38a6f
)

FetchContent_GetProperties(assimp_content)
if(NOT assimp_content_POPULATED)
	FetchContent_Populate(assimp_content)

	set(ASSIMP_BUILD_TESTS FALSE CACHE BOOL "" FORCE)
	set(ASSIMP_INSTALL FALSE CACHE BOOL "" FORCE)
	set(ASSIMP_INJECT_DEBUG_POSTFIX FALSE CACHE BOOL "" FORCE)
	set(ASSIMP_BUILD_ZLIB ON CACHE BOOL "" FORCE)

	add_subdirectory(${assimp_content_SOURCE_DIR} ${assimp_content_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

set(HAM_ENGINE_SHADER_DIR ${CMAKE_CURRENT_BINARY_DIR})

ham_compile_shader(glsl/scene_info.vert ${HAM_ENGINE_SHADER_DIR})
ham_compile_shader(glsl/scene_info.frag ${HAM_ENGINE_SHADER_DIR})
ham_compile_shader(glsl/light.vert ${HAM_ENGINE_SHADER_DIR})
ham_compile_shader(glsl/light.frag ${HAM_ENGINE_SHADER_DIR})
ham_compile_shader(glsl/screen.vert ${HAM_ENGINE_SHADER_DIR})
ham_compile_shader(glsl/screen.frag ${HAM_ENGINE_SHADER_DIR})

set(
	HAM_ENGINE_SHADERS
	${HAM_ENGINE_SHADER_DIR}/scene_info.vert.gl.spv
	${HAM_ENGINE_SHADER_DIR}/scene_info.vert.vk.spv
	${HAM_ENGINE_SHADER_DIR}/scene_info.frag.gl.spv
	${HAM_ENGINE_SHADER_DIR}/scene_info.frag.vk.spv
	${HAM_ENGINE_SHADER_DIR}/light.vert.gl.spv
	${HAM_ENGINE_SHADER_DIR}/light.vert.vk.spv
	${HAM_ENGINE_SHADER_DIR}/light.frag.gl.spv
	${HAM_ENGINE_SHADER_DIR}/light.frag.vk.spv
	${HAM_ENGINE_SHADER_DIR}/screen.vert.gl.spv
	${HAM_ENGINE_SHADER_DIR}/screen.vert.vk.spv
	${HAM_ENGINE_SHADER_DIR}/screen.frag.gl.spv
	${HAM_ENGINE_SHADER_DIR}/screen.frag.vk.spv
)

ham_add_library(
	ham-engine SHARED
	${HAM_ENGINE_HEADERS}
	${HAM_ENGINE_SHADERS}
	engine.cpp
	argparse.cpp
	sys.cpp
	model.cpp
	entity_component.cpp
	world.cpp
	graph.cpp
)

add_library(ham::engine ALIAS ham-engine)

target_compile_definitions(ham-engine PRIVATE HAM_ENGINE_LIB_IMPLEMENTATION)

target_include_directories(
	ham-engine PUBLIC
	$<BUILD_INTERFACE:${HAM_ENGINE_SOURCE_INCLUDE_DIR}>
	$<BUILD_INTERFACE:${HAM_ENGINE_BINARY_INCLUDE_DIR}>
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(
	ham-engine PRIVATE
	robin_hood::robin_hood
	assimp::assimp
)

configure_package_config_file(
	HamEngineConfig.cmake.in
	${PROJECT_BINARY_DIR}/HamEngineConfig.cmake
	INSTALL_DESTINATION ${CMAKE_INSTALL_DATADIR}/ham/cmake
	PATH_VARS CMAKE_INSTALL_INCLUDEDIR CMAKE_INSTALL_LIBDIR
)

write_basic_package_version_file(
	${PROJECT_BINARY_DIR}/HamEngineConfigVersion.cmake
	VERSION ${PROJECT_VERSION}
	COMPATIBILITY ExactVersion
)
