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

struct ham_net{
	ham_derive(ham_object)

	const ham_allocator *allocator;
	ham_dso_handle dso;
};

typedef void(*ham_net_loop_fn)(ham_net *net, ham_f64 dt);

struct ham_net_vtable{
	ham_derive(ham_object_vtable)

	bool(*init)(ham_net *net);
	void(*fini)(ham_net *net);
	void(*loop)(ham_net *net, ham_f64 dt);
};

HAM_C_API_END

/**
 * @}
 */

#endif // !HAM_NET_OBJECT_H
