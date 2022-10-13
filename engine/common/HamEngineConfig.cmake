#
# Ham World Engine Runtime
# Copyright (C) 2022 Hamsmith Ltd.
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

set(HamEngine_PREFIX ${CMAKE_CURRENT_LIST_DIR}/../../../)

set(HamEngine_INCLUDE_DIRS ${HamEngine_PREFIX}/include)
set(HamEngine_LIBRARY_DIRS ${HamEngine_PREFIX}/lib)
set(HamEngine_LIBRARIES ham ham-engine)

if(NOT TARGET ham-engine)
	add_library(ham-engine INTERFACE)
	add_library(ham::engine ALIAS ham-engine)

	target_compile_features(ham-engine INTERFACE c_std_17 cxx_std_20)

	target_link_libraries(ham-engine INTERFACE ${HamEngine_LIBRARIES})
	target_link_directories(ham-engine INTERFACE ${HamEngine_LIBRARY_DIRS})
	target_include_directories(ham-engine INTERFACE ${HamEngine_INCLUDE_DIRS})
endif()
