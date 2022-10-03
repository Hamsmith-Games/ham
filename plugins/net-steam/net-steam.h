#ifndef HAM_NET_STEAM_H
#define HAM_NET_STEAM_H 1

#include "ham/net-object.h" // IWYU pragma: keep

#include <steam/isteamnetworkingsockets.h>

HAM_C_API_BEGIN

struct ham_net_steam{
	ham_derive(ham_net)
};

struct ham_net_socket_steam{
	ham_derive(ham_net_socket)
	union {
		HSteamListenSocket sock;
		HSteamNetConnection conn;
	} steam_handle;
};

HAM_C_API_END

#endif // !HAM_NET_STEAM_H
