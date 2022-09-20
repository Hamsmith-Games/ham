#include "ham/log.h"

HAM_C_API_BEGIN

static inline ham_usize ham_impl_default_log_fn(
	ham_timepoint tp,
	ham_log_level level,
	const char *api,
	const char *msg,
	void *user
){
	(void)user;

	FILE *out_file = (level == HAM_LOG_INFO || level == HAM_LOG_VERBOSE) ? stdout : stderr;

	struct tm tm_val;
	struct tm *const time_info = localtime_r(&tp.tv_sec, &tm_val);

	const long ret = fprintf(
		out_file,
		"[%02d/%02d/%d %02d:%02d:%02d.%03ld] [%s] (%s) %s\n",
		time_info->tm_mday, time_info->tm_mon, time_info->tm_year - 100,
		time_info->tm_hour, time_info->tm_min, time_info->tm_sec, tp.tv_nsec / 1000000,
		ham_log_level_str(level),
		api, msg
	);

	if(out_file == stdout) fflush(stdout);

	return (ham_usize)ret;
}

static const ham_logger ham_impl_default_logger_inst{
	.log = ham_impl_default_log_fn,
	.user = nullptr,
};

const ham_logger *const ham_impl_default_logger = &ham_impl_default_logger_inst;
const ham_logger *ham_impl_global_logger = &ham_impl_default_logger_inst;
ham_thread_local ham_message_buffer ham_impl_message_buf = { 0 };
ham_thread_local const ham_logger *ham_impl_thread_logger = ham_null;
ham_thread_local const ham_logger *const *ham_impl_current_logger = &ham_impl_global_logger;

HAM_C_API_END
