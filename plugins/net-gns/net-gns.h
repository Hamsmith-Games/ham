#ifndef HAM_NET_GNS_H
#define HAM_NET_GNS_H 1

#include "ham/net-object.h" // IWYU pragma: keep

#include <steam/steamnetworkingsockets.h>

HAM_C_API_BEGIN

struct ham_net_gns{
	ham_derive(ham_net)
};

struct ham_net_socket_gns{
	ham_derive(ham_net_socket)
	union {
		HSteamListenSocket sock;
		HSteamNetConnection conn;
	} steam_handle;
};

HAM_C_API_END

#endif // !HAM_NET_GNS_H
