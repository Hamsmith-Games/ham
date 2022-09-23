#include "ham/net-vtable.h"
#include "ham/log.h"

#include <steam/steamnetworkingsockets.h>

HAM_C_API_BEGIN

struct ham_net_context_gns{
	ham_derive(ham_net_context)
};

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

static bool ham_net_init_gns(ham_net_context_gns *ctx){
	(void)ctx;
	return true;
}

static void ham_net_finish_gns(ham_net_context_gns *ctx){
	(void)ctx;
}

static void ham_net_loop_gns(ham_net_context_gns *ctx, ham_f64 dt){
	(void)ctx; (void)dt;
}

HAM_C_API_END

HAM_NET_VTABLE(
	ham_net_context_gns,
	HAM_NET_DEFAULT_PLUGIN_UUID,
	HAM_NET_DEFAULT_PLUGIN_NAME,
	HAM_VERSION,
	"GameNetworkingSockets",
	"Hamsmith Ltd.",
	"GPLv3+",
	"Default networking using Valve's GameNetworkingSockets",

	ham_net_on_load_gns,
	ham_net_on_unload_gns,
	ham_net_init_gns,
	ham_net_finish_gns,
	ham_net_loop_gns
)
