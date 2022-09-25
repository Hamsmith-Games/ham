if(STEAMWORKS_FOUND)
	return()
endif()

if(NOT STEAMWORKS_SDK_PATH)
	set(STEAMWORKS_FOUND FALSE)
	return()
endif()

if(NOT EXISTS ${STEAMWORKS_SDK_PATH})
	message(ERROR "Steamworks SDK could not be found: ${STEAMWORKS_SDK_PATH}")
	set(STEAMWORKS_FOUND FALSE)
	return()
endif()

file(
	ARCHIVE_EXTRACT
	INPUT ${STEAMWORKS_SDK_PATH}
	DESTINATION ${PROJECT_BINARY_DIR}/steamworks
)

set(STEAMWORKS_SOURCE_DIR ${PROJECT_BINARY_DIR}/steamworks/sdk)

if(APPLE)
	set(STEAMWORKS_LIB_SUBDIR osx)
elseif(UNIX)
	set(STEAMWORKS_LIB_SUBDIR linux64)
elseif(WIN32)
	set(STEAMWORKS_LIB_SUBDIR win64)
else()
	message(ERROR "Unsupported Steamworks SDK platform")
	set(STEAMWORKS_FOUND FALSE)
	return()
endif()

set(
	STEAMWORKS_INCLUDE_DIRS
	$<BUILD_INTERFACE:${STEAMWORKS_SOURCE_DIR}/public>
)

set(
	STEAMWORKS_LIBRARY_DIRS
	$<BUILD_INTERFACE:${STEAMWORKS_SOURCE_DIR}/redistributable_bin/${STEAMWORKS_LIB_SUBDIR}>
	$<BUILD_INTERFACE:${STEAMWORKS_SOURCE_DIR}/public/lib/${STEAMWORKS_LIB_SUBDIR}>
)

add_library(Steamworks INTERFACE)
add_library(Steamworks::Steamworks ALIAS Steamworks)

target_include_directories(Steamworks INTERFACE ${STEAMWORKS_INCLUDE_DIRS})
target_link_directories(Steamworks INTERFACE ${STEAMWORKS_LIBRARY_DIRS})
target_link_libraries(Steamworks INTERFACE steam_api)

message(STATUS "Steamworks SDK found: ${STEAMWORKS_SDK_PATH}")
set(STEAMWORKS_FOUND TRUE)
