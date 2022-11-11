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

#include "net-steam.h"

#include "ham/log.h"

void ham_net_socket_steam_status_changed(SteamNetConnectionStatusChangedCallback_t *info){
	const auto sock = std::bit_cast<ham_net_socket_steam*>(info->m_info.m_nUserData);
	(void)sock;

	const ham_net_peer remote_peer{ .id = (ham_u64)info->m_info.m_identityRemote.GetSteamID64() };

	switch(info->m_info.m_eState){
		case k_ESteamNetworkingConnectionState_None:{
			// Can ignore this
			break;
		}

		case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
		case k_ESteamNetworkingConnectionState_ClosedByPeer:{
			const auto isockets = SteamNetworkingSockets();

			const auto find_res = sock->conns.find(info->m_hConn);
			if(find_res == sock->conns.end()){
				ham_logapierrorf("Disconnection from unaknowledged connection");
				isockets->CloseConnection(info->m_hConn, 0, "disconnect by peer", true);
				return;
			}

			const auto sock_obj = ham_super(sock);

			sock_obj->disconnect_fn(sock_obj, ham_super(&find_res->second), sock_obj->user);

			sock->conns.erase(find_res);

			break;
		}

		case k_ESteamNetworkingConnectionState_Connecting:{
			const auto emplace_res = sock->conns.try_emplace(info->m_hConn);
			if(!emplace_res.second){
				ham_logapierrorf("Failed to allocate connection for peer");
				return;
			}

			const auto sock_obj = ham_super(sock);

			auto new_conn = &emplace_res.first->second;
			auto new_obj = ham_super_n(2, new_conn);

			const auto net_vtable = (const ham_net_vtable*)ham_super(sock_obj->net)->vptr;

			new_obj->vptr = (const ham_object_vtable*)net_vtable->connection_vtable();

			ham_super(new_conn)->net = sock_obj->net;
			ham_super(new_conn)->peer = remote_peer;
			ham_super(new_conn)->accepted_fn = nullptr;
			ham_super(new_conn)->rejected_fn = nullptr;
			ham_super(new_conn)->user = nullptr;

			new_conn->conn = info->m_hConn;

			const auto isockets = SteamNetworkingSockets();

			isockets->SetConnectionUserData(info->m_hConn, std::bit_cast<std::int64_t>(new_conn));

			if(!sock_obj->connect_req_fn(sock_obj, ham_super(new_conn), sock_obj->user)){
				ham_logapiverbosef("Connection request rejected");
				sock->conns.erase(emplace_res.first);
				isockets->CloseConnection(info->m_hConn, 1, "rejected", false);
			}
			else{
				ham_logapiverbosef("Connection request accepted");
				isockets->AcceptConnection(info->m_hConn);
			}

			break;
		}

		case k_ESteamNetworkingConnectionState_Connected:{
			const auto find_res = sock->conns.find(info->m_hConn);
			if(find_res == sock->conns.end()){
				ham_logapierrorf("Failed to find acknowledged connection");
				return;
			}

			const auto sock_obj = ham_super(sock);

			sock_obj->connection_fn(sock_obj, ham_super(&find_res->second), sock_obj->user);

			break;
		}

		default: break;
	}
}

static inline ham_net_socket_steam *ham_net_socket_steam_ctor(ham_net_socket_steam *sock, ham_usize nargs, va_list va){
	if(nargs != 2){
		ham_logapierrorf("Wrong number of arguments %zu, expected 2 (ham_net_peer peer, ham_u16 port)", nargs);
		return nullptr;
	}

	ham_net_peer peer = va_arg(va, ham_net_peer);
	ham_u16 port = va_arg(va, ham_u32);

	const auto isockets = SteamNetworkingSockets();

	const auto ret = new(sock) ham_net_socket_steam;

	SteamNetworkingConfigValue_t options[2];
	options[0].SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void*)ham_net_socket_steam_status_changed);

	if(ham_net_peer_cmp(peer, HAM_NET_EMPTY_PEER) == 0){
		options[1].SetInt64(k_ESteamNetworkingConfig_ConnectionUserData, std::bit_cast<std::int64_t>(sock));

		// listen socket
		const auto socket = isockets->CreateListenSocketP2P(port, std::size(options), options);
		if(socket == k_HSteamListenSocket_Invalid){
			ham_logapierrorf("Error in ISteamNetworkingSockets::CreateListenSocketP2P");
			std::destroy_at(ret);
			return nullptr;
		}

		const auto poll_group = isockets->CreatePollGroup();
		if(poll_group == k_HSteamNetPollGroup_Invalid){
			ham_logapierrorf("Error in ISteamNetworkingSockets::CreatePollGroup");
			isockets->CloseListenSocket(socket);
			std::destroy_at(ret);
			return nullptr;
		}

		const auto listen_ptr = &ret->listen;

		listen_ptr->sock = socket;
		listen_ptr->poll_group = poll_group;
	}
	else{
		const auto conn_ptr = &ret->conn;

		options[1].SetInt64(k_ESteamNetworkingConfig_ConnectionUserData, std::bit_cast<std::int64_t>(conn_ptr));

		// connection
		SteamNetworkingIdentity remote;
		remote.SetSteamID64(peer.id);

		const auto conn = isockets->ConnectP2P(remote, port, std::size(options), options);
		if(conn == k_HSteamNetConnection_Invalid){
			ham_logapierrorf("Error in ISteamNetworkingSockets::ConnectP2P");
			std::destroy_at(ret);
			return nullptr;
		}

		const auto net_vt = (const ham_net_vtable*)ham_super(ham_super(sock)->net)->vptr;

		const auto conn_obj = ham_super(conn_ptr);

		ham_super(conn_obj)->vptr = (const ham_object_vtable*)net_vt->connection_vtable();

		conn_obj->net = ham_super(sock)->net;
		conn_obj->peer = peer;
		conn_obj->accepted_fn = nullptr;
		conn_obj->rejected_fn = nullptr;
		conn_obj->user = nullptr;

		conn_ptr->conn = conn;
	}

	return ret;
}

static inline void ham_net_socket_steam_dtor(ham_net_socket_steam *sock){
	const auto isockets = SteamNetworkingSockets();

	if(ham_super(sock)->is_listen){
		const auto listen_ptr = &sock->listen;

		for(auto &&conn_pair : sock->conns){
			const auto handle = conn_pair.second.conn;
			isockets->CloseConnection(handle, 0, "socket closed", false);
		}

		if(!isockets->DestroyPollGroup(listen_ptr->poll_group)){
			ham_logapiwarnf("Error in ISteamNetworkingSockets::DestroyPollGroup");
		}

		if(!isockets->CloseListenSocket(listen_ptr->sock)){
			ham_logapiwarnf("Error in ISteamNetworkingSockets::CloseListenSocket");
		}
	}
	else{
		const auto conn_ptr = &sock->conn;

		if(!isockets->CloseConnection(conn_ptr->conn, k_ESteamNetConnectionEnd_App_Generic, "ham_net_socket destroyed", true)){
			ham_logapiwarnf("Error in ISteamNetworkingSockets::CloseConnection");
		}
	}

	std::destroy_at(sock);
}

static inline ham_usize ham_net_socket_steam_recv(ham_net_socket_steam *sock, ham_net_socket_recv_fn recv_fn, void *user){
	int num_msgs = 0;
	ham_usize processed_msgs = 0;

	const auto isockets = SteamNetworkingSockets();

	if(ham_super(sock)->is_listen){
		const auto listen_ptr = &sock->listen;

		num_msgs = isockets->ReceiveMessagesOnPollGroup(listen_ptr->poll_group, sock->msg_buf, (int)std::size(sock->msg_buf));
		if(num_msgs == 0){
			return 0;
		}
		else if(num_msgs < 0){
			ham_logapierrorf("Error in ISteamNetworkingSockets::ReceiveMessagesOnPollGroup");
			return (ham_usize)-1;
		}
	}
	else{
		const auto conn_ptr = &sock->conn;

		num_msgs = SteamNetworkingSockets()->ReceiveMessagesOnConnection(conn_ptr->conn, sock->msg_buf, (int)std::size(sock->msg_buf));
		if(num_msgs == 0){
			return 0;
		}
		else if(num_msgs < 0){
			ham_logapierrorf("Error in ISteamNetworkingSockets::ReceiveMessagesOnConnection");
			return (ham_usize)-1;
		}
	}

	for(int i = 0; i < num_msgs; i++){
		const auto msg = sock->msg_buf[i];

		const auto steam_conn = msg->GetConnection();

		const auto conn_ptr = std::bit_cast<const ham_net_connection*>(isockets->GetConnectionUserData(steam_conn));

		const auto data = msg->GetData();
		const auto len = msg->GetSize();

		recv_fn(conn_ptr, data, len, user);

		msg->Release();

		++processed_msgs;
	}

	return processed_msgs;
}

static inline ham_usize ham_net_socket_steam_send(ham_net_socket_steam *sock, const ham_net_connection_steam *conn, const void *data, ham_usize len){
	const auto isockets = SteamNetworkingSockets();

	if(!ham_super(sock)->is_listen){
		const auto conn_ptr = &sock->conn;

		if(conn != conn_ptr){
			ham_logapiwarnf("Message sent through connection socket to other connection");
		}
	}

	const auto result = isockets->SendMessageToConnection(conn->conn, data, (ham_u32)len, k_nSteamNetworkingSend_Reliable, nullptr);

	switch(result){
		case k_EResultOK: return len;

		case k_EResultInvalidParam:{
			ham_logapierrorf("Invalid connection handle, or len is too big");
			return (ham_usize)-1;
		}

		case k_EResultInvalidState:{
			ham_logapierrorf("Connection is in an invalid state");
			return (ham_usize)-1;
		}

		case k_EResultNoConnection:{
			ham_logapierrorf("Connection has ended");
			return (ham_usize)-1;
		}

		case k_EResultLimitExceeded:{
			ham_logapierrorf("There was already too much data queued to be sent");
			return (ham_usize)-1;
		}

		default:{
			ham_logapierrorf("Unknown error");
			return (ham_usize)-1;
		}
	}
}

ham_define_net_socket_object(ham_net_socket_steam, ham_net_connection_steam)
