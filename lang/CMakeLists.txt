#
# The Ham Language
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

add_subdirectory(lib)
add_subdirectory(interpreter)
add_subdirectory(compiler)

if(HAM_INSTALL_LANG)
	install(
		FILES interpreter/LICENSE.md
		DESTINATION ${CMAKE_INSTALL_DATADIR}/licenses/ham/hami
	)

    install(
		FILES compiler/LICENSE.md
		DESTINATION ${CMAKE_INSTALL_DATADIR}/licenses/ham/hamc
	)

    install(
		TARGETS hamc hami
		RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	)
endif()
