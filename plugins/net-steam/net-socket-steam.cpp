#include "net-steam.h"

#include "ham/log.h"

static inline ham_net_socket_steam *ham_net_socket_steam_ctor(ham_net_socket_steam *sock, ham_usize nargs, va_list va){
	if(nargs != 2){
		ham_logapierrorf("Wrong number of arguments %zu, expected 2 (ham_net_peer peer, ham_u16 port)", nargs);
		return nullptr;
	}

	ham_net_peer peer = va_arg(va, ham_net_peer);
	ham_u16 port = va_arg(va, ham_u32);

	const auto isockets = SteamNetworkingSockets();

	const auto ret = new(sock) ham_net_socket_steam;

	if(ham_net_peer_cmp(peer, HAM_NET_EMPTY_PEER) == 0){
		// listen socket
		const auto socket = isockets->CreateListenSocketP2P(port, 0, nullptr);
		if(socket == k_HSteamListenSocket_Invalid){
			ham_logapierrorf("Error in ISteamNetworkingSockets::CreateListenSocketP2P");
			std::destroy_at(ret);
			return nullptr;
		}

		ret->steam_handle.sock = socket;
	}
	else{
		// connection
		SteamNetworkingIdentity remote;
		remote.SetSteamID64(peer.id);

		const auto conn = isockets->ConnectP2P(remote, port, 0, nullptr);
		if(conn == k_HSteamNetConnection_Invalid){
			ham_logapierrorf("Error in ISteamNetworkingSockets::ConnectP2P");
			std::destroy_at(ret);
			return nullptr;
		}

		ret->steam_handle.conn = conn;
	}

	return ret;
}

static inline void ham_net_socket_steam_dtor(ham_net_socket_steam *sock){
	const auto isockets = SteamNetworkingSockets();

	if(ham_super(sock)->is_listen){
		if(!isockets->CloseListenSocket(sock->steam_handle.sock)){
			ham_logapiwarnf("Error in ISteamNetworkingSockets::CloseListenSocket");
		}
	}
	else{
		if(!isockets->CloseConnection(sock->steam_handle.conn, k_ESteamNetConnectionEnd_App_Generic, "ham_net_socket destroyed", true)){
			ham_logapiwarnf("Error in ISteamNetworkingSockets::CloseConnection");
		}
	}

	std::destroy_at(sock);
}

ham_define_net_socket_object(ham_net_socket_steam)
