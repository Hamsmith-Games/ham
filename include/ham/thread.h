/**
 * The Ham Programming Language
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

#ifndef HAM_THREAD_H
#define HAM_THREAD_H 1

/**
 * @defgroup HAM_THREAD Threads
 * @ingroup HAM
 * @{
 */

#include "typedefs.h"

HAM_C_API_BEGIN

typedef struct ham_thread ham_thread;

typedef ham_uptr(*ham_thread_fn)(void *user);

ham_api ham_thread *ham_thread_create(ham_thread_fn fn, void *user);

ham_api void ham_thread_destroy(ham_thread *thd);

ham_api bool ham_thread_join(ham_thread *thd, ham_uptr *ret);

ham_api bool ham_thread_set_name(ham_thread *thd, ham_str8 name);

HAM_C_API_END

/**
 * @}
 */

#endif // !HAM_THREAD_H
