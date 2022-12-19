#
# Ham Runtime Plugins
# Copyright (C) 2022 Keith Hammond
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

FetchContent_Declare(
	bullet3_content
	GIT_REPOSITORY https://github.com/bulletphysics/bullet3.git
	GIT_TAG        2c204c49e56ed15ec5fcfa71d199ab6d6570b3f5
)

FetchContent_Declare(
	vhacd_content
	GIT_REPOSITORY https://github.com/kmammou/v-hacd.git
	GIT_TAG        22ec20a7f8ea221ab600df869369b9c6a258cb10
)

FetchContent_GetProperties(vhacd_content)
if(NOT vhacd_content_POPULATED)
	FetchContent_Populate(vhacd_content)

	add_library(VHACD STATIC VHACD.cpp)
	add_library(VHACD::VHACD ALIAS VHACD)

	target_compile_options(VHACD PRIVATE ${HAM_CPU_FLAGS})
	target_include_directories(VHACD PUBLIC ${vhacd_content_SOURCE_DIR}/include)
endif()

FetchContent_GetProperties(bullet3_content)
if(NOT bullet3_content_POPULATED)
	FetchContent_Populate(bullet3_content)

	set(BUILD_SHARED_LIBS_OLD ${BUILD_SHARED_LIBS})
	set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

	set(USE_GRAPHICAL_BENCHMARK OFF CACHE BOOL "" FORCE)
	set(BUILD_CPU_DEMOS OFF CACHE BOOL "" FORCE)
	set(BUILD_BULLET2_DEMOS OFF CACHE BOOL "" FORCE)
	set(BUILD_OPENGL3_DEMOS OFF CACHE BOOL "" FORCE)
	set(BUILD_PYBULLET OFF CACHE BOOL "" FORCE)
	set(BUILD_UNIT_TESTS OFF CACHE BOOL "" FORCE)
	set(INSTALL_CMAKE_FILES OFF CACHE BOOL "" FORCE)
	set(INSTALL_LIBS OFF CACHE BOOL "" FORCE)

	set(USE_SOFT_BODY_MULTI_BODY_DYNAMICS_WORLD ON CACHE BOOL "" FORCE)
	set(BULLET2_MULTITHREADING ON CACHE BOOL "" FORCE)
	set(BUILD_EXTRAS ON CACHE BOOL "" FORCE)
	set(BUILD_BULLET3 ON CACHE BOOL "" FORCE)

	add_subdirectory(${bullet3_content_SOURCE_DIR} ${bullet3_content_BINARY_DIR} EXCLUDE_FROM_ALL)

	set(BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS_OLD} CACHE BOOL "" FORCE)
endif()

find_package(OpenCL REQUIRED)

ham_add_plugin(
	ham-physics-bullet3

	physics_bullet3.h
	physics_bullet3.cpp

	physics_world_bullet3.cpp
	physics_shape_bullet3.cpp
	physics_body_bullet3.cpp
)

target_include_directories(ham-physics-bullet3 PRIVATE ${BULLET_PHYSICS_SOURCE_DIR}/src)
target_link_libraries(
	ham-physics-bullet3 PRIVATE

	OpenCL::OpenCL
	VHACD::VHACD

	Bullet3OpenCL_clew Bullet2FileLoader Bullet3Dynamics Bullet3Collision Bullet3Geometry
	Bullet3Common

	# order is important here
	BulletSoftBody BulletDynamics BulletCollision LinearMath BulletInverseDynamics
)
