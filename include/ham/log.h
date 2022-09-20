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
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef HAM_LOG_H
#define HAM_LOG_H 1

/**
 * @defgroup HAM_LOG Logging
 * @ingroup HAM
 * @{
 */

#include "time.h"

#include <stdarg.h>

HAM_C_API_BEGIN

typedef enum ham_log_level{
	HAM_LOG_INFO,
	HAM_LOG_VERBOSE,
	HAM_LOG_DEBUG,
	HAM_LOG_WARNING,
	HAM_LOG_ERROR,
	HAM_LOG_FATAL,

	HAM_LOG_LEVEL_COUNT,
} ham_log_level;

ham_constexpr ham_nothrow static inline const char *ham_log_level_str(ham_log_level level){
	switch(level){
		case HAM_LOG_INFO:    return "INFO";
		case HAM_LOG_VERBOSE: return "VERBOSE";
		case HAM_LOG_DEBUG:   return "DEBUG";
		case HAM_LOG_WARNING: return "WARNING";
		case HAM_LOG_ERROR:   return "ERROR";
		case HAM_LOG_FATAL:   return "FATAL";
		default:              return "UNKNOWN";
	}
}

typedef ham_usize(*ham_log_fn)(ham_timepoint tp, ham_log_level level, const char *api, const char *message, void *user);

typedef struct ham_logger{
	ham_log_fn log;
	void *user;
} ham_logger;

//! @cond ignore
ham_api extern const ham_logger *const ham_impl_default_logger;
ham_api extern const ham_logger *ham_impl_global_logger;
ham_api extern ham_thread_local ham_message_buffer ham_impl_message_buf;
ham_api extern ham_thread_local const ham_logger *ham_impl_thread_logger;
ham_api extern ham_thread_local const ham_logger *const *ham_impl_current_logger;
//! @endcond

ham_nothrow static inline const ham_logger *ham_current_logger(){
	return *ham_impl_current_logger;
}

ham_nothrow static inline void ham_set_logger(const ham_logger *logger){
	ham_impl_thread_logger = logger;
	ham_impl_current_logger = logger ? &ham_impl_thread_logger : &ham_impl_global_logger;
}

ham_nothrow static inline void ham_set_global_logger(const ham_logger *logger){
	ham_impl_global_logger = logger ? logger : ham_impl_default_logger;
}

#ifdef __GNUC__
__attribute__((format(printf, 3, 4)))
#endif
static inline void ham_logf(ham_log_level level, const char *api, const char *fmt_str, ...){
	ham_timepoint log_tp = (ham_timepoint){ 0, 0 };

	if(!ham_timepoint_now(&log_tp, CLOCK_REALTIME)){
		// bad place for this to happed :/
	}

	va_list va0;
	va_start(va0, fmt_str);
	const int len = vsnprintf(ham_impl_message_buf, HAM_MESSAGE_BUFFER_SIZE-1, fmt_str, va0);
	va_end(va0);

	if(len < 0){
		// wowza
		return;
	}

	const ham_logger *logger = ham_current_logger();

	logger->log(log_tp, level, api, ham_impl_message_buf, logger->user);
}

#define ham_logapif(level, fmt_str, ...) (ham_logf(level, __FUNCTION__, fmt_str __VA_OPT__(,) __VA_ARGS__))

#define ham_loginfof(api, fmt_str, ...)    (ham_logf(HAM_LOG_INFO,    api, fmt_str __VA_OPT__(,) __VA_ARGS__))
#define ham_logverbosef(api, fmt_str, ...) (ham_logf(HAM_LOG_VERBOSE, api, fmt_str __VA_OPT__(,) __VA_ARGS__))
#define ham_logdebugf(api, fmt_str, ...)   (ham_logf(HAM_LOG_DEBUG,   api, fmt_str __VA_OPT__(,) __VA_ARGS__))
#define ham_logwarnf(api, fmt_str, ...)    (ham_logf(HAM_LOG_WARNING, api, fmt_str __VA_OPT__(,) __VA_ARGS__))
#define ham_logerrorf(api, fmt_str, ...)   (ham_logf(HAM_LOG_ERROR,   api, fmt_str __VA_OPT__(,) __VA_ARGS__))
#define ham_logfatalf(api, fmt_str, ...)   (ham_logf(HAM_LOG_FATAL,   api, fmt_str __VA_OPT__(,) __VA_ARGS__))

#define ham_logapiinfof(fmt_str, ...)    ham_logapif(HAM_LOG_INFO,    fmt_str __VA_OPT__(,) __VA_ARGS__)
#define ham_logapiverbosef(fmt_str, ...) ham_logapif(HAM_LOG_VERBOSE, fmt_str __VA_OPT__(,) __VA_ARGS__)
#define ham_logapidebugf(fmt_str, ...)   ham_logapif(HAM_LOG_DEBUG,   fmt_str __VA_OPT__(,) __VA_ARGS__)
#define ham_logapiwarnf(fmt_str, ...)    ham_logapif(HAM_LOG_WARNING, fmt_str __VA_OPT__(,) __VA_ARGS__)
#define ham_logapierrorf(fmt_str, ...)   ham_logapif(HAM_LOG_ERROR,   fmt_str __VA_OPT__(,) __VA_ARGS__)
#define ham_logapifatalf(fmt_str, ...)   ham_logapif(HAM_LOG_FATAL,   fmt_str __VA_OPT__(,) __VA_ARGS__)

// TODO: implement this correctly
//#define ham_logcall(api, fn_name, ...) (ham_logverbosef(api, "%s" HAM_REPEAT(HAM_NARGS(__VA_ARGS__), " %s")), (fn_name)(__VA_ARGS__))

HAM_C_API_END

/**
 * @}
 */

#endif // !HAM_LOG_H
