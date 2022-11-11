/*
 * Ham World Engine Server Manager
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

#ifndef HAM_ENGINE_SERVER_MANAGER_H
#define HAM_ENGINE_SERVER_MANAGER_H 1

#include "ham/str_buffer.h"
#include "ham/fs.h"
#include "ham/engine.h"

#ifdef HAM_ENGINE_SERVER_MANAGER_IMPLEMENTATION
#	define ham_engine_server_manager_api ham_public ham_export
#else
#	define ham_engine_server_manager_api ham_public ham_import
#endif

HAM_C_API_BEGIN

typedef struct ham_engine_server_manager ham_engine_server_manager;

ham_engine_server_manager_api ham_engine_server_manager *ham_server_manager_create(const char *server_exec_path, int server_argc, char **server_argv);
ham_engine_server_manager_api void ham_server_manager_destroy(ham_engine_server_manager *manager);

ham_engine_server_manager_api int ham_server_manager_exec(ham_engine_server_manager *manager);

ham_engine_server_manager_api bool ham_server_manager_redraw_border(ham_engine_server_manager *manager);

ham_engine_server_manager_api bool ham_server_manager_redraw(ham_engine_server_manager *manager);

ham_engine_server_manager_api bool ham_server_manager_exit(ham_engine_server_manager *manager);

ham_engine_server_manager_api bool ham_server_manager_shutdown(ham_engine_server_manager *manager);

HAM_C_API_END

#endif // !HAM_ENGINE_SERVER_MANAGER_H
