#ifndef HAM_CONFIG_H
#define HAM_CONFIG_H 1

/**
 * @defgroup HAM_CONFIG Library configuration
 * @ingroup HAM
 * @{
 */

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
#	define ham_constexpr constexpr
#else
#	define HAM_C_API_BEGIN
#	define HAM_C_API_END
#	define ham_constexpr
#endif

#ifdef _WIN32
#	define ham_import __declspec(dllimport)
#	define ham_export __declspec(dllexport)
#else
#	define ham_import
#	define ham_export
#endif

#ifdef __GNUC__
#	define ham_public __attribute__((visibility("default")))
#	define ham_private __attribute__((visibility("hidden")))
#else
#	define ham_public
#	define ham_private
#endif

#ifdef HAM_LIB_IMPLEMENTATION
#	define ham_api ham_public ham_export
#else
#	define ham_api ham_public ham_import
#endif

/**
 * @}
 */

#endif // !HAM_CONFIG_H
