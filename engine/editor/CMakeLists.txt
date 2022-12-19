#
# Ham World Engine Editor
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

##
# Tools to be included in distribution
##

# FetchContent_Declare(
# 	cmake_content
# 	GIT_REPOSITORY https://gitlab.kitware.com/cmake/cmake.git
# 	GIT_TAG        31f835410efeea50acd43512eb9e5646a26ea177 # v3.24.2
# )
#
# FetchContent_GetProperties(cmake_content)
# if(NOT cmake_content_POPULATED)
# 	FetchContent_Populate(cmake_content)
#
# 	add_subdirectory(${cmake_content_SOURCE_DIR} ${cmake_content_BINARY_DIR} EXCLUDE_FROM_ALL)
# endif()

##
# Dependencies
##

include(ExternalProject)
include(ProcessorCount)

ProcessorCount(NPROC)

set(QSCINTILLA_VERSION 2.13.3)

set(QSCINTILLA_ARCHIVE_NAME "QScintilla_src-${QSCINTILLA_VERSION}.tar.gz")
set(QSCINTILLA_ARCHIVE_URL "https://www.riverbankcomputing.com/static/Downloads/QScintilla/${QSCINTILLA_VERSION}/${QSCINTILLA_ARCHIVE_NAME}")
set(QSCINTILLA_ARCHIVE_SHA256 "711d28e37c8fccaa8229e8e39a5b3b2d97f3fffc63da10b71c71b84fa3649398")

FetchContent_Declare(
	qscintilla_content
	URL "${QSCINTILLA_ARCHIVE_URL}"
	URL_HASH "SHA256=${QSCINTILLA_ARCHIVE_SHA256}"
	DOWNLOAD_EXTRACT_TIMESTAMP ON
)

#FetchContent_MakeAvailable(qscintilla_content)
FetchContent_GetProperties(qscintilla_content)
if(NOT qscintilla_content_POPULATED)
	FetchContent_Populate(qscintilla_content)
endif()

set(QSCINTILLA_LIBRARY "${qscintilla_content_BINARY_DIR}/libqscintilla2_qt6${CMAKE_SHARED_LIBRARY_SUFFIX}.15")

message(STATUS "QScintilla library path: ${QSCINTILLA_LIBRARY}")

find_package(Qt6 REQUIRED COMPONENTS Widgets OpenGLWidgets)

add_custom_command(
	OUTPUT ${QSCINTILLA_LIBRARY}
	COMMAND Qt6::qmake $<$<CONFIG:Debug>:-d> ${qscintilla_content_SOURCE_DIR}/src/qscintilla.pro
	COMMAND make -j${NPROC}

	WORKING_DIRECTORY ${qscintilla_content_BINARY_DIR}
)

add_library(QScintilla SHARED IMPORTED)
set_target_properties(QScintilla PROPERTIES IMPORTED_LOCATION "${QSCINTILLA_LIBRARY}")
target_include_directories(QScintilla INTERFACE "${qscintilla_content_SOURCE_DIR}/src")
add_dependencies(QScintilla qscintilla_content)

FetchContent_Declare(
	mustach_content
	GIT_REPOSITORY https://gitlab.com/jobol/mustach.git
	GIT_TAG        a65e3a24a40aa479879a03ff3d7fb4288af65ea6 # 1.2.2
)

FetchContent_GetProperties(mustach_content)
if(NOT mustach_content_POPULATED)
	FetchContent_Populate(mustach_content)

	add_library(mustach OBJECT ${mustach_content_SOURCE_DIR}/mustach.c)
	add_library(mustach::mustach ALIAS mustach)
	target_include_directories(mustach PUBLIC ${mustach_content_SOURCE_DIR})
	target_link_libraries(mustach PRIVATE ${HAM_CPU_LIBS})
endif()

##
# Editor targets
##

qt_standard_project_setup()

ham_add_executable(
	ham-engine-editor
	images.qrc
	fonts.qrc
	templates.qrc
	models.qrc

	main.cpp

	physics_backend.hpp
	physics_backend.cpp

	engine_instance.hpp

	util.hpp

	window.hpp
	window.cpp

	project.hpp
	project.cpp

	welcome_wizard.hpp
	welcome_wizard.cpp

	main_window.hpp
	main_window.cpp

	renderer_widget.hpp
	renderer_widget.cpp
	renderer_widget_gl.cpp
	renderer_widget_vulkan.cpp

	world_view.hpp
	world_view.cpp

	source_editor.hpp
	source_editor.cpp

	graph_editor.hpp
	graph_editor.cpp
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
		Qt6::Widgets Qt6::OpenGLWidgets
		mustach::mustach
		QScintilla
)
