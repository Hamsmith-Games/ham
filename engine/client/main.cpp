/*
 * The Ham World Engine Client
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

#define HAM_ENGINE_CLIENT_API_NAME "ham-engine-client"

#include "ham/engine.h"
#include "ham/log.h"
#include "ham/renderer.h"
#include "ham/net.h"
#include "ham/plugin.h"
#include "ham/fs.h"

#include "ham/engine/argparse.h"

#include "SDL.h"
#include "SDL_vulkan.h"

#include <vulkan/vulkan_core.h>

#include "client.h"

using namespace ham::typedefs;

static inline bool ham_engine_client_init(ham_engine *engine, void *user){
	(void)engine; (void)user;
	return true;
}

static inline void ham_engine_client_fini(ham_engine *engine, void *user){
	(void)engine; (void)user;
}

static inline void ham_engine_client_loop(ham_engine *engine, ham_f64 dt, void *user){
	(void)user;

	SDL_Event ev;
	while(SDL_PollEvent(&ev)){
		if(ev.type == SDL_QUIT){
			ham_engine_request_exit(engine);
		}
	}
}

bool ham_engine_client_show_message(ham_log_level level, const char *msg){
	ham::log(static_cast<ham::log_level>(level), HAM_ENGINE_CLIENT_API_NAME, "{}", msg);

	SDL_MessageBoxData data;

	SDL_MessageBoxButtonData btns[] = {
		{ SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 0, "Ok" }
	};

	SDL_MessageBoxColorScheme color_scheme;

	color_scheme.colors[SDL_MESSAGEBOX_COLOR_BACKGROUND]        = SDL_MessageBoxColor{ 0x69, 0x69, 0x69 };
	color_scheme.colors[SDL_MESSAGEBOX_COLOR_TEXT]              = SDL_MessageBoxColor{ 0xff, 0xff, 0xff };
	color_scheme.colors[SDL_MESSAGEBOX_COLOR_BUTTON_BORDER]     = SDL_MessageBoxColor{ 0x42, 0x42, 0x42 };
	color_scheme.colors[SDL_MESSAGEBOX_COLOR_BUTTON_BACKGROUND] = SDL_MessageBoxColor{ 0x42, 0x42, 0x42 };
	color_scheme.colors[SDL_MESSAGEBOX_COLOR_BUTTON_SELECTED]   = SDL_MessageBoxColor{ 0xff, 0x78, 0x00 };

	const auto title_str = ham::format("Ham Engine - {}", ham_log_level_str(level));

	data.flags = SDL_MESSAGEBOX_ERROR;
	data.window = nullptr;
	data.title = title_str.c_str();
	data.message = msg;
	data.numbuttons = 1;
	data.buttons = btns;
	data.colorScheme = &color_scheme;

	int btn_id = 0; // will always be 0

	const int res = SDL_ShowMessageBox(&data, &btn_id);
	if(res != 0){
		ham::logerror(HAM_ENGINE_CLIENT_API_NAME, "Error in SDL_ShowMessageBox: {}", SDL_GetError());
		return false;
	}

	return true;
}

int main(int argc, char *argv[]){
	ham_engine_args args;
	if(!ham_engine_args_parse(&args, argc, argv)){
		ham::logerror(HAM_ENGINE_CLIENT_API_NAME, "Error in ham_engine_args_parse");
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
		ham::logerror(HAM_ENGINE_CLIENT_API_NAME, "Could not find 'ham.json' in app directory: {}", args.app_dir);
		return -1;
	}

	const auto app_info_json = ham::json_document::open(app_info_json_path);

	ham_engine_app app_info;
	if(!ham_engine_app_load_json(&app_info, app_info_json.root())){
		ham::logerror(HAM_ENGINE_CLIENT_API_NAME, "Error in ham_engine_app_load_json");
		return -1;
	}

	app_info.init = ham_engine_client_init;
	app_info.fini = ham_engine_client_fini;
	app_info.loop = ham_engine_client_loop;
	app_info.user = nullptr;

	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC) != 0){
		ham::logerror(HAM_ENGINE_CLIENT_API_NAME, "Error in SDL_Init: {}", SDL_GetError());
		return -2;
	}

	std::atexit(SDL_Quit);

	bool has_vulkan = true, has_gl = true;

	if(SDL_Vulkan_LoadLibrary(nullptr) != 0){
		ham::logwarn(HAM_ENGINE_CLIENT_API_NAME, "Error in SDL_Vulkan_LoadLibrary: {}", SDL_GetError());
		has_vulkan = false;
	}

	if(SDL_GL_LoadLibrary(nullptr) != 0){
		ham::logwarn(HAM_ENGINE_CLIENT_API_NAME, "Error in SDL_GL_LoadLibrary: {}", SDL_GetError());
		has_gl = false;
	}

	if(!has_gl && !has_vulkan){
		if(!ham_engine_client_show_message(HAM_LOG_ERROR, "Could not find Vulkan or OpenGL library")){
			ham::logerror(HAM_ENGINE_CLIENT_API_NAME, "Error in ham_engine_client_show_message: 4LL 1S L05T");
			return -69;
		}

		return -3;
	}

	// TODO: add 'ham-renderer-gl'
	if(!has_vulkan){
		if(!ham_engine_client_show_message(HAM_LOG_ERROR, "Could not find Vulkan library")){
			ham::logerror(HAM_ENGINE_CLIENT_API_NAME, "Error in ham_engine_client_show_message: 4LL 1S L05T");
			return -69;
		}

		return -3;
	}

	const auto engine = ham_engine_create(&app_info, nullptr);
	if(!engine){
		if(!ham_engine_client_show_message(HAM_LOG_ERROR, "Error in ham_engine_create")){
			ham::logerror(HAM_ENGINE_CLIENT_API_NAME, "Error in ham_engine_client_show_message: 4LL 1S L05T");
			return -69;
		}

		return -4;
	}

	ham::engine::client_net_subsystem net_subsys(engine);
	ham::engine::client_video_subsystem vid_subsys(engine);

	return ham_engine_main(engine);
}
