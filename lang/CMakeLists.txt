add_subdirectory(lib)
add_subdirectory(interpreter)
add_subdirectory(compiler)

if(HAM_INSTALL_LANG)
	install(
		FILES interpreter/LICENSE.md
		DESTINATION ${CMAKE_INSTALL_DATADIR}/licenses/ham/hami
	)

    install(
		FILES compiler/LICENSE.md
		DESTINATION ${CMAKE_INSTALL_DATADIR}/licenses/ham/hamc
	)

    install(
		TARGETS hamc hami
		RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	)
endif()
