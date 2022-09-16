find_package(ICU REQUIRED COMPONENTS uc)
find_package(GMP REQUIRED)
find_package(MPFR REQUIRED)

set(
	HAM_SOURCES
	${HAM_HEADERS}
	memory.cpp
	str_buffer.cpp
	lex.cpp
	#parse.cpp
)

add_library(ham SHARED ${HAM_SOURCES})

set_target_properties(
	ham PROPERTIES
	C_STANDARD 11
	CXX_STANDARD 20
	C_VISIBILITY_PRESET hidden
	CXX_VISIBILITY_PRESET hidden
	VISIBILITY_INLINES_HIDDEN ON
)

target_compile_definitions(ham PRIVATE HAM_LIB_IMPLEMENTATION)

target_compile_features(ham PUBLIC c_std_11 cxx_std_20)

target_include_directories(
	ham PUBLIC
	$<BUILD_INTERFACE:${HAM_SOURCE_INCLUDE_DIR}>
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(ham PUBLIC gmp::gmp mpfr::mpfr)
target_link_libraries(ham PRIVATE ICU::uc robin_hood::robin_hood)

add_library(ham::ham ALIAS ham)
