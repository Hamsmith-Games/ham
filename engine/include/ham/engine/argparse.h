/*
 * Ham World Engine Runtime
 * Copyright (C) 2022 Keith Hammond
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

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
