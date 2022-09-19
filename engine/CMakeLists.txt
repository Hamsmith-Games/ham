#
# The Ham World Engine
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
# The Ham World Engine
##

set(HAM_ENGINE_SOURCE_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/include)

set(
	HAM_ENGINE_SOURCE_HEADERS
	${HAM_ENGINE_SOURCE_INCLUDE_DIR}/ham/engine.h
)

add_subdirectory(common)
add_subdirectory(server)
add_subdirectory(client)
add_subdirectory(editor)

if(HAM_INSTALL_ENGINE)
	install(
		TARGETS ham-engine-common ham-engine-client ham-engine-client
		RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
		LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	)

    install(
		DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/include/ham
		DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
		FILES_MATCHING
		    PATTERN "*.h"
			PATTERN "*.hpp"
	)
endif()
