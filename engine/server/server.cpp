/*
 * Ham World Engine Server
 * Copyright (C) 2022 Hamsmith Ltd.
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

#define HAM_ENGINE_SERVER_API_NAME "ham-engine-server"
#define HAM_ENGINE_SERVER_OBJ_NAME "ham_engine_server"

#include "ham/engine-object.h"
#include "ham/engine/argparse.h"

#include "ham/log.h"
#include "ham/fs.h"
#include "ham/net.h"

static inline bool ham_engine_server_init(ham_engine *engine, void*){
	(void)engine;
	return true;
}

static inline void ham_engine_server_fini(ham_engine *engine, void*){
	(void)engine;
}

static inline void ham_engine_server_loop(ham_engine *engine, ham_f64 dt, void*){
	(void)engine; (void)dt;
}

int main(int argc, char *argv[]){
	ham_engine_args args;
	if(!ham_engine_args_parse(&args, argc, argv)){
		ham::logerror(HAM_ENGINE_SERVER_API_NAME, "Error in ham_engine_args_parse");
		return -1;
	}

	if(args.show_help){
		fmt::print("{}", ham_engine_args_help_str(argv[0]));
		return 0;
	}
	else if(args.show_version){
		fmt::print("{}", ham_engine_args_version_str(false));
		return 0;
	}
	else if(args.verbose){
		ham_log_set_verbose(true);
	}

	const auto app_info_json_path = ham::format("{}/ham.json", args.app_dir);
	if(!ham::path_exists(app_info_json_path)){
		ham::logerror(HAM_ENGINE_SERVER_API_NAME, "Could not find 'ham.json' in app directory: {}", args.app_dir);
		return -1;
	}

	const auto app_info_json = ham::json_document::open(app_info_json_path);

	ham_engine_app app_info;
	if(!ham_engine_app_load_json(&app_info, app_info_json.root())){
		ham::logerror(HAM_ENGINE_SERVER_API_NAME, "Error in ham_engine_app_load_json");
		return -1;
	}

	app_info.init = ham_engine_server_init;
	app_info.fini = ham_engine_server_fini;
	app_info.loop = ham_engine_server_loop;
	app_info.user = nullptr;

	const auto engine = ham_engine_create2(&app_info);
	if(!engine){
		ham::logerror(HAM_ENGINE_SERVER_API_NAME, "Error in ham_engine_create");
		return -4;
	}

	static const auto sig_handler = +[](int){ ham_engine_request_exit(ham_gengine()); }; // this will be U.B. in a signal handler :/

	struct sigaction exit_sigaction{};
	exit_sigaction.sa_handler = sig_handler;

	sigaction(SIGTERM, &exit_sigaction, nullptr);
	sigaction(SIGINT,  &exit_sigaction, nullptr);

	return ham_engine_main(engine);
}
