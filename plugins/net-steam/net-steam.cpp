#include "ham/net-vtable.h"
#include "ham/plugin.h"
#include "ham/log.h"

#include <steam/steam_api.h>

HAM_C_API_BEGIN

extern ham_nothrow ham_u32 ham_net_steam_appid();

struct ham_net_steam{
	ham_derive(ham_net)
};

static ham_net_steam *ham_net_ctor_steam(ham_net_steam *net, va_list va){
	(void)va;
	return new(net) ham_net_steam;
}

static void ham_net_dtor_steam(ham_net_steam *net){
	std::destroy_at(net);
}

static void ham_net_loop_steam(ham_net *net, ham_f64 dt){
	(void)net; (void)dt;

	SteamAPI_RunCallbacks();
}

ham_define_object_x(2, ham_net_steam, 1, ham_net_vtable, ham_net_ctor_steam, ham_net_dtor_steam, ( .loop = ham_net_loop_steam ))

static void ham_impl_steam_debug(ESteamNetworkingSocketsDebugOutputType type, const char *msg){
	switch(type){
		case k_ESteamNetworkingSocketsDebugOutputType_Bug:
		case k_ESteamNetworkingSocketsDebugOutputType_Error:{ ham_logerrorf("Steamworks", "%s", msg); break; }
		case k_ESteamNetworkingSocketsDebugOutputType_Warning:{ ham_logwarnf("Steamworks", "%s", msg); break; }
		case k_ESteamNetworkingSocketsDebugOutputType_Verbose:{ ham_logverbosef("Steamworks", "%s", msg); break; }
		case k_ESteamNetworkingSocketsDebugOutputType_Debug:{ ham_logdebugf("Steamworks", "%s", msg); break; }

		case k_ESteamNetworkingSocketsDebugOutputType_Important:{
			ham_loginfof("Steamworks", "IMPORTANT! %s", msg);
			break;
		}

		case k_ESteamNetworkingSocketsDebugOutputType_Everything:
		case k_ESteamNetworkingSocketsDebugOutputType_Msg:
		case k_ESteamNetworkingSocketsDebugOutputType_None:
		default:{
			ham_loginfof("Steamworks", "%s", msg);
			break;
		}
	}
}

static bool ham_net_on_load_steam(){
	if(SteamAPI_RestartAppIfNecessary(ham_net_steam_appid())){
		ham_logapierrorf("Application must be launched from steam to use the Steamworks SDK");
		return false;
	}

	return SteamAPI_Init();
}

static void ham_net_on_unload_steam(){
	SteamAPI_Shutdown();
}

HAM_C_API_END

HAM_PLUGIN(
	ham_net_context_steam,
	HAM_NET_STEAMWORKS_PLUGIN_UUID,
	HAM_NET_STEAMWORKS_PLUGIN_NAME,
	HAM_VERSION,
	"Steam Networking",
	"Hamsmith Ltd.",
	"GPLv3+",
	HAM_NET_PLUGIN_CATEGORY,
	"Networking using Valve's Steamworks SDK",

	ham_net_on_load_steam,
	ham_net_on_unload_steam
)
