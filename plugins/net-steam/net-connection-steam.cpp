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

HAM_C_API_BEGIN

void ham_net_connection_steam_status_changed(SteamNetConnectionStatusChangedCallback_t *info){
	const auto conn_ptr = std::bit_cast<ham_net_connection_steam*>(info->m_info.m_nUserData);

	switch(info->m_info.m_eState){
		case k_ESteamNetworkingConnectionState_None:{
			// Can ignore this
			break;
		}

		case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
		case k_ESteamNetworkingConnectionState_ClosedByPeer:{
			const auto isockets = SteamNetworkingSockets();

			const auto conn_obj = ham_super(conn_ptr);

			if(conn_obj->connected){
				conn_obj->connected = false;
				conn_obj->disconnect_fn(conn_obj, conn_obj->user);
			}
			else{
				conn_obj->rejected_fn(conn_obj, conn_obj->user);
			}

			isockets->CloseConnection(conn_ptr->conn, 0, "closed by peer", false);
			conn_ptr->conn = k_HSteamNetConnection_Invalid;

			break;
		}

		case k_ESteamNetworkingConnectionState_Connecting:{
			// TODO: handle connecting
			break;
		}

		case k_ESteamNetworkingConnectionState_Connected:{
			const auto conn_obj = ham_super(conn_ptr);

			conn_obj->connected = true;
			conn_obj->accepted_fn(conn_obj, conn_obj->user);

			break;
		}

		default:{
			break;
		}
	}
}

static inline ham_net_connection_steam *ham_net_connection_steam_ctor(ham_net_connection_steam *ptr, ham_u32 nargs, va_list va){
	if(nargs != 2){
		ham_logapierrorf("Got %u args, expected 2 (ham_net_peer peer, ham_u16 port)", nargs);
		return nullptr;
	}

	ham_net_peer peer = va_arg(va, ham_net_peer);
	ham_u16 port = (ham_u16)va_arg(va, ham_u32);

	const auto isockets = SteamNetworkingSockets();

	SteamNetworkingIdentity ident;
	ident.SetSteamID64(peer.id);

	SteamNetworkingConfigValue_t options[2];
	options[0].SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void*)ham_net_connection_steam_status_changed);
	options[1].SetInt64(k_ESteamNetworkingConfig_ConnectionUserData, std::bit_cast<std::int64_t>(ptr));

	const auto conn_handle = isockets->ConnectP2P(ident, port, (int)std::size(options), options);
	if(conn_handle == k_HSteamNetConnection_Invalid){
		ham_logapierrorf("Error in ISteamNetworkingSockets::ConnectP2P");
		return nullptr;
	}

	const auto ret = new(ptr) ham_net_connection_steam;

	ret->conn = conn_handle;

	return ret;
}

static inline void ham_net_connection_steam_dtor(ham_net_connection_steam *ptr){
	const auto isockets = SteamNetworkingSockets();

	const auto obj = ham_super(ptr);

	if(obj->connected){
		obj->disconnect_fn(obj, obj->user);
	}

	isockets->CloseConnection(ptr->conn, 0, "user destroyed connection", false);

	std::destroy_at(ptr);
}

static inline ham_usize ham_net_connection_steam_send(ham_net_connection_steam *conn, const void *data, ham_usize len){
	const auto isockets = SteamNetworkingSockets();

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

static inline ham_usize ham_net_connection_steam_recv(ham_net_connection_steam *conn, ham_net_connection_recv_fn fn, void *user){
	const auto isockets = SteamNetworkingSockets();

	SteamNetworkingMessage_t *msgs[8];

	const int num_msgs = isockets->ReceiveMessagesOnConnection(conn->conn, msgs, 8);
	if(num_msgs == 0){
		return 0;
	}
	else if(num_msgs < 0){
		ham_logapierrorf("Error in ISteamNetworkingSockets::ReceiveMessagesOnConnection");
		return (ham_usize)-1;
	}

	for(int i = 0; i < num_msgs; i++){
		const auto msg = msgs[i];

		const auto data = msg->GetData();
		const auto len = msg->GetSize();

		fn(data, len, user);

		msg->Release();
	}

	return (ham_usize)num_msgs;
}

HAM_C_API_END

ham_define_net_connection_object(ham_net_connection_steam)
