/*
 * Ham Runtime
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

#ifndef HAM_NET_H
#define HAM_NET_H 1

/**
 * @defgroup HAM_NET Networking
 * @ingroup HAM
 * @{
 */

#include "typedefs.h"

HAM_C_API_BEGIN

/**
 * @brief Net object.
 */
typedef struct ham_net ham_net;

/**
 * @brief Union used for storing peer information.
 * @warning Do not directly use the members of this union.
 */
typedef union ham_net_peer{
	ham_u64 id;
	ham_u8 ipv4[4];
	ham_u16 ipv6[8];
} ham_net_peer;

#define HAM_NET_EMPTY_PEER ((ham_net_peer){.ipv6={0,0,0,0,0,0,0,0}})

ham_nothrow static inline int ham_net_peer_cmp(ham_net_peer lhs, ham_net_peer rhs){
	return memcmp(&lhs, &rhs, sizeof(ham_net_peer));
}

/**
 * @brief Server object.
 */
typedef struct ham_net_server ham_net_server;

/**
 * @brief Client object.
 */
typedef struct ham_net_client ham_net_client;

/**
 * @brief Socket for listening or connecting.
 */
typedef struct ham_net_socket ham_net_socket;

/**
 * @brief Create a new net object.
 * @param plugin_id plugin to use
 * @param obj_id object type within \p plugin
 * @returns newly created net object or ``NULL`` on error
 * @see ham_net_destroy
 */
ham_api ham_net *ham_net_create(const char *plugin_id, const char *obj_id);

/**
 * @brief Destroy a net object.
 * @param net net object to destroy
 * @see ham_net_create
 */
ham_api void ham_net_destroy(ham_net *net);

/**
 * @brief Call the ``loop`` function on a net object.
 * @param net net object to use
 * @param dt dt passed to ``loop``
 */
ham_api void ham_net_loop(ham_net *net, ham_f64 dt);

/**
 * @brief Find a networking peer based on a query.
 * The given query can be a valid network address or a plugin-specific identifier.
 * @param net net object to query
 * @param ret where to return the result
 * @param query search query
 * @returns whether a peer was successfully found
 */
ham_api bool ham_net_find_peer(ham_net *net, ham_net_peer *ret, ham_str8 query);

/**
 * @brief Create a listen or connection socket.
 * @param net net object to create socket with
 * @param peer ``HAM_NET_NULL_PEER`` for a listen socket, or the peer to connect to
 * @param port port to listen on or connect through
 * @returns newly created socket object or ``NULL`` on error
 */
ham_api ham_net_socket *ham_net_socket_create(ham_net *net, ham_net_peer peer, ham_u16 port);

/**
 * @brief Destroy a socket.
 * @param socket socket object to destroy
 */
ham_api void ham_net_socket_destroy(ham_net_socket *socket);

HAM_C_API_END

/**
 * @}
 */

#endif // !HAM_NET_H
