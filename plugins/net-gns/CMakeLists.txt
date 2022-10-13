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

FetchContent_Declare(
	gns_content
	GIT_REPOSITORY https://github.com/ValveSoftware/GameNetworkingSockets.git
	GIT_TAG        e0d33386903202d9af61e76d69c54e46ece2f457
)

FetchContent_GetProperties(gns_content)
if(NOT gns_content_POPULATED)
	FetchContent_Populate(gns_content)

	set(USE_STEAMWEBRTC ON CACHE BOOL "" FORCE)
	add_subdirectory(${gns_content_SOURCE_DIR} ${gns_content_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

ham_add_plugin(
	ham-net-gns
	net-gns.h
	net-gns.cpp
	net-socket-gns.cpp
	net-connection-gns.cpp
)

target_link_libraries(ham-net-gns PRIVATE GameNetworkingSockets_s)
