find_package(ICU REQUIRED COMPONENTS uc)

add_library(ham lex.cpp)

target_compile_features(ham PUBLIC c_std_11 cxx_std_20)

target_include_directories(
	ham PUBLIC
	$<BUILD_INTERFACE:${HAM_SOURCE_INCLUDE_DIR}>
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(ham PUBLIC ICU::uc)
