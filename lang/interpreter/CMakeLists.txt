#
# Ham Programming Language Interpreter
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

find_package(Readline REQUIRED)

ham_add_executable(hami main.cpp)

add_executable(ham::interpreter ALIAS hami)

target_link_libraries(hami PRIVATE readline::readline)
