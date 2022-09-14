find_package(PkgConfig)

if(NOT PKG_CONFIG_FOUND)
	set(GMP_FOUND FALSE)
	return()
endif()

pkg_check_modules(GMP_PC QUIET gmp)

if(NOT GMP_PC_FOUND)
	set(GMP_FOUND FALSE)
	return()
endif()

set(GMP_LIBRARIES ${GMP_PC_LIBRARIES})
set(GMP_INCLUDE_DIRS ${GMP_PC_INCLUDE_DIRS})

if(NOT TARGET gmp)
	add_library(gmp INTERFACE)
	target_link_libraries(gmp INTERFACE ${GMP_LIBRARIES})
	target_include_directories(gmp INTERFACE ${GMP_INCLUDE_DIRS})
	add_library(gmp::gmp ALIAS gmp)
endif()

set(GMP_FOUND TRUE)
