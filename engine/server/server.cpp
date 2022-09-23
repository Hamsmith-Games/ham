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

#include "ham/engine-vtable.h"
#include "ham/log.h"

#include "ham/net.h"

struct ham_engine_server_context{
	ham_derive(ham_engine_context)

	ham_net_context *net;
};

static inline bool ham_engine_server_on_load(){ return true; }
static inline void ham_engine_server_on_unload(){}

static inline bool ham_engine_server_init(ham_engine_server_context *ctx){
	(void)ctx;

	ctx->net = ham_net_context_create(HAM_NET_DEFAULT_PLUGIN_NAME);
	if(!ctx->net){
		ham_logapierrorf("Error in ham_net_context_create");
		return false;
	}

	return true;
}

static inline void ham_engine_server_finish(ham_engine_server_context *ctx){
	ham_net_context_destroy(ctx->net);
}

static inline void ham_engine_server_loop(ham_engine_server_context *ctx, ham_f64 dt){
	ham_net_context_loop(ctx->net, dt);
	ham_engine_request_exit(ham_super(ctx));
}

HAM_ENGINE_VTABLE(
	ham_engine_server_context,
	HAM_ENGINE_SERVER_PLUGIN_UUID,
	HAM_ENGINE_SERVER_API_NAME,
	HAM_VERSION,
	"Ham World Engine Server",
	"Hamsmith Ltd.",
	"GPLv3+",
	"Ham World Engine Server",
	ham_engine_server_on_load,
	ham_engine_server_on_unload,
	ham_engine_server_init,
	ham_engine_server_finish,
	ham_engine_server_loop
)

int main(int argc, char *argv[]){
	const auto ctx = ham_engine_create(HAM_ENGINE_SERVER_API_NAME, argc, argv);
	return ham_engine_exec(ctx);
}
