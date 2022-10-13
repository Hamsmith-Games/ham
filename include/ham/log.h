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
ham_api extern bool ham_impl_verbose_flag;
ham_api extern const ham_logger *const ham_impl_default_logger;
ham_api extern const ham_logger *ham_impl_global_logger;
ham_api extern ham_thread_local ham_message_buffer ham_impl_message_buf;
ham_api extern ham_thread_local const ham_logger *ham_impl_thread_logger;
ham_api extern ham_thread_local const ham_logger *const *ham_impl_current_logger;
//! @endcond

ham_nothrow static inline bool ham_log_is_verbose(){
	return ham_impl_verbose_flag;
}

ham_nothrow static inline void ham_log_set_verbose(bool enabled){
	ham_impl_verbose_flag = enabled;
}

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
		// bad place for this to happen :/
	}

	if(level == HAM_LOG_VERBOSE && !ham_log_is_verbose()){
		return;
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

#ifdef HAM_DEBUG
	if(level > HAM_LOG_WARNING){
		ham_breakpoint();
	}
#endif
}

#define ham_logapif(level, fmt_str, ...) (ham_logf(level, __FUNCTION__, fmt_str __VA_OPT__(,) __VA_ARGS__))

#define ham_loginfof(api, fmt_str, ...)    (ham_logf(HAM_LOG_INFO,    api, fmt_str __VA_OPT__(,) __VA_ARGS__))
#define ham_logverbosef(api, fmt_str, ...) (ham_logf(HAM_LOG_VERBOSE, api, fmt_str __VA_OPT__(,) __VA_ARGS__))

#ifdef HAM_DEBUG
#	define ham_logdebugf(api, fmt_str, ...) (ham_logf(HAM_LOG_DEBUG,   api, fmt_str __VA_OPT__(,) __VA_ARGS__))
#else
#	define ham_logdebugf(api, fmt_str, ...)
#endif

#define ham_logwarnf(api, fmt_str, ...)    (ham_logf(HAM_LOG_WARNING, api, fmt_str __VA_OPT__(,) __VA_ARGS__))
#define ham_logerrorf(api, fmt_str, ...)   (ham_logf(HAM_LOG_ERROR,   api, fmt_str __VA_OPT__(,) __VA_ARGS__))
#define ham_logfatalf(api, fmt_str, ...)   (ham_logf(HAM_LOG_FATAL,   api, fmt_str __VA_OPT__(,) __VA_ARGS__))

#define ham_logapiinfof(fmt_str, ...)    ham_logapif(HAM_LOG_INFO,    fmt_str __VA_OPT__(,) __VA_ARGS__)
#define ham_logapiverbosef(fmt_str, ...) ham_logapif(HAM_LOG_VERBOSE, fmt_str __VA_OPT__(,) __VA_ARGS__)

#ifdef HAM_DEBUG
#	define ham_logapidebugf(fmt_str, ...) ham_logapif(HAM_LOG_DEBUG,   fmt_str __VA_OPT__(,) __VA_ARGS__)
#else
#	define ham_logapidebugf(fmt_str, ...)
#endif

#define ham_logapiwarnf(fmt_str, ...)    ham_logapif(HAM_LOG_WARNING, fmt_str __VA_OPT__(,) __VA_ARGS__)
#define ham_logapierrorf(fmt_str, ...)   ham_logapif(HAM_LOG_ERROR,   fmt_str __VA_OPT__(,) __VA_ARGS__)
#define ham_logapifatalf(fmt_str, ...)   ham_logapif(HAM_LOG_FATAL,   fmt_str __VA_OPT__(,) __VA_ARGS__)

// TODO: implement this correctly
//#define ham_logcall(api, fn_name, ...) (ham_logverbosef(api, "%s" HAM_REPEAT(HAM_NARGS(__VA_ARGS__), " %s")), (fn_name)(__VA_ARGS__))

HAM_C_API_END

#ifdef __cplusplus

#include "str_buffer.h"

#if defined(__clang__) && !(__has_builtin(__builtin_source_location))
#	include <experimental/source_location>
	namespace ham::detail{ using source_location = std::experimental::source_location; };
#elif defined(__GNUC__) || defined(_MSVC_VER)
#	include <source_location>
	namespace ham::detail{ using source_location = std::source_location; };
#else
#	error "Unsupported compiler"
#endif

namespace ham{
	enum class log_level{
		info = HAM_LOG_INFO,
		verbose = HAM_LOG_VERBOSE,
		debug = HAM_LOG_DEBUG,
		warning = HAM_LOG_WARNING,
		error = HAM_LOG_ERROR,
		fatal = HAM_LOG_FATAL,
	};

	template<typename ... Args>
	static inline void log(log_level level, const char8 *api, const fmt::format_string<Args...> &fmt_str, Args &&... args) noexcept{
		ham_timepoint log_tp = (ham_timepoint){ 0, 0 };

		if(!ham_timepoint_now(&log_tp, CLOCK_REALTIME)){
			// bad place for this to happen :/
		}

		if(level == log_level::verbose && !ham_log_is_verbose()){
			return;
		}

		format_buffered(sizeof(ham_impl_message_buf), ham_impl_message_buf, fmt_str, std::forward<Args>(args)...);

		const ham_logger *logger = ham_current_logger();

		logger->log(log_tp, static_cast<ham_log_level>(level), api, ham_impl_message_buf, logger->user);

	#ifdef HAM_DEBUG
		if(static_cast<int>(level) > HAM_LOG_WARNING){
			ham_breakpoint();
		}
	#endif
	}

	template<typename ... Args>
	static inline void loginfo(const char *api, const fmt::format_string<Args...> &fmt_str, Args &&... args) noexcept{
		log(log_level::info, api, fmt_str, std::forward<Args>(args)...);
	}

	template<typename ... Args>
	static inline void logverbose(const char *api, const fmt::format_string<Args...> &fmt_str, Args &&... args) noexcept{
		log(log_level::verbose, api, fmt_str, std::forward<Args>(args)...);
	}

	template<typename ... Args>
	static inline void logdebug(const char *api, const fmt::format_string<Args...> &fmt_str, Args &&... args) noexcept{
		log(log_level::debug, api, fmt_str, std::forward<Args>(args)...);
	}

	template<typename ... Args>
	static inline void logwarn(const char *api, const fmt::format_string<Args...> &fmt_str, Args &&... args) noexcept{
		log(log_level::warning, api, fmt_str, std::forward<Args>(args)...);
	}

	template<typename ... Args>
	static inline void logerror(const char *api, const fmt::format_string<Args...> &fmt_str, Args &&... args) noexcept{
		log(log_level::error, api, fmt_str, std::forward<Args>(args)...);
	}

	template<typename ... Args>
	static inline void logfatal(const char *api, const fmt::format_string<Args...> &fmt_str, Args &&... args) noexcept{
		log(log_level::fatal, api, fmt_str, std::forward<Args>(args)...);
	}

	//
	// API logging (api == function name)
	//

	struct logapi_format_string{
		public:
			template<
				typename S,
				std::enable_if_t<
					std::is_convertible_v<const S&, fmt::string_view>,
					int
				> = 0
			>
			consteval logapi_format_string(
				const S& s,
				const detail::source_location loc_ = detail::source_location::current()
			)
				: m_fmt_str(s)
				, m_loc(loc_){}

			constexpr const fmt::string_view &fmt_str() const noexcept{ return m_fmt_str; }
			constexpr const detail::source_location &loc() const noexcept{ return m_loc; }

			constexpr operator fmt::string_view() const noexcept{ return m_fmt_str; }

		private:
			fmt::string_view m_fmt_str;
			detail::source_location m_loc;
	};

	template<typename ... Args>
	static inline void logapi(log_level level, const logapi_format_string &fmt_str, Args &&... args) noexcept{
		ham_timepoint log_tp = (ham_timepoint){ 0, 0 };

		if(!ham_timepoint_now(&log_tp, CLOCK_REALTIME)){
			// bad place for this to happen :/
		}

		fmt::detail::check_format_string<Args...>(fmt_str.fmt_str());

		if(level == log_level::verbose && !ham_log_is_verbose()){
			return;
		}

		// TODO: make this work without vformat
		const auto fmt_res = fmt::vformat_to_n(
			ham_impl_message_buf, sizeof(ham_impl_message_buf)-1,
			fmt_str, fmt::make_format_args(std::forward<Args>(args)...)
		);

		*fmt_res.out = '\0';

		const ham_logger *logger = ham_current_logger();

		logger->log(log_tp, static_cast<ham_log_level>(level), fmt_str.loc().function_name(), ham_impl_message_buf, logger->user);

	#ifdef HAM_DEBUG
		if(static_cast<int>(level) > HAM_LOG_WARNING){
			ham_breakpoint();
		}
	#endif
	}

	template<typename ... Args>
	static inline void logapiinfo(const logapi_format_string &fmt_str, Args &&... args) noexcept{
		logapi(log_level::info, fmt_str, std::forward<Args>(args)...);
	}

	template<typename ... Args>
	static inline void logapiverbose(const logapi_format_string &fmt_str, Args &&... args) noexcept{
		logapi(log_level::verbose, fmt_str, std::forward<Args>(args)...);
	}

	template<typename ... Args>
	static inline void logapidebug(const logapi_format_string &fmt_str, Args &&... args) noexcept{
		logapi(log_level::debug, fmt_str, std::forward<Args>(args)...);
	}

	template<typename ... Args>
	static inline void logapiwarn(const logapi_format_string &fmt_str, Args &&... args) noexcept{
		logapi(log_level::warning, fmt_str, std::forward<Args>(args)...);
	}

	template<typename ... Args>
	static inline void logapierror(const logapi_format_string &fmt_str, Args &&... args) noexcept{
		logapi(log_level::error, fmt_str, std::forward<Args>(args)...);
	}

	template<typename ... Args>
	static inline void logapifatal(const logapi_format_string &fmt_str, Args &&... args) noexcept{
		logapi(log_level::fatal, fmt_str, std::forward<Args>(args)...);
	}
}

#endif // __cplusplus

/**
 * @}
 */

#endif // !HAM_LOG_H
