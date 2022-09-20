if(MAGIC_DIR)
	find_library(
		MAGIC
		NAMES libmagic magic
		HINTS ${MAGIC_DIR}
	)
else()
	find_package(PkgConfig REQUIRED)
	pkg_check_modules(MAGIC libmagic)
endif()

if(MAGIC_FOUND AND NOT TARGET magic_lib)
	add_library(magic_lib INTERFACE)
	add_library(magic::magic ALIAS magic_lib)

	target_compile_options    (magic_lib INTERFACE ${MAGIC_CFLAGS})
	target_link_options       (magic_lib INTERFACE ${MAGIC_LDFLAGS})
	target_include_directories(magic_lib INTERFACE ${MAGIC_INCLUDE_DIRS})
	target_link_directories   (magic_lib INTERFACE ${MAGIC_LIBRARY_DIRS})
	target_link_libraries     (magic_lib INTERFACE ${MAGIC_LIBRARIES})
endif()
