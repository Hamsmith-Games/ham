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

#ifndef HAM_NET_GNS_H
#define HAM_NET_GNS_H 1

#include "ham/net-object.h" // IWYU pragma: keep

#include <steam/steamnetworkingsockets.h>

HAM_C_API_BEGIN

struct ham_net_gns{
	ham_derive(ham_net)
};

struct ham_net_connection_gns{
	ham_derive(ham_net_connection)
	HSteamNetConnection conn;
};

struct ham_net_socket_gns_listen{
	HSteamListenSocket sock;
};

struct ham_net_socket_gns{
	ham_derive(ham_net_socket)

	union {
		ham_net_socket_gns_listen listen;
		ham_net_connection_gns conn;
	};
};

HAM_C_API_END

#endif // !HAM_NET_GNS_H
