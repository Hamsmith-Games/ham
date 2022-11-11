/*
 * Ham Runtime
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
 * @brief Opaque handle representing a connection from one socket to another socket.
 */
typedef struct ham_net_connection ham_net_connection;

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
 * @defgroup HAM_NET_CONNECTION Connections
 * @{
 */

typedef void(*ham_net_connection_accepted_fn)(ham_net_connection *conn, void *user);
typedef void(*ham_net_connection_rejected_fn)(ham_net_connection *conn, void *user);
typedef void(*ham_net_connection_disconnect_fn)(ham_net_connection *conn, void *user);

ham_api ham_net_connection *ham_net_connection_create(
	ham_net *net,
	ham_net_peer remote_peer, ham_u16 remote_port,
	ham_net_connection_accepted_fn accepted_fn,
	ham_net_connection_rejected_fn rejected_fn,
	ham_net_connection_disconnect_fn disconnect_fn,
	void *user
);

ham_api void ham_net_connection_destroy(ham_net_connection *conn);

typedef void(*ham_net_connection_recv_fn)(const void *data, ham_usize len, void *user);

/**
 * Receive messages on a connection.
 * @param conn connection to poll
 * @param recv_fn function to call on each messages data
 * @param user data passed in each call to \p recv_fn
 * @returns number of messages received or ``(ham_usize)-1`` on error
 */
ham_api ham_usize ham_net_connection_recv(ham_net_connection *conn, ham_net_connection_recv_fn recv_fn, void *user);

/**
 * Send messages through a connection.
 * @param conn connection to use
 * @param data data to send
 * @param len size of \p data in bytes
 * @returns whether the message was successfully sent
 */
ham_api bool ham_net_connection_send(ham_net_connection *conn, const void *data, ham_usize len);

/**
 * @}
 */

/**
 * @defgroup HAM_NET_SOCKET Sockets
 * @{
 */

typedef bool(*ham_net_socket_connection_request_fn)(ham_net_socket *sock, const ham_net_connection *conn, void *user);
typedef void(*ham_net_socket_connection_fn)(ham_net_socket *sock, const ham_net_connection *conn, void *user);
typedef void(*ham_net_socket_disconnect_fn)(ham_net_socket *sock, const ham_net_connection *conn, void *user);

typedef ham_usize(*ham_net_socket_message_fn)(ham_net_socket *sock, const ham_net_connection *conn, ham_usize len, const void *bytes, void *user);

/**
 * @brief Create a listen socket.
 * @param net net object to create socket with
 * @param port port to listen on
 * @param connect_req_fn connection request callback
 * @param connection_fn connection successful callback
 * @param disconnect_fn disconnection callback
 * @param user data passed to all callbacks
 * @returns newly created socket object or ``NULL`` on error
 */
ham_api ham_net_socket *ham_net_socket_create(
	ham_net *net, ham_u16 port,
	ham_net_socket_connection_request_fn connect_req_fn,
	ham_net_socket_connection_fn connection_fn,
	ham_net_socket_disconnect_fn disconnect_fn,
	void *user
);

/**
 * @brief Destroy a socket.
 * @param socket socket object to destroy
 */
ham_api void ham_net_socket_destroy(ham_net_socket *socket);

/**
 * @brief Check if a socket is a listen socket.
 */
ham_api bool ham_net_socket_is_listen(const ham_net_socket *socket);

ham_api ham_usize ham_net_socket_send(ham_net_socket *socket, const ham_net_connection *conn, const void *data, ham_usize len);

typedef void(*ham_net_socket_recv_fn)(const ham_net_connection *conn, const void *data, ham_usize len, void *user);

ham_api ham_usize ham_net_socket_recv(ham_net_socket *socket, ham_net_socket_recv_fn recv_fn, void *user);

/**
 * @}
 */

HAM_C_API_END

/**
 * @}
 */

#endif // !HAM_NET_H
