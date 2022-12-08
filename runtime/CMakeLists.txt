#
# The Ham Runtime
# Copyright (C) 2022 Keith Hammond
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

include(CMakePackageConfigHelpers)
include(FetchContent)

set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(Threads REQUIRED)
find_package(Magic REQUIRED)
find_package(GMP REQUIRED)
find_package(MPFR REQUIRED)
find_package(FreeImage REQUIRED)

FetchContent_Declare(
	libsndfile_content
	GIT_REPOSITORY https://github.com/libsndfile/libsndfile.git
	GIT_TAG        ea3ac90e98c6a98cd52cae39010446fba368a2e3
)

FetchContent_GetProperties(libsndfile_content)
if(NOT libsndfile_content_POPULATED)
	FetchContent_Populate(libsndfile_content)
	set(ENABLE_SSE2 ON CACHE BOOL "" FORCE)
	add_subdirectory(${libsndfile_content_SOURCE_DIR} ${libsndfile_content_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

set(
	HAM_SOURCES
	${HAM_HEADERS}
	typesys.cpp
	object.cpp
	memory.cpp
	log.cpp
	fs.cpp
	plugin.cpp
	colony.cpp
	octree.cpp
	str_buffer.cpp
	lex.cpp
	parse.cpp
	json.cpp
	shape.cpp
	image.cpp
	net.cpp
	audio.cpp
	renderer.cpp
	ham.cpp
)

if(NOT CMAKE_USE_PTHREADS_INIT)
	message(FATAL_ERROR "Async currently only available with POSIX thread API support")
endif()

if(WIN32)
	list(
		APPEND HAM_SOURCES
		dso-win32.cpp
		async-pthread.cpp
	)
elseif(UNIX)
	list(
		APPEND HAM_SOURCES
		dso-unix.cpp
		async-pthread.cpp
	)
else()
	message(FATAL_ERROR "Unsupported platform")
endif()

configure_package_config_file(
	HamConfig.cmake.in
	${PROJECT_BINARY_DIR}/HamConfig.cmake
	INSTALL_DESTINATION ${CMAKE_INSTALL_DATADIR}/ham/cmake
	PATH_VARS CMAKE_INSTALL_INCLUDEDIR CMAKE_INSTALL_LIBDIR
)

write_basic_package_version_file(
	${PROJECT_BINARY_DIR}/HamConfigVersion.cmake
	VERSION ${PROJECT_VERSION}
	COMPATIBILITY ExactVersion
)

add_library(ham SHARED ${HAM_SOURCES})
add_library(ham::runtime ALIAS ham)

set_target_properties(
	ham PROPERTIES

	C_EXTENSIONS ON
	CXX_EXTENSIONS ON

	C_VISIBILITY_PRESET hidden
	CXX_VISIBILITY_PRESET hidden
	VISIBILITY_INLINES_HIDDEN ON
)

target_compile_definitions(ham PRIVATE HAM_LIB_IMPLEMENTATION)

target_compile_features(ham PUBLIC c_std_11 cxx_std_20)

target_include_directories(
	ham PUBLIC
	$<BUILD_INTERFACE:${HAM_SOURCE_INCLUDE_DIR}>
	$<BUILD_INTERFACE:${HAM_BINARY_INCLUDE_DIR}>
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(
	ham
	PUBLIC
		Threads::Threads
	    gmp::gmp
		mpfr::mpfr
		fmt::fmt
)

target_link_libraries(
	ham
	PRIVATE
	    ${CMAKE_DL_LIBS}
	    ${HAM_CPU_LIBS}
		bfd
		magic::magic
		FreeImage::FreeImage
		SndFile::sndfile
		robin_hood::robin_hood
		yyjson
)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
	target_compile_options(ham PRIVATE "-Wall" "-Werror")
	target_link_options(ham PRIVATE "-Wl,--enable-new-dtags")
elseif(MSVC)
	target_compile_options(ham PRIVATE "/W4" "/nologo")
endif()
