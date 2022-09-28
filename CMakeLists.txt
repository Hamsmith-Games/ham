#
# The Hamsmith Development Environment
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

##
# Project definitions
##

cmake_minimum_required(VERSION 3.21 FATAL_ERROR)

project(
	ham
	VERSION 0.0.1
	DESCRIPTION "The Hamsmith Programming Language"
	LANGUAGES C CXX
)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

##
# Configuration
##

include(GNUInstallDirs)
include(FetchContent)

include(StrHelpers)
include(CPUFeatures)

if(PROJECT_IS_TOP_LEVEL)
	set(CMAKE_SKIP_INSTALL_RPATH ON)
	set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

set(HAM_VERSION_MAJOR ${ham_VERSION_MAJOR})
set(HAM_VERSION_MINOR ${ham_VERSION_MINOR})
set(HAM_VERSION_PATCH ${ham_VERSION_PATCH})
set(HAM_VERSION ${ham_VERSION})

check_target_arch(HAM_TARGET_ARCH)

if(HAM_TARGET_ARCH STREQUAL "x86_64")
	set(HAM_CPU_LIBS cpu::x86-64-v2)
else()
	set(HAM_CPU_LIBS)
endif()

string_camel_to_snake("${CMAKE_BUILD_TYPE}" HAM_BUILD_TYPE)
string(TOUPPER ${HAM_BUILD_TYPE} HAM_BUILD_TYPE)

set(HAM_BUILD_TYPE_STR ${CMAKE_BUILD_TYPE})

set(HAM_SOURCE_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/include)
set(HAM_BINARY_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/include)

if(NOT HAM_TARGET_TRIPLE)
	execute_process(
		COMMAND
		    gcc -dumpmachine

		OUTPUT_STRIP_TRAILING_WHITESPACE
		OUTPUT_VARIABLE
		    HAM_TARGET_TRIPLE
	)
endif()

set(HAM_UUID_NAMESPACE "94d6f295-4e71-4b60-a7e5-a9fdc8bef7d2")

# Net plugin info

set(HAM_NET_GNS_PLUGIN ham-net-gns)
set(HAM_NET_STEAMWORKS_PLUGIN ham-net-steam)

string(
	UUID HAM_NET_UUID_NAMESPACE
	NAMESPACE ${HAM_UUID_NAMESPACE}
	NAME "net"
	TYPE SHA1
)

string(
	UUID HAM_NET_GNS_PLUGIN_UUID
	NAMESPACE ${HAM_NET_UUID_NAMESPACE}
	NAME ${HAM_NET_GNS_PLUGIN}
	TYPE SHA1
)

string(
	UUID HAM_NET_STEAMWORKS_PLUGIN_UUID
	NAMESPACE ${HAM_NET_UUID_NAMESPACE}
	NAME ${HAM_NET_STEAMWORKS_PLUGIN}
	TYPE SHA1
)

set(HAM_NET_DEFAULT_PLUGIN ${HAM_NET_GNS_PLUGIN})
set(HAM_NET_DEFAULT_PLUGIN_UUID ${HAM_NET_GNS_PLUGIN_UUID})

# Render plugin info

set(HAM_RENDERER_VULKAN_PLUGIN ham-renderer-vulkan)

string(
	UUID HAM_RENDERER_UUID_NAMESPACE
	NAMESPACE ${HAM_UUID_NAMESPACE}
	NAME "Renderer"
	TYPE SHA1
)

string(
	UUID HAM_RENDERER_VULKAN_PLUGIN_UUID
	NAMESPACE ${HAM_RENDERER_UUID_NAMESPACE}
	NAME ${HAM_RENDERER_VULKAN_PLUGIN}
	TYPE SHA1
)

set(HAM_RENDERER_DEFAULT_PLUGIN ${HAM_RENDERER_VULKAN_PLUGIN})
set(HAM_RENDERER_DEFAULT_PLUGIN_UUID ${HAM_RENDERER_VULKAN_PLUGIN_UUID})

# Keep a list of all headers

set(
	HAM_HEADERS
	${HAM_BINARY_INCLUDE_DIR}/ham/config.h

	${HAM_SOURCE_INCLUDE_DIR}/ham.h

	${HAM_SOURCE_INCLUDE_DIR}/ham/pp.h
	${HAM_SOURCE_INCLUDE_DIR}/ham/typedefs.h
	${HAM_SOURCE_INCLUDE_DIR}/ham/typesys.h
	${HAM_SOURCE_INCLUDE_DIR}/ham/log.h
	${HAM_SOURCE_INCLUDE_DIR}/ham/check.h
	${HAM_SOURCE_INCLUDE_DIR}/ham/memory.h
	${HAM_SOURCE_INCLUDE_DIR}/ham/time.h
	${HAM_SOURCE_INCLUDE_DIR}/ham/async.h
	${HAM_SOURCE_INCLUDE_DIR}/ham/functional.h
	${HAM_SOURCE_INCLUDE_DIR}/ham/object.h
	${HAM_SOURCE_INCLUDE_DIR}/ham/math.h
	${HAM_SOURCE_INCLUDE_DIR}/ham/hash.h
	${HAM_SOURCE_INCLUDE_DIR}/ham/fs.h
	${HAM_SOURCE_INCLUDE_DIR}/ham/dso.h
	${HAM_SOURCE_INCLUDE_DIR}/ham/plugin.h
	${HAM_SOURCE_INCLUDE_DIR}/ham/colony.h
	${HAM_SOURCE_INCLUDE_DIR}/ham/buffer.h
	${HAM_SOURCE_INCLUDE_DIR}/ham/str_buffer.h
	${HAM_SOURCE_INCLUDE_DIR}/ham/json.h

	${HAM_SOURCE_INCLUDE_DIR}/ham/lex.h
	${HAM_SOURCE_INCLUDE_DIR}/ham/parse.h

	${HAM_SOURCE_INCLUDE_DIR}/ham/shape.h

	${HAM_SOURCE_INCLUDE_DIR}/ham/net.h
	${HAM_SOURCE_INCLUDE_DIR}/ham/net-object.h
	${HAM_SOURCE_INCLUDE_DIR}/ham/renderer.h
	${HAM_SOURCE_INCLUDE_DIR}/ham/renderer-object.h

	# C++ only headers
	${HAM_SOURCE_INCLUDE_DIR}/ham/meta.hpp
	${HAM_SOURCE_INCLUDE_DIR}/ham/std_vector.hpp
)

set(
	HAM_INCLUDE_DIRS
	$<BUILD_INTERFACE:${HAM_SOURCE_INCLUDE_DIR}>
	$<BUILD_INTERFACE:${HAM_BINARY_INCLUDE_DIR}>
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# User options

option(HAM_BUILD_DOCS    "Whether to build documentation"       ${PROJECT_IS_TOP_LEVEL})
option(HAM_BUILD_PLUGINS "Whether to build the default plugins" ON)
option(HAM_BUILD_TESTS   "Whether to build library tests"       ${PROJECT_IS_TOP_LEVEL})
option(HAM_BUILD_TOOLS   "Whether to build tools"               ${PROJECT_IS_TOP_LEVEL})
option(HAM_BUILD_ENGINE  "Whether to build the world engine"    ${PROJECT_IS_TOP_LEVEL})

option(HAM_INSTALL_DOCS    "Whether to install generated documentation" ${PROJECT_IS_TOP_LEVEL})
option(HAM_INSTALL_PLUGINS "Whether to install the default plugins"     ON)
option(HAM_INSTALL_TOOLS   "Whether to install compiled tools"          ${PROJECT_IS_TOP_LEVEL})
option(HAM_INSTALL_TESTS   "Whether to install the tests"               OFF)
option(HAM_INSTALL_ENGINE  "Whether to install the world engine"        OFF)

set(HAM_ENGINE_MAX_SUBSYSTEMS "16" CACHE STRING "Maximum number of subsystems allowed in an engine before an error")
set(HAM_BUILD_STEAM_APPID "480" CACHE STRING "Steam appid to put in steam_appid.txt for build directories")

set(HAM_COLONY_MAX_BUCKETS 16 CACHE STRING "Maximum number of storage buckets for each colony before an out of memory error")
set(HAM_OBJECT_INSTANCE_MANAGER_MAX_BLOCKS 16 CACHE STRING "Maximum number of storage blocks for each instance manager before an out of memory error")

# Create configuration file

configure_file(
	${HAM_SOURCE_INCLUDE_DIR}/ham/config.h.in
	${HAM_BINARY_INCLUDE_DIR}/ham/config.h
	@ONLY
	NEWLINE_STYLE UNIX
)

##
# Dependencies
##

FetchContent_Declare(
	robin_hood_content
	GIT_REPOSITORY https://github.com/martinus/robin-hood-hashing.git
	GIT_TAG        fb1483621fda28d4afb31c0097c1a4a457fdd35b
)

FetchContent_Declare(
	yyjson_content
	GIT_REPOSITORY https://github.com/ibireme/yyjson.git
	GIT_TAG        c3856514de0a67d7b66939bf3ed491a2d6e61277 # 0.5.1
)

FetchContent_MakeAvailable(yyjson_content)

FetchContent_GetProperties(robin_hood_content)
if(NOT robin_hood_content_POPULATED)
	FetchContent_Populate(robin_hood_content)

	add_subdirectory(${robin_hood_content_SOURCE_DIR} ${robin_hood_content_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

##
# Third-party/Licensed dependencies
##

find_file(
	STEAMWORKS_SDK_PATH
	NAMES steamworks_sdk_155.zip
	PATHS ${CMAKE_CURRENT_LIST_DIR}/third_party
)

##
# Runtime library, required for all other components
##

add_subdirectory(runtime)

macro(ham_add_executable TGT)
	add_executable(${TGT} ${ARGN})

	target_compile_features(${TGT} PRIVATE c_std_17 cxx_std_20)
	target_link_libraries(${TGT} PUBLIC ham::ham)
	target_link_libraries(${TGT} PRIVATE ${HAM_CPU_FLAGS})

	set_target_properties(
		${TGT} PROPERTIES
		C_STANDARD 17
		C_EXTENSIONS ON
		C_VISIBILITY_PRESET hidden
		CXX_STANDARD 20
		CXX_EXTENSIONS ON
		CXX_VISIBILITY_PRESET hidden
		VISIBILITY_INLINES_HIDDEN ON
	)

    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
		target_link_options(${TGT} PRIVATE "-Wl,--enable-new-dtags")
	endif()
endmacro()

macro(ham_add_library TGT)
	add_library(${TGT} ${ARGN})
	target_compile_features(${TGT} PRIVATE c_std_17 cxx_std_20)
	target_link_libraries(${TGT} PRIVATE ham::ham ${HAM_CPU_FLAGS})
	set_target_properties(
		${TGT} PROPERTIES
		C_STANDARD 17
		C_EXTENSIONS ON
		C_VISIBILITY_PRESET hidden
		CXX_STANDARD 20
		CXX_EXTENSIONS ON
		CXX_VISIBILITY_PRESET hidden
		VISIBILITY_INLINES_HIDDEN ON
	)
endmacro()

##
# Targets
##

if(HAM_BUILD_PLUGINS)
	add_subdirectory(plugins)
endif()

if(HAM_BUILD_TOOLS)
	add_subdirectory(interpreter)
	add_subdirectory(comp)
endif()

if(HAM_BUILD_ENGINE)
	add_subdirectory(engine)
	if(HAM_BUILD_PLUGINS)
		add_dependencies(ham-engine ${HAM_PLUGINS})
	endif()
endif()

if(HAM_BUILD_TESTS)
	add_subdirectory(test EXCLUDE_FROM_ALL)
endif()

if(HAM_BUILD_DOCS)
	add_subdirectory(docs EXCLUDE_FROM_ALL)
endif()

##
# Installation
##

install(
	FILES ${CMAKE_CURRENT_LIST_DIR}/include/ham.h
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
	DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/include/ham
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
	FILES_MATCHING
	    PATTERN "*.h"
		PATTERN "*.hpp"
)

install(
	FILES runtime/LICENSE.md
	DESTINATION ${CMAKE_INSTALL_DATADIR}/licenses/ham/libham
)

install(
	TARGETS ham
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

if(HAM_INSTALL_TOOLS)
	install(
		FILES interpreter/LICENSE.md
		DESTINATION ${CMAKE_INSTALL_DATADIR}/licenses/ham/hami
	)

	install(
		FILES compiler/LICENSE.md
		DESTINATION ${CMAKE_INSTALL_DATADIR}/licenses/ham/hamc
	)

	install(
		TARGETS hamc hami
		RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	)
endif()

if(HAM_INSTALL_TEST)
	install(
		FILES test/LICENSE.md
		DESTINATION ${CMAKE_INSTALL_DATADIR}/licenses/ham/ham-test
	)

	install(
		TARGETS ham-test
		RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	)
endif()
