if(GMP_DIR)
	find_library(
		GMP
		NAMES gmp
		HINTS ${GMP_DIR}
	)
else()
	find_package(PkgConfig REQUIRED)
	pkg_check_modules(GMP gmp)
endif()

if(GMP_FOUND AND NOT TARGET gmp_lib)
	add_library(gmp_lib INTERFACE)
	add_library(gmp::gmp ALIAS gmp_lib)

	target_compile_options    (gmp_lib INTERFACE ${GMP_CFLAGS})
	target_link_options       (gmp_lib INTERFACE ${GMP_LDFLAGS})
	target_include_directories(gmp_lib INTERFACE ${GMP_INCLUDE_DIRS})
	target_link_directories   (gmp_lib INTERFACE ${GMP_LIBRARY_DIRS})
	target_link_libraries     (gmp_lib INTERFACE ${GMP_LIBRARIES})
endif()
