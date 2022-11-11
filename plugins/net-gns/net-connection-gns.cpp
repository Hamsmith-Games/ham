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

#include "net-gns.h"

static inline ham_net_connection_gns *ham_net_connection_gns_ctor(ham_net_connection_gns *ptr, ham_u32 nargs, va_list va){
	return new(ptr) ham_net_connection_gns;
}

static inline void ham_net_connection_gns_dtor(ham_net_connection_gns *ptr){
	std::destroy_at(ptr);
}

static inline ham_usize ham_net_connection_gns_send(ham_net_connection_gns *conn, const void *data, ham_usize len){
	return (ham_usize)-1;
}

static inline ham_usize ham_net_connection_gns_recv(ham_net_connection_gns *conn, ham_net_connection_recv_fn fn, void *user){
	return (ham_usize)-1;
}

ham_define_net_connection_object(ham_net_connection_gns)
