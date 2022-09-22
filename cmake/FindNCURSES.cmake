if(NCURSES_DIR)
	find_library(
		NCURSES
		NAMES
			ncursesw libncursesw
			ncurses libncurses
		HINTS ${NCURSES_DIR}
	)
else()
	find_package(PkgConfig REQUIRED)
	pkg_check_modules(NCURSES ncurses)
endif()

if(NCURSES_FOUND AND NOT TARGET ncurses_lib)
	add_library(ncurses_lib INTERFACE)
	add_library(ncurses::ncurses ALIAS ncurses_lib)

	target_compile_options    (ncurses_lib INTERFACE ${NCURSES_CFLAGS})
	target_link_options       (ncurses_lib INTERFACE ${NCURSES_LDFLAGS})
	target_include_directories(ncurses_lib INTERFACE ${NCURSES_INCLUDE_DIRS})
	target_link_directories   (ncurses_lib INTERFACE ${NCURSES_LIBRARY_DIRS})
	target_link_libraries     (ncurses_lib INTERFACE ${NCURSES_LIBRARIES})
endif()
