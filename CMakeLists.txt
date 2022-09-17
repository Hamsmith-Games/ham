##
# Project definitions
##

cmake_minimum_required(VERSION 3.21 FATAL_ERROR)

project(
	ham-lang
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

set(HAM_SOURCE_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/include)

set(
	HAM_HEADERS
	${HAM_SOURCE_INCLUDE_DIR}/ham.h
	${HAM_SOURCE_INCLUDE_DIR}/ham/config.h
	${HAM_SOURCE_INCLUDE_DIR}/ham/typedefs.h
	${HAM_SOURCE_INCLUDE_DIR}/ham/math.h
	${HAM_SOURCE_INCLUDE_DIR}/ham/str_buffer.h
	${HAM_SOURCE_INCLUDE_DIR}/ham/hash.h
	${HAM_SOURCE_INCLUDE_DIR}/ham/lex.h
	${HAM_SOURCE_INCLUDE_DIR}/ham/parse.h
	${HAM_SOURCE_INCLUDE_DIR}/ham/memory.h

	${HAM_SOURCE_INCLUDE_DIR}/ham/meta.hpp
	${HAM_SOURCE_INCLUDE_DIR}/ham/std_vector.hpp
)

option(HAM_BUILD_TESTS "Whether to build library tests" ${PROJECT_IS_TOP_LEVEL})
option(HAM_BUILD_TOOLS "Whether to build tools" ${PROJECT_IS_TOP_LEVEL})

option(HAM_INSTALL_TOOLS "Whether to install compiled tools" ${PROJECT_IS_TOP_LEVEL})

##
# Dependencies
##

# none yet...

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
	add_subdirectory(comp)
endif()

if(HAM_BUILD_TESTS)
	add_subdirectory(test)
endif()

##
# Installation
##

install(
	FILES ${CMAKE_CURRENT_LIST_DIR}/include/gpwe.h
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
	DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/include/gpwe
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
	TARGETS ham
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

if(HAM_INSTALL_TOOLS)
	install(
		TARGETS hamc
		RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	)
endif()
