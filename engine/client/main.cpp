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

#include "ham/log.h"
#include "ham/engine.h"

#include "SDL.h"

int main(int argc, char *argv[]){
	ham_engine_init(argc, argv);

	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC) != 0){
		ham_logerrorf(HAM_ENGINE_CLIENT_API_NAME, "Error in SDL_Init: %s", SDL_GetError());
		return 1;
	}

	atexit(SDL_Quit);

	return ham_exec();
}
