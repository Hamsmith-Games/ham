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

#define HAM_ENGINE_CLIENT_API_NAME "ham-engine-client"

#include "ham/dll.h"
#include "ham/log.h"
#include "ham/engine-vtable.h"

#include "SDL.h"

struct ham_engine_client_context{
	ham_derive(ham_engine_context)
};

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

static inline bool ham_engine_client_init(ham_engine_client_context *ctx){
	(void)ctx;
	return true;
}

static inline void ham_engine_client_finish(ham_engine_client_context *ctx){
	(void)ctx;
}

HAM_ENGINE_VTABLE(
	ham_engine_client_context,
	HAM_ENGINE_CLIENT_PLUGIN_UUID,
	"ham-engine-client",
	HAM_VERSION,
	"Ham World Engine Client",
	"Hamsmith Ltd.",
	"GPLv3+",
	"Ham World Engine Client",
	ham_engine_client_on_load,
	ham_engine_client_on_unload,
	ham_engine_client_init,
	ham_engine_client_finish
)

int main(int argc, char *argv[]){
	const auto engine = ham_engine_create("ham-engine-client", argc, argv);

	return ham_engine_exec(engine);
}
