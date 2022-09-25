#
# The Ham Runtime
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

set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(GMP REQUIRED)
find_package(MPFR REQUIRED)
find_package(Magic REQUIRED)
find_package(Threads REQUIRED)

set(
	HAM_SOURCES
	${HAM_HEADERS}
	object.cpp
	memory.cpp
	log.cpp
	str_buffer.cpp
	thread.cpp
	fs.cpp
	plugin.cpp
	lex.cpp
	parse.cpp
	net.cpp
)

if(NOT CMAKE_USE_PTHREADS_INIT)
	message(FATLA_ERROR "Async currently only available with pthread support")
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

add_library(ham SHARED ${HAM_SOURCES})

set_target_properties(
	ham PROPERTIES

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
		magic::magic
)

target_link_libraries(
	ham
	PRIVATE
	    ${CMAKE_DL_LIBS}
	    ${HAM_CPU_LIBS}
		bfd
		robin_hood::robin_hood
)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
	target_link_options(ham PRIVATE "-Wl,--enable-new-dtags")
endif()

add_library(ham::ham ALIAS ham)
