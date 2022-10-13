#
# Ham Runtime
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

set(HAM_PREFIX ${CMAKE_CURRENT_LIST_DIR}/../../../)

set(Ham_INCLUDE_DIRS ${HAM_PREFIX}/include)
set(Ham_LIBRARY_DIRS ${HAM_PREFIX}/lib)
set(Ham_LIBRARIES ham)

if(NOT TARGET ham-runtime)
	add_library(ham-runtime INTERFACE)
	add_library(ham::runtime ALIAS ham-runtime)

	target_compile_features(ham-runtime INTERFACE c_std_17 cxx_std_20)

	target_link_libraries(ham-runtime INTERFACE ${Ham_LIBRARIES})
	target_link_directories(ham-runtime INTERFACE ${Ham_LIBRARY_DIRS})
	target_include_directories(ham-runtime INTERFACE ${Ham_INCLUDE_DIRS})
endif()
