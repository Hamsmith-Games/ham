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

void ham_sem_destroy(ham_sem *sem){
	if(!ham_check(sem != NULL)) return;

	const auto allocator = sem->allocator;

	const int res = sem_destroy(&sem->sem);
	if(res != 0){
		ham_logapierrorf("Error in sem_close: %s", strerror(errno));
	}

	ham_allocator_delete(allocator, sem);
}

bool ham_sem_post(ham_sem *sem){
	if(!ham_check(sem != NULL)) return false;

	const int res = sem_post(&sem->sem);
	if(res != 0){
		ham_logapierrorf("Error in sem_post: %s", strerror(errno));
		return false;
	}

	return true;
}

bool ham_sem_wait(ham_sem *sem){
	if(!ham_check(sem != NULL)) return false;

	const int res = sem_wait(&sem->sem);
	if(res != 0){
		ham_logapierrorf("Error in sem_wait: %s", strerror(errno));
		return false;
	}

	return true;
}

ham_i32 ham_sem_try_wait(ham_sem *sem){
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

ham_mutex *ham_mutex_create(){
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

#ifndef NDEBUG
	ptr->mut = PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP;
	pthread_mutexattr_settype(&mut_attr, PTHREAD_MUTEX_ERRORCHECK);
#else
	ptr->mut = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutexattr_settype(&mut_attr, PTHREAD_MUTEX_NORMAL);
#endif

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

void ham_mutex_destroy(ham_mutex *mut){
	if(!ham_check(mut != NULL)) return;

	const auto allocator = mut->allocator;

	const int res = pthread_mutex_destroy(&mut->mut);
	if(res != 0){
		ham_logapierrorf("Error in pthread_mutex_destroy: %s", strerror(res));
	}

	ham_allocator_delete(allocator, mut);
}

bool ham_mutex_lock(ham_mutex *mut){
	if(!ham_check(mut != NULL)) return false;

	const int res = pthread_mutex_lock(&mut->mut);
	if(res != 0){
		ham_logapierrorf("Error in pthread_mutex_lock: %s", strerror(res));
		return false;
	}

	return true;
}

ham_i32 ham_mutex_try_lock(ham_mutex *mut){
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

bool ham_mutex_unlock(ham_mutex *mut){
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

void ham_cond_destroy(ham_cond *cond){
	if(!ham_check(cond != NULL)) return;

	const auto allocator = cond->allocator;

	const int res = pthread_cond_destroy(&cond->cond);
	if(res != 0){
		ham_logapierrorf("Error in pthread_cond_destroy: %s", strerror(res));
	}

	ham_allocator_delete(allocator, cond);
}

bool ham_cond_wait(ham_cond *cond, ham_mutex *mut){
	if(!ham_check(cond != NULL)) return false;

	const int res = pthread_cond_wait(&cond->cond, &mut->mut);
	if(res != 0){
		ham_logapierrorf("Error in pthread_cond_wait: %s", strerror(res));
		return false;
	}

	return true;
}

HAM_C_API_END
