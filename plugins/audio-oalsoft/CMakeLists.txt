#
# Hamsmith Runtime Plugins
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

##
# Dependencies
##

FetchContent_Declare(
	oalsoft_content
	GIT_REPOSITORY https://github.com/kcat/openal-soft.git
	GIT_TAG        dc83d99c95a42c960150ddeee06c124134b52208
)

FetchContent_GetProperties(oalsoft_content)
if(NOT oalsoft_content_POPULATED)
	FetchContent_Populate(oalsoft_content)

	set(ALSOFT_UTILS OFF CACHE BOOL "" FORCE)
	set(ALSOFT_NO_CONFIG_UTIL ON CACHE BOOL "" FORCE)
	set(ALSOFT_EXAMPLES OFF CACHE BOOL "" FORCE)
	set(ALSOFT_INSTALL OFF CACHE BOOL "" FORCE)
	set(ALSOFT_INSTALL_CONFIG OFF CACHE BOOL "" FORCE)
	set(ALSOFT_INSTALL_EXAMPLES OFF CACHE BOOL "" FORCE)
	set(ALSOFT_INSTALL_UTILS OFF CACHE BOOL "" FORCE)
	set(ALSOFT_UPDATE_BUILD_VERSION OFF CACHE BOOL "" FORCE)
	set(ALSOFT_EMBED_HRTF_DATA ON CACHE BOOL "" FORCE)
	add_subdirectory(${oalsoft_content_SOURCE_DIR} ${oalsoft_content_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

##
# Targets
##

ham_add_plugin(
	ham-audio-oalsoft
	audio_context_oalsoft.cpp
)

target_link_libraries(ham-audio-oalsoft PRIVATE OpenAL)
target_include_directories(ham-audio-oalsoft PRIVATE ${oalsoft_content_SOURCE_DIR}/include)
