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

#include "ham/engine/argparse.h"

#include "ham/check.h"

#include <getopt.h>

using namespace ham::typedefs;

#define HAM_ENGINE_VERSION_LINE "Ham World Engine - " HAM_ENGINE_BUILD_TYPE_STR " - v" HAM_ENGINE_VERSION_STR "\n"

#define HAM_ENGINE_VERSION_LINE_FANCY \
	"┌──┐\n" \
	"│██│    ┌──┐\n" \
	"│██│    │██│ ┌──────┐   ┌───┐  ┌───┐\n" \
	"│██└────┘██├─┘██████├──┬┘███└──┘███└┐\n" \
	"│██████████│██┌─────┤██│████████████│\n" \
	"│██┌────┐██│██│     │██│██┌─┐██┌─┐██│\n" \
	"│██│    │██│██└─────┘██│██│ └──┘ │██│\n" \
	"│██│    │██├─┐█████████│██│      │██│\n" \
	"└──┘    └──┘ └─────────┴──┘      └──┘ World Engine - " HAM_ENGINE_BUILD_TYPE_STR " - v" HAM_ENGINE_VERSION_STR "\n"

HAM_C_API_BEGIN

const char *ham_engine_args_version_str(bool fancy){
	return fancy ? HAM_ENGINE_VERSION_LINE : HAM_ENGINE_VERSION_LINE_FANCY;
}

const char *ham_engine_args_help_str(const char *argv0){
	static thread_local ham_message_buffer_utf8 msg_buf;

	const char *exec_name = argv0;

	const auto argv0_str = ham::str8(argv0);
	const auto new_beg = argv0_str.rfind("/");
	if(new_beg != ham::str8::npos){
		exec_name += new_beg;
	}

	ham::format_buffered(
		sizeof(msg_buf), msg_buf,
		"{}"
		"\n"
		"    Usage: {} [OPTIONS...]\n"
		"\n"
		"    Possible options:\n"
		"        -h|--help:             Print this help message and exit\n"
		"        -v|--version:          Print the version string and exit\n"
		"        -V|--verbose:          Print more log messages\n"
		"        -a|--app-dir DIR:      Set the game directory\n"
		"\n",
		ham_engine_args_version_str(true),
		exec_name
	);

	return msg_buf;
}

bool ham_engine_args_parse(ham_engine_args *ret, int argc, char *const *argv){
	if(
		!ham_check(ret != NULL) ||
		!ham_check(argc > 0) ||
		!ham_check(argv != NULL)
	){
		return false;
	}

	int show_version = 0, show_help = 0, verbose = 0;
	const char *app_dir = nullptr;

	struct option options[] = {
		{ "help",    no_argument,       &show_help,    1 },
		{ "version", no_argument,       &show_version, 1 },
		{ "verbose", no_argument,       &verbose,      1 },
		{ "app-dir", required_argument, nullptr,       0 },

		{ nullptr, no_argument, nullptr, 0 }
	};

	while(1){
		int option_idx = 0;
		const int res = getopt_long(argc, argv, "hvVa:", options, &option_idx);
		if(res == -1) break;

		switch(res){
			// long option
			case 0:{
				switch(option_idx){
					case 0: show_help    = 1; break;
					case 1: show_version = 1; break;
					case 2: verbose      = 1; break;
					case 3: app_dir = optarg; break;

					default:{
						ham::logapierror("Error in getopt_long: unexpected long option at index {}\n", option_idx);
						return false;
					}
				}

				break;
			}

			case 'h': show_help    = 1; break;
			case 'v': show_version = 1; break;
			case 'V': verbose      = 1; break;
			case 'a': app_dir = optarg; break;

			case '?':{
				ham::logapierror("Unexpected argument: {}\n", optarg);
				return false;
			}

			default:{
				ham::logapierror("Error in getopt_long: returned character 0x%o\n", res);
				return false;
			}
		}
	}

	if(!app_dir){
		static ham_path_buffer_utf8 cwd_buf;
		app_dir = getcwd(cwd_buf, sizeof(cwd_buf)-1);
	}

	ret->show_help = show_help != 0;
	ret->show_version = show_version != 0;
	ret->verbose = verbose != 0;
	ret->app_dir = app_dir;

	return true;
}

HAM_C_API_END
