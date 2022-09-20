#ifndef HAM_CHECK_H
#define HAM_CHECK_H 1

/**
 * @defgroup HAM_CHECK Checking and assertions
 * @ingroup HAM
 * @{
 */

#include "log.h"

HAM_C_API_BEGIN

//! @cond ignore
#define ham_impl_check_msg(cond_, fmt_str_, ...) \
	((cond_) ? true : (ham_logapierrorf((fmt_str_) __VA_OPT__(,) __VA_ARGS__), false))
//! @endcond

#define ham_check_msg(cond_, fmt_str_, ...) ham_impl_check_msg(cond_, fmt_str_ __VA_OPT__(,) __VA_ARGS__)

//! @cond ignore
#define ham_impl_check(cond_) ham_check_msg(cond_, "Condition not met: %s", #cond_)
//! @endcond

#define ham_check(cond_) ham_impl_check(cond_)

HAM_C_API_END

/**
 * @}
 */

#endif // !HAM_CHECK_H
