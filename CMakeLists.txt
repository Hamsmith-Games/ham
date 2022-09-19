#
# The Ham Programming Language
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
	DESCRIPTION "The Ham Programming Language"
	LANGUAGES C CXX
)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

##
# Configuration
##

include(GNUInstallDirs)
include(FetchContent)

set(HAM_VERSION_MAJOR ${ham_VERSION_MAJOR})
set(HAM_VERSION_MINOR ${ham_VERSION_MINOR})
set(HAM_VERSION_PATCH ${ham_VERSION_PATCH})
set(HAM_VERSION ${ham_VERSION})

set(HAM_SOURCE_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/include)
set(HAM_BINARY_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/include)

configure_file(
	${HAM_SOURCE_INCLUDE_DIR}/ham/config.h.in
	${HAM_BINARY_INCLUDE_DIR}/ham/config.h
	@ONLY
	NEWLINE_STYLE UNIX
)

set(
	HAM_HEADERS
	${HAM_BINARY_INCLUDE_DIR}/ham/config.h

	${HAM_SOURCE_INCLUDE_DIR}/ham.h
	${HAM_SOURCE_INCLUDE_DIR}/ham/typedefs.h
	${HAM_SOURCE_INCLUDE_DIR}/ham/math.h
	${HAM_SOURCE_INCLUDE_DIR}/ham/str_buffer.h
	${HAM_SOURCE_INCLUDE_DIR}/ham/hash.h
	${HAM_SOURCE_INCLUDE_DIR}/ham/thread.h
	${HAM_SOURCE_INCLUDE_DIR}/ham/fs.h
	${HAM_SOURCE_INCLUDE_DIR}/ham/plugin.h
	${HAM_SOURCE_INCLUDE_DIR}/ham/lex.h
	${HAM_SOURCE_INCLUDE_DIR}/ham/parse.h
	${HAM_SOURCE_INCLUDE_DIR}/ham/memory.h

	${HAM_SOURCE_INCLUDE_DIR}/ham/meta.hpp
	${HAM_SOURCE_INCLUDE_DIR}/ham/std_vector.hpp
)

set(
	HAM_INCLUDE_DIRS
	$<BUILD_INTERFACE:${HAM_SOURCE_INCLUDE_DIR}>
	$<BUILD_INTERFACE:${HAM_BINARY_INCLUDE_DIR}>
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

option(HAM_BUILD_DOCS   "Whether to build documentation"    ${PROJECT_IS_TOP_LEVEL})
option(HAM_BUILD_TESTS  "Whether to build library tests"    ${PROJECT_IS_TOP_LEVEL})
option(HAM_BUILD_TOOLS  "Whether to build tools"            ${PROJECT_IS_TOP_LEVEL})
option(HAM_BUILD_ENGINE "Whether to build the world engine" ${PROJECT_IS_TOP_LEVEL})

option(HAM_INSTALL_DOCS   "Whether to install generated documentation" ${PROJECT_IS_TOP_LEVEL})
option(HAM_INSTALL_TOOLS  "Whether to install compiled tools"          ${PROJECT_IS_TOP_LEVEL})
option(HAM_INSTALL_TESTS  "Whether to install the tests"               OFF)
option(HAM_INSTALL_ENGINE "Whether to install the world engine"        OFF)

##
# Dependencies
##

FetchContent_Declare(
	robin_hood_content
	GIT_REPOSITORY https://github.com/martinus/robin-hood-hashing.git
	GIT_TAG        fb1483621fda28d4afb31c0097c1a4a457fdd35b
)

FetchContent_MakeAvailable(robin_hood_content)

##
# Targets
##

add_subdirectory(lib)

if(HAM_BUILD_TOOLS)
	add_subdirectory(interpreter)
	add_subdirectory(comp)
endif()

if(HAM_BUILD_ENGINE)
	add_subdirectory(engine)
endif()

if(HAM_BUILD_TESTS)
	add_subdirectory(test)
endif()

if(HAM_BUILD_DOCS)
	add_subdirectory(docs)
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
	TARGETS ham
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

if(HAM_INSTALL_TOOLS)
	install(
		TARGETS hamc hami
		RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	)
endif()

if(HAM_INSTALL_TEST)
	install(
		TARGETS ham-test
		RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	)
endif()
