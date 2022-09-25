#include "ham/net-vtable.h"
#include "ham/plugin.h"
#include "ham/log.h"

#include <steam/steamnetworkingsockets.h>

HAM_C_API_BEGIN

struct ham_net_gns{
	ham_derive(ham_net)
};

struct ham_net_vtable_gns{
	ham_derive(ham_net_vtable)
};

static ham_net_gns *ham_net_gns_ctor(ham_net_gns *net, va_list va){
	(void)va;
	return new(net) ham_net_gns;
}

static void ham_net_gns_dtor(ham_net_gns *net){
	std::destroy_at(net);
}

static void ham_net_gns_loop(ham_net *net, ham_f64 dt){
	(void)net; (void)dt;

}

ham_define_object_x(2, ham_net_gns, 1, ham_net_vtable, ham_net_gns_ctor, ham_net_gns_dtor, ( .loop = ham_net_gns_loop ))

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
