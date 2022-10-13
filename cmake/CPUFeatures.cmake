set(cpu_archdetect_c_code "
#if defined(__arm__) || defined(__TARGET_ARCH_ARM)
    #if defined(__ARM_ARCH_7__) \\
        || defined(__ARM_ARCH_7A__) \\
        || defined(__ARM_ARCH_7R__) \\
        || defined(__ARM_ARCH_7M__) \\
        || (defined(__TARGET_ARCH_ARM) && __TARGET_ARCH_ARM-0 >= 7)
        #undef armv7
armv7
    #elif defined(__ARM_ARCH_6__) \\
        || defined(__ARM_ARCH_6J__) \\
        || defined(__ARM_ARCH_6T2__) \\
        || defined(__ARM_ARCH_6Z__) \\
        || defined(__ARM_ARCH_6K__) \\
        || defined(__ARM_ARCH_6ZK__) \\
        || defined(__ARM_ARCH_6M__) \\
        || (defined(__TARGET_ARCH_ARM) && __TARGET_ARCH_ARM-0 >= 6)
        #undef armv6
armv6
    #elif defined(__ARM_ARCH_5TEJ__) \\
        || (defined(__TARGET_ARCH_ARM) && __TARGET_ARCH_ARM-0 >= 5)
        #undef armv5
armv5
    #else
    	#undef arm
arm
    #endif
#elif defined(__i386) || defined(__i386__) || defined(_M_IX86)
	#undef i386
i386
#elif defined(__x86_64) || defined(__x86_64__) || defined(__amd64) || defined(_M_X64)
	#undef x86_64
x86_64
#elif defined(__ia64) || defined(__ia64__) || defined(_M_IA64)
	#undef ia64
ia64
#elif defined(__ppc__) || defined(__ppc) || defined(__powerpc__) \\
      || defined(_ARCH_COM) || defined(_ARCH_PWR) || defined(_ARCH_PPC)  \\
      || defined(_M_MPPC) || defined(_M_PPC)
    #if defined(__ppc64__) || defined(__powerpc64__) || defined(__64BIT__)
    	#undef ppc64
ppc64
    #else
    	#undef ppc
ppc
    #endif
#else
unknown
#endif
")

function(check_target_arch out)
	if("${ANDROID_ABI}" STREQUAL "armeabi-v7a")
		set(${out} armv7 PARENT_SCOPE)
	elseif("${ANDROID_ABI}" STREQUAL "arm64-v8a")
		set(${out} armv8 PARENT_SCOPE)
	elseif("${ANDROID_ABI}" STREQUAL "x86")
		set(${out} i386 PARENT_SCOPE)
	elseif("${ANDROID_ABI}" STREQUAL "x86_64")
		set(${out} x86_64 PARENT_SCOPE)
	else()
		file(WRITE "${CMAKE_BINARY_DIR}/cpu_features-archdetect.h" "${cpu_archdetect_c_code}")

		if(MSVC)
			set(CPU_FEATURES_PREPROCESS "${CMAKE_C_COMPILER}) /EP /nologo")
		else()
			set(CPU_FEATURES_PREPROCESS ${CMAKE_C_COMPILER} -E -P)
		endif()

		execute_process(
			COMMAND ${CPU_FEATURES_PREPROCESS} "${CMAKE_BINARY_DIR}/cpu_features-archdetect.h"
			OUTPUT_VARIABLE CPU_FEATURES_PROCESSOR
			OUTPUT_STRIP_TRAILING_WHITESPACE
		)

		string(STRIP "${CPU_FEATURES_PROCESSOR}" CPU_FEATURES_PROCESSOR)
		set(${out} "${CPU_FEATURES_PROCESSOR}" PARENT_SCOPE)
	endif()
endfunction()

macro(add_cpu_feature_lib name gnu-flags msvc-flags)
	add_library(cpu-${name} INTERFACE IMPORTED)
	add_library(cpu::${name} ALIAS cpu-${name})
	if(MSVC)
		target_compile_options(cpu-${name} INTERFACE ${msvc-flags})
	else()
		target_compile_options(cpu-${name} INTERFACE ${gnu-flags})
	endif()
endmacro()

add_cpu_feature_lib(armv8-a   "-march=armv8-a"   "/arch:armv8.0")
add_cpu_feature_lib(armv8.1-a "-march=armv8.1-a" "/arch:armv8.1")
add_cpu_feature_lib(armv8.2-a "-march=armv8.2-a" "/arch:armv8.2")
add_cpu_feature_lib(armv8.3-a "-march=armv8.3-a" "/arch:armv8.3")
add_cpu_feature_lib(armv8.4-a "-march=armv8.4-a" "/arch:armv8.4")
add_cpu_feature_lib(armv8.5-a "-march=armv8.5-a" "/arch:armv8.5")
add_cpu_feature_lib(armv8.6-a "-march=armv8.6-a" "/arch:armv8.6")
add_cpu_feature_lib(armv8.7-a "-march=armv8.7-a" "/arch:armv8.7")
add_cpu_feature_lib(armv8.8-a "-march=armv8.8-a" "/arch:armv8.8")

add_cpu_feature_lib(x86-64-v2 "-march=x86-64-v2" "/arch:SSE4.2")
add_cpu_feature_lib(x86-64-v3 "-march=x86-64-v3" "/arch:AVX2")
add_cpu_feature_lib(x86-64-v4 "-march=x86-64-v4" "/arch:AVX512")

add_cpu_feature_lib(sse2 "-msse2" "/arch:SSE2")
add_cpu_feature_lib(sse3 "-msse2" "/arch:SSE3")
add_cpu_feature_lib(sse4_1 "-msse4.1" "/arch:SSE4.1")
add_cpu_feature_lib(sse4_2 "-msse4.2" "/arch:SSE4.2")
add_cpu_feature_lib(avx "-mavx" "/arch:AVX")
add_cpu_feature_lib(avx2 "-mavx2" "/arch:AVX2")
add_cpu_feature_lib(
	avx512
	"-mavx512f -mavx512pf -mavx512er -mavx512cd -mavx512vl -mavx512bw -mavx512dq -mavx512ifma -mavx512vbmi"
	"/arch:AVX512"
)
