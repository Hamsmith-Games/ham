#
# Ham World Engine Server
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

##
# Server executable
##

add_executable(ham-engine-server server.h server.cpp)

set_target_properties(
	ham-engine-server PROPERTIES
	ENABLE_EXPORTS ON
	C_VISIBILITY_PRESET hidden
	CXX_VISIBILITY_PRESET hidden
	VISIBILITY_INLINES_HIDDEN ON
)

target_compile_features(ham-engine-server PRIVATE c_std_17 cxx_std_20)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
	target_link_options(ham-engine-server PRIVATE "-rdynamic")
endif()

target_link_libraries(ham-engine-server PRIVATE ham::engine)

##
# Manager/Console
##

find_package(NCURSES REQUIRED)

add_executable(ham-engine-server-manager manager.h manager.cpp)

target_compile_features(ham-engine-server-manager PRIVATE c_std_17 cxx_std_20)

target_compile_definitions(ham-engine-server-manager PRIVATE HAM_ENGINE_SERVER_MANAGER_IMPLEMENTATION)

target_link_libraries(ham-engine-server-manager PRIVATE ham::engine ncurses::ncurses)
