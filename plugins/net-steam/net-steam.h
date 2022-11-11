/*
 * Ham Runtime Plugins
 * Copyright (C) 2022 Keith Hammond
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

#ifndef HAM_NET_STEAM_H
#define HAM_NET_STEAM_H 1

#include "ham/net-object.h" // IWYU pragma: keep

#include <steam/isteamnetworkingsockets.h>

#include "robin_hood.h"

HAM_C_API_BEGIN

struct ham_net_steam{
	ham_derive(ham_net)
};

struct ham_net_connection_steam{
	ham_derive(ham_net_connection)
	HSteamNetConnection conn;
};

struct ham_net_socket_steam_listen{
	HSteamListenSocket sock;
	HSteamNetPollGroup poll_group;
};

struct ham_net_socket_steam{
	ham_derive(ham_net_socket)

	union {
		ham_net_socket_steam_listen listen;
		ham_net_connection_steam conn;
	};

	ISteamNetworkingMessage *msg_buf[8];
	robin_hood::unordered_node_map<HSteamNetConnection, ham_net_connection_steam> conns;
};

HAM_C_API_END

#endif // !HAM_NET_STEAM_H
