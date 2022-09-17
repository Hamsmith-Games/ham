/**
 * The Ham Programming Language
 * Copyright (C) 2022  Hamsmith Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef HAM_CONFIG_H
#define HAM_CONFIG_H 1

/**
 * @defgroup HAM_CONFIG Library configuration
 * @ingroup HAM
 * @{
 */

#if !defined(__GNUC__) && !defined(__cplusplus)
#	error "Ham requires GNU C extensions or C++"
#endif

#ifndef HAM_NAME_BUFFER_SIZE
#	define HAM_NAME_BUFFER_SIZE 128
#endif

#ifndef HAM_PATH_BUFFER_SIZE
#	define HAM_PATH_BUFFER_SIZE 256
#endif

#ifndef HAM_MESSAGE_BUFFER_SIZE
#	define HAM_MESSAGE_BUFFER_SIZE 512
#endif

#define HAM_CONCAT_(a, b) a##b
#define HAM_CONCAT(a, b) HAM_CONCAT_(a, b)

#ifdef HAM_UTF32
#	define HAM_UTF 32
#elif defined(HAM_UTF16)
#	define HAM_UTF 16
#else
#	define HAM_UTF8
#	define HAM_UTF 8
#endif

#ifdef __cplusplus
#	define HAM_C_API_BEGIN extern "C" {
#	define HAM_C_API_END }
#	define ham_null nullptr
#	define ham_auto auto
#	define ham_constexpr constexpr
#	define ham_thread_local thread_local
#else
#	define HAM_C_API_BEGIN
#	define HAM_C_API_END
#	define ham_null NULL
#	define ham_constexpr
#endif

#ifdef _WIN32
#	define ham_import __declspec(dllimport)
#	define ham_export __declspec(dllexport)
#	define ham_nothrow __declspec(nothrow)
#	define ham_popcnt16 __popcnt16
#	define ham_popcnt32 __popcnt
#	define ham_popcnt64 __popcnt64
#else
#	define ham_import
#	define ham_export
#endif

#ifdef __GNUC__

#	define ham_public __attribute__((visibility("default")))
#	define ham_private __attribute__((visibility("hidden")))

#	ifndef ham_nothrow
#		define ham_nothrow __attribute__((nothrow))
#	endif

#	ifndef ham_auto
#		define ham_auto __auto_type
#	endif

#	ifndef ham_thread_local
#		define ham_thread_local __thread
#	endif

#	ifndef ham_popcnt16
#		define ham_popcnt16 __builtin_popcount
#	endif

#	ifndef ham_popcnt32
#		define ham_popcnt32 __builtin_popcount
#	endif

#	ifndef ham_popcnt64
#		define ham_popcnt64 __builtin_popcountl
#	endif

#	define ham_min(a, b) \
		({	const ham_auto a_ = (a); const ham_auto b_ = (b); \
			a_ < b_ ? a_ : b_; })

#	define ham_max(a, b) \
		({	const ham_auto a_ = (a); const ham_auto b_ = (b); \
			a_ < b_ ? b_ : a_; })

#else

#	define ham_public
#	define ham_private

#	define ham_min(a, b) \
		([](const auto a_; const auto b_) constexpr{ \
			return a_ < b_ ? a_ : b_; \
		}((a), (b)))

#	define ham_min(a, b) \
		([](const auto a_; const auto b_) constexpr{ \
			return a_ < b_ ? b_ : a_; \
		}((a), (b)))

#endif // __GNUC__

#define ham_popcnt ham_popcnt32

#ifdef HAM_LIB_IMPLEMENTATION
#	define ham_api ham_public ham_export
#else
#	define ham_api ham_public ham_import
#endif


/**
 * @}
 */

#endif // !HAM_CONFIG_H
