/*
 * Ham Runtime
 * Copyright (C) 2022 Hamsmith Ltd.
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
// Semaphores
//

struct ham_sem{
	const ham_allocator *allocator;
	sem_t sem;
};

ham_sem *ham_sem_create(ham_u32 initial_val){
	const auto allocator = ham_current_allocator();

	const auto ptr = ham_allocator_new(allocator, ham_sem);
	if(!ptr) return nullptr;

	ptr->allocator = allocator;

	const int res = sem_init(&ptr->sem, 0, initial_val);
	if(res != 0){
		ham_logapierrorf("Error in sem_init: %s", strerror(errno));
		ham_allocator_delete(allocator, ptr);
		return nullptr;
	}

	return ptr;
}

ham_nothrow void ham_sem_destroy(ham_sem *sem){
	if(!ham_check(sem != NULL)) return;

	const auto allocator = sem->allocator;

	const int res = sem_destroy(&sem->sem);
	if(res != 0){
		ham_logapierrorf("Error in sem_close: %s", strerror(errno));
	}

	ham_allocator_delete(allocator, sem);
}

ham_nothrow bool ham_sem_post(ham_sem *sem){
	if(!ham_check(sem != NULL)) return false;

	const int res = sem_post(&sem->sem);
	if(res != 0){
		ham_logapierrorf("Error in sem_post: %s", strerror(errno));
		return false;
	}

	return true;
}

ham_nothrow bool ham_sem_wait(ham_sem *sem){
	if(!ham_check(sem != NULL)) return false;

	const int res = sem_wait(&sem->sem);
	if(res != 0){
		ham_logapierrorf("Error in sem_wait: %s", strerror(errno));
		return false;
	}

	return true;
}

ham_nothrow ham_i32 ham_sem_try_wait(ham_sem *sem){
	if(!ham_check(sem != NULL)) return -1;

	const int res = sem_trywait(&sem->sem);
	if(res != 0){
		if(errno == EAGAIN) return 1;

		ham_logapierrorf("Error in sem_trywait: %s", strerror(errno));
		return -1;
	}

	return 0;
}

//
// Mutually exclusive locks
//

struct ham_mutex{
	const ham_allocator *allocator;
	pthread_mutex_t mut;
};

ham_mutex *ham_mutex_create(ham_mutex_kind kind){
	const auto allocator = ham_current_allocator();

	const auto ptr = ham_allocator_new(allocator, ham_mutex);
	if(!ptr) return nullptr;

	ptr->allocator = allocator;

	pthread_mutexattr_t mut_attr;
	int res = pthread_mutexattr_init(&mut_attr);
	if(res != 0){
		ham_logapierrorf("Error in pthread_mutexattr_init: %s", strerror(res));
		ham_allocator_delete(allocator, ptr);
		return nullptr;
	}

	switch (kind) {
		case HAM_MUTEX_RECURSIVE:{
			ptr->mut = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
			pthread_mutexattr_settype(&mut_attr, PTHREAD_MUTEX_RECURSIVE_NP);
			break;
		}

		case HAM_MUTEX_ERRORCHECK:{
			ptr->mut = PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP;
			pthread_mutexattr_settype(&mut_attr, PTHREAD_MUTEX_ERRORCHECK_NP);
			break;
		}

		default:{
			ham_logapiwarnf("Unrecognized ham_mutex_kind (0x%x) using HAM_MUTEX_NORMAL", kind);
		}

		case HAM_MUTEX_NORMAL:{
			ptr->mut = PTHREAD_MUTEX_INITIALIZER;
			pthread_mutexattr_settype(&mut_attr, PTHREAD_MUTEX_NORMAL);
			break;
		}
	}

	res = pthread_mutex_init(&ptr->mut, &mut_attr);
	if(res != 0){
		ham_logapierrorf("Error in pthread_mutex_init: %s", strerror(res));

		res = pthread_mutexattr_destroy(&mut_attr);
		if(res != 0){
			ham_logapierrorf("Error in pthread_mutexattr_destroy: %s", strerror(res));
		}

		ham_allocator_delete(allocator, ptr);
		return nullptr;
	}

	res = pthread_mutexattr_destroy(&mut_attr);
	if(res != 0){
		ham_logapierrorf("Error in pthread_mutexattr_destroy: %s", strerror(res));
	}

	return ptr;
}

ham_nothrow void ham_mutex_destroy(ham_mutex *mut){
	if(!ham_check(mut != NULL)) return;

	const auto allocator = mut->allocator;

	const int res = pthread_mutex_destroy(&mut->mut);
	if(res != 0){
		ham_logapierrorf("Error in pthread_mutex_destroy: %s", strerror(res));
	}

	ham_allocator_delete(allocator, mut);
}

ham_nothrow bool ham_mutex_lock(ham_mutex *mut){
	if(!ham_check(mut != NULL)) return false;

	const int res = pthread_mutex_lock(&mut->mut);
	if(res != 0){
		ham_logapierrorf("Error in pthread_mutex_lock: %s", strerror(res));
		return false;
	}

	return true;
}

ham_nothrow ham_i32 ham_mutex_try_lock(ham_mutex *mut){
	if(!ham_check(mut != NULL)) return -1;

	const int res = pthread_mutex_trylock(&mut->mut);

	switch(res){
		case 0: return 0;
		case EBUSY: return 1;

		default:{
			ham_logapierrorf("Error in pthread_mutex_trylock: %s", strerror(res));
			return -1;
		}
	}
}

ham_nothrow bool ham_mutex_unlock(ham_mutex *mut){
	if(!ham_check(mut != NULL)) return false;

	const int res = pthread_mutex_unlock(&mut->mut);
	if(res != 0){
		ham_logapierrorf("Error in pthread_mutex_unlock: %s", strerror(res));
		return false;
	}

	return true;
}

//
// Condition variables
//

struct ham_cond{
	const ham_allocator *allocator;
	pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
};

ham_cond *ham_cond_create(){
	const auto allocator = ham_current_allocator();

	const auto ptr = ham_allocator_new(allocator, ham_cond);
	if(!ptr) return nullptr;

	ptr->allocator = allocator;

	const int res = pthread_cond_init(&ptr->cond, nullptr);
	if(res != 0){
		ham_logapierrorf("Error in pthread_cond_init: %s", strerror(res));
		ham_allocator_delete(allocator, ptr);
		return nullptr;
	}

	return ptr;
}

ham_nothrow void ham_cond_destroy(ham_cond *cond){
	if(!ham_check(cond != NULL)) return;

	const auto allocator = cond->allocator;

	const int res = pthread_cond_destroy(&cond->cond);
	if(res != 0){
		ham_logapierrorf("Error in pthread_cond_destroy: %s", strerror(res));
	}

	ham_allocator_delete(allocator, cond);
}

ham_nothrow bool ham_cond_signal(ham_cond *cond){
	if(!ham_check(cond != NULL)) return false;

	const int res = pthread_cond_signal(&cond->cond);
	if(res != 0){
		ham_logapierrorf("Error in pthread_cond_signal: %s", strerror(res));
		return false;
	}

	return true;
}

ham_nothrow bool ham_cond_broadcast(ham_cond *cond){
	if(!ham_check(cond != NULL)) return false;

	const int res = pthread_cond_broadcast(&cond->cond);
	if(res != 0){
		ham_logapierrorf("Error in pthread_cond_broadcast: %s", strerror(res));
		return false;
	}

	return true;
}

ham_nothrow bool ham_cond_wait(ham_cond *cond, ham_mutex *mut){
	if(!ham_check(cond != NULL)) return false;

	const int res = pthread_cond_wait(&cond->cond, &mut->mut);
	if(res != 0){
		ham_logapierrorf("Error in pthread_cond_wait: %s", strerror(res));
		return false;
	}

	return true;
}

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
