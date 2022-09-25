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

set(HAM_PLUGINS "")

##
# Networking plugins
##

add_subdirectory(net-gns)
list(APPEND HAM_PLUGINS ham-net-gns)

if(STEAMWORKS_FOUND)
	add_subdirectory(net-steam)
	list(APPEND HAM_PLUGINS ham-net-steam)
endif()

set(HAM_PLUGINS "${HAM_PLUGINS}" PARENT_SCOPE)

##
# Installation
##

if(NOT HAM_INSTALL_PLUGINS)
	return()
endif()

set(HAM_PLUGIN_INSTALL_DIR ${CMAKE_INSTALL_DATADIR}/Hamsmith/ham/plugins)

install(
	TARGETS ${HAM_PLUGINS}
	RUNTIME DESTINATION ${HAM_PLUGIN_INSTALL_DIR}
	LIBRARY DESTINATION ${HAM_PLUGIN_INSTALL_DIR}
)
