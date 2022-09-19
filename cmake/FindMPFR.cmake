if(MPFR_DIR)
	find_library(
		MPFR
		NAMES mpfr
		HINTS ${MPFR_DIR}
	)
else()
	find_package(PkgConfig REQUIRED)
	pkg_check_modules(MPFR mpfr)
endif()

if(MPFR_FOUND AND NOT TARGET mpfr_lib)
	add_library(mpfr_lib INTERFACE)
	add_library(mpfr::mpfr ALIAS mpfr_lib)

	target_compile_options    (mpfr_lib INTERFACE ${MPFR_CFLAGS})
	target_link_options       (mpfr_lib INTERFACE ${MPFR_LDFLAGS})
	target_link_directories   (mpfr_lib INTERFACE ${MPFR_LIBRARY_DIRS})
	target_include_directories(mpfr_lib INTERFACE ${MPFR_INCLUDE_DIRS})
	target_link_libraries     (mpfr_lib INTERFACE ${MPFR_LIBRARIES})
endif()
