#
# Ham Programming Language Tests
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

add_executable(
	ham-test
	tests.hpp
	test-meta.cpp
	test-object.cpp
	test-utf.cpp
	test-lex.cpp
	test-parse.cpp
	main.cpp
)

target_link_libraries(ham-test PRIVATE ${HAM_CPU_LIBS})
target_link_libraries(ham-test PUBLIC ham::ham)
