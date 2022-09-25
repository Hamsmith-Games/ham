#
# Hamsmith Runtime Plugins
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
# Dependencies
##

find_package(Steamworks)

##
# Configuration
##

set(HAM_PLUGIN_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})

macro(ham_add_plugin TGT)
	ham_add_library(${TGT} SHARED ${ARGN})
	set(HAM_PLUGINS "${TGT};${HAM_PLUGINS}" PARENT_SCOPE)
	set_target_properties(
		${TGT} PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY ${HAM_PLUGIN_BINARY_DIR}
		LIBRARY_OUTPUT_DIRECTORY ${HAM_PLUGIN_BINARY_DIR}
	)
endmacro()

##
# Networking plugins
##

add_subdirectory(net-gns)

if(STEAMWORKS_FOUND)
	add_subdirectory(net-steam)
endif()

##
# Renderer plugins
##

add_subdirectory(renderer-vulkan)

##
# Installation
##

message(STATUS "Plugins: ${HAM_PLUGINS}")

set(HAM_PLUGINS "${HAM_PLUGINS}" PARENT_SCOPE)

if(NOT HAM_INSTALL_PLUGINS)
	return()
endif()

set(HAM_PLUGIN_INSTALL_DIR ${CMAKE_INSTALL_DATADIR}/Hamsmith/ham/plugins)

install(
	TARGETS ${HAM_PLUGINS}
	RUNTIME DESTINATION ${HAM_PLUGIN_INSTALL_DIR}
	LIBRARY DESTINATION ${HAM_PLUGIN_INSTALL_DIR}
)
