if(READLINE_DIR)
	find_library(
		READLINE
		NAMES readline
		HINTS ${READLINE_DIR}
	)
else()
	find_package(PkgConfig REQUIRED)
	pkg_check_modules(READLINE readline)
endif()

if(READLINE_FOUND AND NOT TARGET readline_lib)
	add_library(readline_lib INTERFACE)
	add_library(readline::readline ALIAS readline_lib)

	target_compile_options    (readline_lib INTERFACE ${READLINE_CFLAGS})
	target_link_options       (readline_lib INTERFACE ${READLINE_LDFLAGS})
	target_include_directories(readline_lib INTERFACE ${READLINE_INCLUDE_DIRS})
	target_link_directories   (readline_lib INTERFACE ${READLINE_LIBRARY_DIRS})
	target_link_libraries     (readline_lib INTERFACE ${READLINE_LIBRARIES})
endif()
