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

FetchContent_Declare(
	glbinding_content
	GIT_REPOSITORY https://github.com/cginternals/glbinding.git
	GIT_TAG        28d32d9bbc72aedf815f18113b0bd3aa7b354108
)

FetchContent_GetProperties(glbinding_content)
if(NOT glbinding_content_POPULATED)
	FetchContent_Populate(glbinding_content)

	set(BUILD_SHARED_LIBS_OLD ${BUILD_SHARED_LIBS})

	set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
	set(OPTION_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
	set(OPTION_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

	add_subdirectory(${glbinding_content_SOURCE_DIR} ${glbinding_content_BINARY_DIR} EXCLUDE_FROM_ALL)

	if(BUILD_SHARED_LIBS_OLD)
		set(BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)
	endif()
endif()

##
# Targets
##

set(
	HAM_RENDERER_GL_SHADERS
	${CMAKE_CURRENT_LIST_DIR}/screen.vert
	${CMAKE_CURRENT_LIST_DIR}/screen.frag
)

ham_add_plugin(
	ham-renderer-gl
	renderer-gl.hpp
	renderer-gl.cpp
)

target_link_libraries(ham-renderer-gl PRIVATE glbinding::glbinding glbinding::glbinding-aux)
