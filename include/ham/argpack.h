#ifndef HAM_ARGPACK_H
#define HAM_ARGPACK_H 1

#include "typedefs.h"
#include "memory.h"

HAM_C_API_BEGIN

typedef struct ham_argpack_args ham_argpack_args;

//! @cond ignore

typedef struct ham_impl_argpack_arg{
	const char *type_name;
	void *ptr;
} ham_impl_argpack_arg;

struct ham_argpack_args{
	const ham_allocator *allocator;
	ham_u32 nargs;
	ham_impl_argpack_arg *args;
};

static inline bool ham_impl_pack_args(ham_argpack_args *ret, ham_u32 nargs, ...){
	const ham_allocator *const allocator = ham_current_allocator();

	ham_impl_argpack_arg *arg_ptrs = nullptr;
	if(nargs > 0){
		arg_ptrs = (ham_impl_argpack_arg*)ham_allocator_alloc(allocator, alignof(ham_impl_argpack_arg), sizeof(ham_impl_argpack_arg) * nargs);

		va_list va;
		va_start(va, nargs);

		for(ham_u32 i = 0; i < nargs; i++){
			// TODO: get type names for validation
			ham_impl_argpack_arg *arg_ptr = arg_ptrs + i;
			arg_ptr->type_name = ham_null;
			arg_ptr->ptr = va_arg(va, void*);
		}

		va_end(va);
	}

	ret->allocator = allocator;
	ret->nargs = nargs;
	ret->args = arg_ptrs;
	return true;
}

//! @endcond

//#define ham_pack_args(ret, ...) ham_impl_pack_args(ret, HAM_NARGS(__VA_ARGS__) __VA_OPT__(,) __VA_ARGS__)

HAM_C_API_END

#endif // HAM_ARGPACK_H
