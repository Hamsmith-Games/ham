/*
 * Ham Runtime
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

#include "ham/async.h"
#include "ham/memory.h"
#include "ham/check.h"

#include <pthread.h>
#include <semaphore.h>

HAM_C_API_BEGIN

//
// Threads
//

struct ham_thread{
	const ham_allocator *allocator;
	pthread_t pthd;
	ham_thread_fn fn;
	void *user;
};

static void *ham_impl_thread_routine(void *data){
	const auto thd = reinterpret_cast<ham_thread*>(data);

	const ham_uptr res = thd->fn(thd->user);

	void *ret;
	memcpy(&ret, &res, sizeof(void*));

	return ret;
}

ham_thread *ham_thread_create(ham_thread_fn fn, void *user){
	if(!fn) return nullptr;

	const ham::allocator<ham_thread> allocator;

	const auto mem = allocator.allocate(1);
	if(!mem) return nullptr;

	const auto ptr = allocator.construct(mem);
	if(!ptr){
		allocator.deallocate(mem);
		return nullptr;
	}

	ptr->fn = fn;
	ptr->user = user;

	int res = pthread_create(&ptr->pthd, nullptr, ham_impl_thread_routine, ptr);
	if(res != 0){
		// TODO: signal error
		allocator.destroy(ptr);
		allocator.deallocate(mem);
		return nullptr;
	}

	return ptr;
}

ham_nothrow void ham_thread_destroy(ham_thread *thd){
	if(!thd) return;

	const ham::allocator<ham_thread> allocator = thd->allocator;

	int res = pthread_join(thd->pthd, nullptr);
	if(res != 0){
		// TODO: signal error
		(void)res;
	}

	allocator.destroy(thd);
	allocator.deallocate(thd);
}

ham_nothrow bool ham_thread_join(ham_thread *thd, ham_uptr *ret){
	if(!thd) return false;

	void *result;

	const int res = pthread_join(thd->pthd, &result);
	if(res != 0){
		// TODO: signal error
		return false;
	}

	if(ret) memcpy(ret, &result, sizeof(void*));

	return true;
}

ham_nothrow bool ham_thread_set_name(ham_thread *thd, ham_str8 name){
	if(
	   !thd ||
	   !name.ptr ||
	   !name.len ||
	   name.len >= HAM_NAME_BUFFER_SIZE
	){
		return false;
	}

	ham_name_buffer_utf8 name_buf;
	memcpy(name_buf, name.ptr, name.len);
	name_buf[name.len] = '\0';

	const int res = pthread_setname_np(thd->pthd, name_buf);
	if(res != 0){
		// TODO: signal error
		return false;
	}

	return true;
}

HAM_C_API_END
