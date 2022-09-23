#
# Ham World Engine Runtime
# Copyright (C) 2022  Hamsmith Ltd.
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

add_library(
	ham-engine SHARED
	${HAM_ENGINE_SOURCE_HEADERS}
	engine.cpp
	sys.cpp
)

add_library(ham::engine ALIAS ham-engine)

set_target_properties(
	ham-engine PROPERTIES
	C_VISIBILITY_PRESET hidden
	CXX_VISIBILITY_PRESET hidden
	VISIBILITY_INLINES_HIDDEN ON
)

target_compile_features(ham-engine PUBLIC c_std_17 cxx_std_20)

target_compile_definitions(ham-engine PRIVATE HAM_ENGINE_LIB_IMPLEMENTATION)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
	target_link_options(ham-engine PRIVATE "-Wl,--enable-new-dtags")
endif()

target_link_libraries(ham-engine PRIVATE ${HAM_CPU_LIBS})
target_link_libraries(ham-engine PUBLIC ham::ham)

target_include_directories(
	ham-engine PUBLIC
	$<BUILD_INTERFACE:${HAM_ENGINE_SOURCE_INCLUDE_DIR}>
	$<BUILD_INTERFACE:${HAM_ENGINE_BINARY_INCLUDE_DIR}>
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
