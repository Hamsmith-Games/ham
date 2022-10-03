#include "net-gns.h"

static inline ham_net_socket_gns *ham_net_socket_gns_ctor(ham_net_socket_gns *sock, ham_usize nargs, va_list va){
	return new(sock) ham_net_socket_gns;
}

static inline void ham_net_socket_gns_dtor(ham_net_socket_gns *sock){
	std::destroy_at(sock);
}

ham_define_net_socket_object(ham_net_socket_gns)
