find_package(PkgConfig)

if(NOT PKG_CONFIG_FOUND)
	set(MPFR_FOUND FALSE)
	return()
endif()

pkg_check_modules(MPFR_PC QUIET mpfr)

if(NOT MPFR_PC_FOUND)
	set(MPFR_FOUND FALSE)
	return()
endif()

set(MPFR_LIBRARIES ${MPFR_PC_LIBRARIES})
set(MPFR_INCLUDE_DIRS ${MPFR_PC_INCLUDE_DIRS})

if(NOT TARGET mpfr)
	add_library(mpfr INTERFACE)
	target_link_libraries(mpfr INTERFACE ${MPFR_LIBRARIES})
	target_include_directories(mpfr INTERFACE ${MPFR_INCLUDE_DIRS})
	add_library(mpfr::mpfr ALIAS mpfr)
endif()

set(MPFR_FOUND TRUE)
