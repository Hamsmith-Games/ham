#include "ham/net-object.h"
#include "ham/plugin.h"
#include "ham/log.h"

#include "net-steam.h"

#include <inttypes.h>

#include <steam/steam_api.h>
#include <steam/isteamnetworkingutils.h>

HAM_C_API_BEGIN

extern ham_nothrow ham_u32 ham_net_steam_appid();

static ham_net_steam *ham_net_steam_ctor(ham_net_steam *net, ham_usize nargs, va_list va){
	(void)nargs; (void)va;
	return new(net) ham_net_steam;
}

static void ham_net_steam_dtor(ham_net_steam *net){
	std::destroy_at(net);
}

static bool ham_net_steam_init(ham_net_steam *net){
	(void)net;
	return true;
}

static void ham_net_steam_fini(ham_net_steam *net){
	(void)net;
}

static void ham_net_steam_loop(ham_net_steam *net, ham_f64 dt){
	(void)net; (void)dt;

	SteamAPI_RunCallbacks();
}

static bool ham_net_steam_find_peer(ham_net_steam *net, ham_net_peer *ret, ham_str8 query){
	if(!query.ptr || !query.len){
		const auto user = SteamUser();
		const auto friends = SteamFriends();

		const CSteamID user_id = user->GetSteamID();
		ret->id = (ham_u64)user_id.ConvertToUint64();

		const char *user_name = friends->GetPersonaName();
		ham_logapidebugf("Got current user (%" PRIu64 ") %s", ret->id, user_name);

		return true;
	}

	ham_name_buffer_utf8 query_buf;
	if(query.len >= sizeof(query_buf)){
		ham_logapierrorf(
			"Query string too long (%zu, max %d): %.*s",
			query.len, HAM_NAME_BUFFER_SIZE-1,
			(int)ham_min(query.len, HAM_I32_MAX), query.ptr
		);
		return false;
	}

	memcpy(query_buf, query.ptr, query.len);
	query_buf[query.len] = '\0';

	const auto friends = SteamFriends();

	const int friend_flags = k_EFriendFlagAll & ~(k_EFriendFlagBlocked | k_EFriendFlagIgnored);

	const int num_friends = friends->GetFriendCount(friend_flags);
	if(num_friends == -1){
		ham_logapierrorf("Error getting number of friends: current user is not logged on");
		return false;
	}

	for(int i = 0; i < num_friends; i++){
		const CSteamID friend_id = friends->GetFriendByIndex(i, friend_flags);
		const char *friend_name = friends->GetFriendPersonaName(friend_id);
		const char *friend_nick = friends->GetPlayerNickname(friend_id);

		if(strcmp(query_buf, friend_name) == 0 || strcmp(query_buf, friend_nick) == 0){
			ret->id = (ham_u64)friend_id.ConvertToUint64();
			ham_logapidebugf("Found friend (%" PRIu64 ") %s", ret->id, friend_name);
			return true;
		}
	}

	return false;
}

ham_define_net_object(ham_net_steam, ham_net_socket_steam)

static void ham_impl_steam_debug(ESteamNetworkingSocketsDebugOutputType type, const char *msg){
	switch(type){
		case k_ESteamNetworkingSocketsDebugOutputType_Bug:
		case k_ESteamNetworkingSocketsDebugOutputType_Error:{ ham_logerrorf("Steamworks", "%s", msg); break; }
		case k_ESteamNetworkingSocketsDebugOutputType_Warning:{ ham_logwarnf("Steamworks", "%s", msg); break; }
		case k_ESteamNetworkingSocketsDebugOutputType_Verbose:{ ham_logverbosef("Steamworks", "%s", msg); break; }
		case k_ESteamNetworkingSocketsDebugOutputType_Debug:{ ham_logdebugf("Steamworks", "%s", msg); break; }

		case k_ESteamNetworkingSocketsDebugOutputType_Important:{
			ham_logwarnf("Steamworks", "IMPORTANT! %s", msg);
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

	if(!SteamAPI_Init()){
		ham_logapierrorf("Failed to initialize Steam API");
		return false;
	}

	SteamNetworkingUtils()->SetDebugOutputFunction(
		#ifdef HAM_DEBUG
		k_ESteamNetworkingSocketsDebugOutputType_Msg,
		#else
		k_ESteamNetworkingSocketsDebugOutputType_Warning,
		#endif
		ham_impl_steam_debug
	);

	return true;
}

static void ham_net_on_unload_steam(){
	SteamAPI_Shutdown();
}

HAM_C_API_END

HAM_PLUGIN(
	ham_net_steam,
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
