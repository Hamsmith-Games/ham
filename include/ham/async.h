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

#ifndef HAM_ASYNC_H
#define HAM_ASYNC_H 1

/**
 * @defgroup HAM_ASYNC Asynchronous utilities
 * @ingroup HAM
 * @{
 */

#include "typedefs.h"

HAM_C_API_BEGIN

/**
 * @defgroup HAM_ASYNC_SEM Semaphores
 * @{
 */

typedef struct ham_sem ham_sem;

ham_api ham_sem *ham_sem_create(ham_u32 initial_val);

ham_api void ham_sem_destroy(ham_sem *sem);

ham_api bool ham_sem_post(ham_sem *sem);

ham_api bool ham_sem_wait(ham_sem *sem);

/**
 * Try to wait for a semaphore to post.
 * @param sem semaphore to try to wait on
 * @returns `1` if the semphore would block, `0` if the semaphore was decremented or `-1` on error
 */
ham_api ham_i32 ham_sem_try_wait(ham_sem *sem);

/**
 * @}
 */

/**
 * @defgroup HAM_ASYNC_MUTEX Mutually exclusive locks
 * @{
 */

typedef struct ham_mutex ham_mutex;

ham_api ham_mutex *ham_mutex_create();

ham_api void ham_mutex_destroy(ham_mutex *mut);

ham_api bool ham_mutex_lock(ham_mutex *mut);

/**
 * Try to lock a mutex.
 * @param mut mutex to try lock
 * @returns `1` if the mutex would block, `0` if the mutex was locked or `-1` on error
 */
ham_api ham_i32 ham_mutex_trylock(ham_mutex *mut);

ham_api bool ham_mutex_unlock(ham_mutex *mut);

/**
 * @}
 */

/**
 * @defgroup HAM_ASYNC_COND Condition variables
 * @{
 */

typedef struct ham_cond ham_cond;

ham_api ham_cond *ham_cond_create();

ham_api void ham_cond_destroy(ham_cond *cond);

ham_api bool ham_cond_signal(ham_cond *cond);
ham_api bool ham_cond_broadcast(ham_cond *cond);

ham_api bool ham_cond_wait(ham_cond *cond, ham_mutex *mut);

/**
 * @}
 */

HAM_C_API_END

/**
 * @}
 */

#endif // !HAM_ASYNC_H
