/*
 * Ham Runtime
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
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

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
	(ham_likely(cond_) ? true : (ham_logapierrorf((fmt_str_) __VA_OPT__(,) __VA_ARGS__), false))
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
