#
# Ham World Engine Editor
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

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

ham_add_executable(
	ham-engine-editor
	images.qrc
	fonts.qrc

	main.cpp

	window.hpp
	window.cpp

	welcome_wizard.hpp
	welcome_wizard.cpp

	main_window.hpp
	main_window.cpp
)

set_target_properties(
	ham-engine-editor
	PROPERTIES
	    WIN32_EXECUTABLE ON
		MACOSX_BUNDLE    ON
		ENABLE_EXPORTS   ON
		AUTORCC          ON
		AUTOMOC          ON
)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
	target_link_options(ham-engine-client PRIVATE "-rdynamic")
endif()

target_link_libraries(
	ham-engine-editor
	PRIVATE
	    ham::engine
		Qt6::Widgets
)
