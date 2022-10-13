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

#ifndef HAM_NET_OBJECT_H
#define HAM_NET_OBJECT_H 1

/**
 * @defgroup HAM_NET_VTABLE Networking plugin vtables
 * @ingroup HAM_NET
 * @{
 */

#include "net.h"

#include "ham/dso.h"
#include "ham/object.h"
#include "ham/memory.h"

HAM_C_API_BEGIN

typedef struct ham_net_vtable ham_net_vtable;
typedef struct ham_net_socket_vtable ham_net_socket_vtable;
typedef struct ham_net_connection_vtable ham_net_connection_vtable;

struct ham_net{
	ham_derive(ham_object)

	const ham_allocator *allocator;
	ham_dso_handle dso;
};

struct ham_net_socket{
	ham_derive(ham_object)
	ham_net *net;
	ham_net_peer peer;
	ham_u16 port;
	bool is_listen;

	ham_net_socket_connection_request_fn connect_req_fn;
	ham_net_socket_connection_fn connection_fn;
	ham_net_socket_disconnect_fn disconnect_fn;
	void *user;
};

struct ham_net_connection{
	ham_derive(ham_object)
	ham_net *net;
	ham_net_peer peer;
	bool connected;

	ham_net_connection_accepted_fn accepted_fn;
	ham_net_connection_rejected_fn rejected_fn;
	ham_net_connection_disconnect_fn disconnect_fn;
	void *user;
};

struct ham_net_vtable{
	ham_derive(ham_object_vtable)

	const ham_net_socket_vtable*(*socket_vtable)();
	const ham_net_connection_vtable*(*connection_vtable)();

	bool(*init)(ham_net *net);
	void(*fini)(ham_net *net);
	void(*loop)(ham_net *net, ham_f64 dt);

	bool(*find_peer)(ham_net *net, ham_net_peer *ret, ham_str8 query);
};

struct ham_net_socket_vtable{
	ham_derive(ham_object_vtable)

	ham_usize(*send)(
		ham_net_socket *sock, const ham_net_connection *conn,
		const void *buf, ham_usize buf_len
	);

	ham_usize(*recv)(ham_net_socket *sock, ham_net_socket_recv_fn recv_fn, void *user);
};

struct ham_net_connection_vtable{
	ham_derive(ham_object_vtable)

	ham_usize(*send)(ham_net_connection *conn, const void *data, ham_usize len);
	ham_usize(*recv)(ham_net_connection *conn, ham_net_connection_recv_fn recv_fn, void *user);
};

//! @cond ignore
#define ham_impl_define_net_object( \
	derived_, derived_socket_, derived_connection_, \
	ctor_fn_, dtor_fn_, \
	init_name_, init_fn_, \
	fini_name_, fini_fn_, \
	loop_name_, loop_fn_, \
	find_peer_name_, find_peer_fn_ \
) \
	const ham_object_vtable *ham_object_vptr_name(derived_socket_)(); \
	const ham_object_vtable *ham_object_vptr_name(derived_connection_)(); \
	static const ham_net_socket_vtable *ham_impl_socket_vtable_##derived_(){ return (const ham_net_socket_vtable*)ham_object_vptr_name(derived_socket_)(); } \
	static const ham_net_connection_vtable *ham_impl_connection_vtable_##derived_(){ return (const ham_net_connection_vtable*)ham_object_vptr_name(derived_connection_)(); } \
	static bool init_name_(ham_net *net){ return (init_fn_)((derived_*)net); } \
	static void fini_name_(ham_net *net){ (fini_fn_)((derived_*)net); } \
	static void loop_name_(ham_net *net, ham_f64 dt){ (loop_fn_)((derived_*)net, dt); } \
	static bool find_peer_name_(ham_net *net, ham_net_peer *ret, ham_str8 query){ return (find_peer_fn_)((derived_*)net, ret, query); } \
	ham_define_object_x( \
		2, derived_, 1, ham_net_vtable, \
		ctor_fn_, dtor_fn_, \
		(	.socket_vtable = ham_impl_socket_vtable_##derived_, \
			.connection_vtable = ham_impl_connection_vtable_##derived_, \
			.init = init_name_, \
			.fini = fini_name_, \
			.loop = loop_name_, \
			.find_peer = find_peer_name_, \
		) \
	)

#define ham_impl_define_net_socket_object(derived_, derived_conn_, send_fn_, recv_fn_) \
	static ham_usize ham_impl_send_##derived_(ham_net_socket *sock, const ham_net_connection *conn, const void *data, ham_usize len){ \
		return (send_fn_)((derived_*)sock, (const derived_conn_*)conn, data, len); \
	} \
	static ham_usize ham_impl_recv_##derived_(ham_net_socket *sock, ham_net_socket_recv_fn fn, void *user){ \
		return (recv_fn_)((derived_*)sock, fn, user); \
	} \
	ham_define_object_x( \
		2, derived_, 1, ham_net_socket_vtable, \
		derived_##_ctor, derived_##_dtor, \
		(	.send = ham_impl_send_##derived_, \
			.recv = ham_impl_recv_##derived_, \
		) \
	)

#define ham_impl_define_net_connection_object(derived_, send_fn_, recv_fn_) \
	static ham_usize ham_impl_send_##derived_(ham_net_connection *conn, const void *data, ham_usize len){ \
		return (send_fn_)((derived_*)conn, data, len); \
	} \
	static ham_usize ham_impl_recv_##derived_(ham_net_connection *conn, ham_net_connection_recv_fn fn, void *user){ \
		return (recv_fn_)((derived_*)conn, fn, user); \
	} \
	ham_define_object_x( \
		2, derived_, 1, ham_net_connection_vtable, \
		derived_##_ctor, derived_##_dtor, \
		(	.send = ham_impl_send_##derived_, \
			.recv = ham_impl_recv_##derived_, \
		) \
	)

//! @endcond

#define ham_define_net_object(derived, derived_socket, derived_connection) \
	ham_impl_define_net_object( \
		derived, derived_socket, derived_connection, \
		derived##_ctor, \
		derived##_dtor, \
		ham_impl_init_##derived, derived##_init, \
		ham_impl_fini_##derived, derived##_fini, \
		ham_impl_loop_##derived, derived##_loop, \
		ham_impl_find_peer_##derived, derived##_find_peer \
	)

#define ham_define_net_socket_object(derived, derived_conn) \
	ham_impl_define_net_socket_object(derived, derived_conn, derived##_send, derived##_recv)

#define ham_define_net_connection_object(derived) \
	ham_impl_define_net_connection_object(derived, derived##_send, derived##_recv)

HAM_C_API_END

#ifdef __cplusplus

namespace ham{
	namespace detail{
		template<> struct ham_object_vtable<ham_net>: id<ham_net_vtable>{};
		template<> struct ham_object_vtable<ham_net_socket>: id<ham_net_socket_vtable>{};
	}

	template<typename Impl>
	class object_interface<Impl, ham_net>{
		public:
			Impl *self() noexcept{ return static_cast<Impl*>(this); }
			const Impl *self() const noexcept{ return static_cast<Impl*>(this); }

			const ham_net_socket_vtable *socket_vtable(){ return self()->vtable()->socket_vtable(); }

			bool init(){ return self()->vtable()->init(self()->ptr()); }
			void fini(){ self()->vtable()->fini(self()->ptr()); }
			void loop(f64 dt){ self()->vtable()->loop(self()->ptr(), dt); }
	};

	// socket::ctor(ham_net_peer *ret, ham_str8 query)
}

#endif // __cplusplus

/**
 * @}
 */

#endif // !HAM_NET_OBJECT_H
