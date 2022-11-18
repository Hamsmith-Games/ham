#
# Ham Runtime Tests
# Copyright (C) 2022 Keith Hammond
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

FetchContent_Declare(
	glm_content
	GIT_REPOSITORY https://github.com/g-truc/glm.git
	GIT_TAG        cc98465e3508535ba8c7f6208df934c156a018dc
)

FetchContent_GetProperties(glm_content)
if(NOT glm_content_POPULATED)
	FetchContent_Populate(glm_content)

	add_subdirectory(${glm_content_SOURCE_DIR} ${glm_content_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

ham_add_executable(
	ham-test
	tests.hpp
	test-meta.cpp
	test-object.cpp
	test-utf.cpp
	test-lex.cpp
	test-parse.cpp
	test-buffer.cpp
	test-octree.cpp
	test-mat.cpp
	test-quat.cpp
	test-transform.cpp
	test-camera.cpp
	test-typesys.cpp
	main.cpp
)

target_link_libraries(ham-test PRIVATE glm::glm)
