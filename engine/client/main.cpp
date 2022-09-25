/*
 * The Ham World Engine Client
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

#define HAM_ENGINE_CLIENT_API_NAME "ham-engine-client"
#define HAM_ENGINE_CLIENT_OBJ_NAME "ham_engine_client"

#include "ham/engine-vtable.h"
#include "ham/plugin.h"
#include "ham/log.h"

#include "SDL.h"

static inline bool ham_engine_client_on_load(){
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC) != 0){
		ham_logerrorf(HAM_ENGINE_CLIENT_API_NAME, "Error in SDL_Init: %s", SDL_GetError());
		return false;
	}

	return true;
}

static inline void ham_engine_client_on_unload(){
	SDL_Quit();
}

HAM_PLUGIN(
	ham_engine_client,
	HAM_ENGINE_CLIENT_PLUGIN_UUID,
	HAM_ENGINE_CLIENT_API_NAME,
	HAM_VERSION,
	"Ham World Engine Client",
	"Hamsmith Ltd.",
	"GPLv3+",
	HAM_ENGINE_PLUGIN_CATEGORY,
	"Ham World Engine Client",
	ham_engine_client_on_load,
	ham_engine_client_on_unload
)

struct ham_engine_client{
	ham_derive(ham_engine)

	SDL_Window *window;
};

static inline ham_engine_client *ham_engine_client_construct(ham_engine_client *mem, va_list va){
	(void)va;
	return new(mem) ham_engine_client;
}

static inline void ham_engine_client_destroy(ham_engine_client *engine){
	std::destroy_at(engine);
}

static inline bool ham_engine_client_init(ham_engine *engine_base){
	const auto engine = (ham_engine_client*)engine_base;

	engine->window = SDL_CreateWindow(
		HAM_ENGINE_CLIENT_API_NAME,
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		1024, 768,
		SDL_WINDOW_VULKAN | SDL_WINDOW_SHOWN
	);
	if(!engine->window){
		ham_logerrorf(HAM_ENGINE_CLIENT_API_NAME, "Error in SDL_CreateWindow: %s", SDL_GetError());
		return false;
	}

	ham_logapiverbosef("Created window 1024x768");

	// TODO: get a VkInstance and create renderer

	return true;
}

static inline void ham_engine_client_fini(ham_engine *engine_base){
	const auto engine = (ham_engine_client*)engine_base;

	SDL_DestroyWindow(engine->window);
}

static inline void ham_engine_client_loop(ham_engine *engine, ham_f64 dt){
	(void)engine; (void)dt;

	SDL_Event ev;
	while(SDL_PollEvent(&ev)){
		if(ev.type == SDL_QUIT){
			ham_engine_request_exit(engine);
		}
	}
}

ham_define_object_x(
	2, ham_engine_client,
	1, ham_engine_vtable,
	ham_engine_client_construct,
	ham_engine_client_destroy,
	(
		.init = ham_engine_client_init,
		.fini = ham_engine_client_fini,
		.loop = ham_engine_client_loop,
	)
)

int main(int argc, char *argv[]){
	const auto engine = ham_engine_create(HAM_ENGINE_CLIENT_API_NAME, HAM_ENGINE_CLIENT_OBJ_NAME, argc, argv);
	return ham_engine_exec(engine);
}
