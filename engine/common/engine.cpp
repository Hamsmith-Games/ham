/**
 * The Ham World Engine
 * Copyright (C) 2022  Hamsmith Ltd.
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

#include "ham/engine.h"
#include "ham/log.h"
#include "ham/fs.h"

#include <unistd.h>
#include <getopt.h>

HAM_C_API_BEGIN

[[noreturn]]
static inline void ham_impl_display_version(){
	printf("Ham World Engine v" HAM_VERSION_STR "\n");
	exit(EXIT_SUCCESS);
}

[[noreturn]]
static void ham_impl_print_help(const char *argv0){
	printf(
		"Ham World Engine v" HAM_VERSION_STR "\n"
		"\n"
		"    Usage: %s [OPTIONS]\n"
		"\n"
		"    Possible options:\n"
		"        -h|--help:          Print this help message\n"
		"        -g|--game-dir DIR:  Set the game/app directory\n"
		"\n"
		,
		argv0
	);
	exit(EXIT_SUCCESS);
}

static struct option ham_impl_long_options[] = {
	{ "help",		no_argument,		0, 0 },
	{ "game-dir",	required_argument,	0, 0 },
	{ 0,            0,                  0, 0 }
};

void ham_engine_init(int argc, char **argv){
	(void)argc; (void)argv;

	ham_path_buffer path_buf;

	const char *game_dir = getcwd(path_buf, HAM_PATH_BUFFER_SIZE);

	while(1){
		int option_index = 0;
		const int getopt_res = getopt_long(argc, argv, "hg:", ham_impl_long_options, &option_index);

		if(getopt_res == -1){
			break;
		}

		switch(getopt_res){
			case 0:{
				if(option_index == 0){
					ham_impl_print_help(argv[0]);
					exit(0);
				}
				else if(option_index == 1){
					game_dir = optarg;
				}
				else{
					fprintf(stderr, "Error in getopt_long: unexpected long option at index %d\n", option_index);
					exit(1);
				}

				break;
			}

			case 'h': ham_impl_print_help(argv[0]);

			case 'g':{
				game_dir = optarg;
				break;
			}

			case '?':{
				fprintf(stderr, "Unexpected argument: %s\n", optarg);
				exit(1);
			}

			default:{
				fprintf(stderr, "Error in getopt_long: returned character 0x%o\n", getopt_res);
				exit(1);
			}
		}
	}

	if(optind < argc){
		fprintf(stderr, "Unexpected arguments:");
		while(optind < argc){
			fprintf(stderr, " %s", argv[optind++]);
		}
		fprintf(stderr, "\n");
	}

	const ham_str8 game_path_str = { game_dir, strlen(game_dir) };

	if(!ham_path_exists_utf8(game_path_str)){
		fprintf(stderr, "Game directory does not exist: %s\n", game_dir);
		exit(1);
	}

	ham_logapiinfof("Game directory: %s", game_dir);
}

int ham_exec(){ return 0; }

HAM_C_API_END
