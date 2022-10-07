##
# This project is not designed to be used directly and is
# intended for the main Ham `CMakeLists.txt` when building for android
##

cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

project(
	ham-android-deps
	VERSION 0.0.1
	DESCRIPTION "Ham Android dependencies"
)

include(ExternalProject)
include(FetchContent)
include(ProcessorCount)

ProcessorCount(NPROC)

if(NOT EXISTS "${CMAKE_INSTALL_PREFIX}/lib/libssl.a")
	find_program(MAKE_EXE NAMES make gmake nmake)

	file(REAL_PATH "${CMAKE_INSTALL_PREFIX}" OPENSSL_INSTALL_PREFIX)

	ExternalProject_Add(
		openssl_ext

		URL "https://www.openssl.org/source/openssl-3.0.5.tar.gz"
		URL_HASH "SHA256=aa7d8d9bef71ad6525c55ba11e5f4397889ce49c2c9349dcea6d3e4f0b024a7a"
		DOWNLOAD_EXTRACT_TIMESTAMP ON

		CONFIGURE_COMMAND ./Configure android-arm64 --release "--prefix=${OPENSSL_INSTALL_PREFIX}" "--openssldir=${OPENSSL_INSTALL_PREFIX}"
		BUILD_COMMAND ${MAKE_EXE} -j${NPROC}
		INSTALL_COMMAND ${MAKE_EXE} install

		BUILD_IN_SOURCE ON
	)
endif()

FetchContent_Declare(
	protobuf_content
	GIT_REPOSITORY https://github.com/protocolbuffers/protobuf.git
	GIT_TAG        54489e95e01882407f356f83c9074415e561db00
)

FetchContent_MakeAvailable(protobuf_content)
