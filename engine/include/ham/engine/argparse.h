#ifndef HAM_ENGINE_ARGPARSE_H
#define HAM_ENGINE_ARGPARSE_H 1

/**
 * @defgroup HAM_ENGINE_ARGPARSE Engine argument parsing
 * @ingroup HAM_ENGINE
 * @{
 */

#include "../engine.h"

HAM_C_API_BEGIN

typedef struct ham_engine_args{
	bool show_help;
	bool show_version;
	bool verbose;
	const char *app_dir;
} ham_engine_args;

ham_engine_api const char *ham_engine_args_version_str(bool fancy);

ham_engine_api const char *ham_engine_args_help_str(const char *argv0);

ham_engine_api bool ham_engine_args_parse(ham_engine_args *ret, int argc, char *const *argv);

HAM_C_API_END

/**
 * @}
 */

#endif // !HAM_ENGINE_ARGPARSE_H
