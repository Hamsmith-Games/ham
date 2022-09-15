find_package(PkgConfig REQUIRED)

pkg_check_modules(GMP REQUIRED gmp)
pkg_check_modules(MPFR REQUIRED mpfr)

find_package(ICU REQUIRED COMPONENTS uc)

set(
	HAM_SOURCES
	${HAM_HEADERS}
	memory.cpp
	lex.cpp
	parse.cpp
)

add_library(ham SHARED ${HAM_SOURCES})

target_compile_features(ham PUBLIC c_std_11 cxx_std_20)

target_include_directories(
	ham PUBLIC
	$<BUILD_INTERFACE:${HAM_SOURCE_INCLUDE_DIR}>
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(ham PUBLIC ${GMP_LIBRARIES} ${MPFR_LIBRARIES})
target_include_directories(ham PUBLIC ${GMP_INCLUDE_DIRS} ${MPFR_INCLUDE_DIRS})

target_link_libraries(ham PRIVATE ICU::uc robin_hood::robin_hood)

add_library(ham::ham ALIAS ham)
