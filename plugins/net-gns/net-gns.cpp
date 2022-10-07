/*
 * Ham Runtime Plugins
 * Copyright (C) 2022  Hamsmith Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "ham/net-object.h"
#include "ham/plugin.h"
#include "ham/log.h"

#include "net-gns.h"

HAM_C_API_BEGIN

static ham_net_gns *ham_net_gns_ctor(ham_net_gns *net, ham_usize nargs, va_list va){
	(void)nargs; (void)va;
	return new(net) ham_net_gns;
}

static void ham_net_gns_dtor(ham_net_gns *net){
	std::destroy_at(net);
}

static bool ham_net_gns_init(ham_net_gns *net){
	(void)net;
	return true;
}

static void ham_net_gns_fini(ham_net_gns *net){
	(void)net;
}

static void ham_net_gns_loop(ham_net_gns *net, ham_f64 dt){
	(void)net; (void)dt;

}

static bool ham_net_gns_find_peer(ham_net_gns *net, ham_net_peer *ret, ham_str8 query){
	return false;
}

ham_define_net_object(ham_net_gns, ham_net_socket_gns, ham_net_connection_gns)

static void ham_impl_gns_debug(ESteamNetworkingSocketsDebugOutputType type, const char *msg){
	switch(type){
		case k_ESteamNetworkingSocketsDebugOutputType_Bug:
		case k_ESteamNetworkingSocketsDebugOutputType_Error:{ ham_logerrorf("GameNetworkingSockets", "%s", msg); break; }
		case k_ESteamNetworkingSocketsDebugOutputType_Warning:{ ham_logwarnf("GameNetworkingSockets", "%s", msg); break; }
		case k_ESteamNetworkingSocketsDebugOutputType_Verbose:{ ham_logverbosef("GameNetworkingSockets", "%s", msg); break; }
		case k_ESteamNetworkingSocketsDebugOutputType_Debug:{ ham_logdebugf("GameNetworkingSockets", "%s", msg); break; }

		case k_ESteamNetworkingSocketsDebugOutputType_Important:{
			ham_loginfof("GameNetworkingSockets", "IMPORTANT! %s", msg);
			break;
		}

		case k_ESteamNetworkingSocketsDebugOutputType_Everything:
		case k_ESteamNetworkingSocketsDebugOutputType_Msg:
		case k_ESteamNetworkingSocketsDebugOutputType_None:
		default:{
			ham_loginfof("GameNetworkingSockets", "%s", msg);
			break;
		}
	}
}

static bool ham_net_on_load_gns(){
	SteamDatagramErrMsg err_msg;
	if(!GameNetworkingSockets_Init(nullptr, err_msg)){
		ham_logapierrorf("Error in GameNetworkingSockets_Init: %s", err_msg);
		return false;
	}

	return true;
}

static void ham_net_on_unload_gns(){
	GameNetworkingSockets_Kill();
}

HAM_C_API_END

HAM_PLUGIN(
	ham_net_gns,
	HAM_NET_GNS_PLUGIN_UUID,
	HAM_NET_GNS_PLUGIN_NAME,
	HAM_VERSION,
	"GameNetworkingSockets",
	"Hamsmith Ltd.",
	"GPLv3+",
	HAM_NET_PLUGIN_CATEGORY,
	"Networking using Valve's GameNetworkingSockets",
	ham_net_on_load_gns,
	ham_net_on_unload_gns
)
