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

typedef struct ham_net ham_net;

ham_api ham_net *ham_net_create(const char *plugin_id, const char *obj_id);

ham_api void ham_net_destroy(ham_net *net);

ham_api void ham_net_loop(ham_net *net, ham_f64 dt);

HAM_C_API_END

/**
 * @}
 */

#endif // !HAM_NET_H
