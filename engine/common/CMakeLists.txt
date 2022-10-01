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

ham_add_library(
	ham-engine SHARED
	${HAM_ENGINE_HEADERS}
	engine.cpp
	subsys.cpp
	sys.cpp
	world.cpp
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
)
