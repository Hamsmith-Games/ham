/*
 * Ham World Engine Server
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

#define HAM_ENGINE_SERVER_API_NAME "ham-engine-server"
#define HAM_ENGINE_SERVER_OBJ_NAME "ham_engine_server"

#include "ham/engine-object.h"
#include "ham/log.h"

#include "ham/net.h"

static ham_engine *ham_gengine = nullptr;

ham_extern_c ham_public ham_export ham_nothrow ham_u32 ham_net_steam_appid(){ return 480; }

static inline bool ham_engine_server_on_load(){ return true; }
static inline void ham_engine_server_on_unload(){}

HAM_PLUGIN(
	ham_engine_server,
	HAM_ENGINE_SERVER_PLUGIN_UUID,
	HAM_ENGINE_SERVER_API_NAME,
	HAM_VERSION,
	"Ham World Engine Server",
	"Hamsmith Ltd.",
	"GPLv3+",
	HAM_ENGINE_PLUGIN_CATEGORY,
	"Ham World Engine Server",
	ham_engine_server_on_load,
	ham_engine_server_on_unload
)

struct ham_engine_server{
	ham_derive(ham_engine)

	ham_net *net;
};

static inline ham_engine_server *ham_engine_server_construct(ham_engine_server *mem, ham_usize nargs, va_list va){
	(void)nargs; (void)va;
	return new(mem) ham_engine_server;
}

static inline void ham_engine_server_destroy(ham_engine_server *ptr){
	std::destroy_at(ptr);
}

static inline bool ham_engine_server_init(ham_engine *engine_base){
	const auto engine = (ham_engine_server*)engine_base;

	engine->net = ham_net_create(HAM_NET_STEAMWORKS_PLUGIN_NAME, HAM_NET_STEAMWORKS_OBJECT_NAME);
	if(!engine->net){
		ham_logapierrorf("Error in ham_net_create");
		return false;
	}

	return true;
}

static inline void ham_engine_server_fini(ham_engine *engine_base){
	const auto engine = (ham_engine_server*)engine_base;

	ham_net_destroy(engine->net);
}

static inline void ham_engine_server_loop(ham_engine *engine_base, ham_f64 dt){
	const auto engine = (ham_engine_server*)engine_base;
	ham_net_loop(engine->net, dt);
}

ham_define_object_x(
	2, ham_engine_server,
	1, ham_engine_vtable,
	ham_engine_server_construct,
	ham_engine_server_destroy,
	(
		.init = ham_engine_server_init,
		.fini = ham_engine_server_fini,
		.loop = ham_engine_server_loop,
	)
)

int main(int argc, char *argv[]){
	const auto engine = ham_engine_create(HAM_ENGINE_SERVER_API_NAME, HAM_ENGINE_SERVER_OBJ_NAME, argc, argv);

	ham_gengine = engine;

	static const auto sig_handler = +[](int){ ham_engine_request_exit(ham_gengine); }; // this will be U.B. in a signal handler :/

	struct sigaction exit_sigaction{};
	exit_sigaction.sa_handler = sig_handler;

	sigaction(SIGTERM, &exit_sigaction, nullptr);
	sigaction(SIGINT,  &exit_sigaction, nullptr);

	return ham_engine_exec(engine);
}
