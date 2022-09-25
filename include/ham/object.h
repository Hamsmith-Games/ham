#ifndef HAM_OBJECT_H
#define HAM_OBJECT_H 1

/**
 * @defgroup HAM_OBJECT Object-oriented utilities
 * @ingroup HAM
 * @{
 */

#include "typedefs.h"

#include <stdarg.h>

HAM_C_API_BEGIN

#define HAM_SUPER_NAME _ham_super

typedef struct ham_object         ham_object;
typedef struct ham_object_vtable  ham_object_vtable;
typedef struct ham_object_manager ham_object_manager;

ham_api ham_object_manager *ham_object_manager_create(const ham_object_vtable *vtable);
ham_api void ham_object_manager_destroy(ham_object_manager *manager);

ham_api bool ham_object_manager_contains(const ham_object_manager *manager, const ham_object *obj);

ham_api ham_usize ham_object_manager_block_index(const ham_object_manager *manager, const ham_object *obj);

ham_api ham_object *ham_object_vnew(ham_object_manager *manager, va_list va);

static inline ham_object *ham_object_new(ham_object_manager *manager, ...){
	va_list va;
	va_start(va, manager);
	ham_object *const ret = ham_object_vnew(manager, va);
	va_end(va);
	return ret;
}

ham_api bool ham_object_delete(ham_object_manager *manager, ham_object *obj);

typedef struct ham_object_info{
	const char *type_id;
	ham_usize alignment, size;
} ham_object_info;

struct ham_object_vtable{
	const ham_object_info*(*info)();

	ham_object*(*construct)(ham_object *ptr, va_list va);
	void(*destroy)(ham_object*);
};

struct ham_object{
	const ham_object_vtable *vtable;
};

#define ham_derive(base) base HAM_SUPER_NAME;

//! @cond ignore
#define ham_impl_super_1(derived_ptr) (&(derived_ptr)->HAM_SUPER_NAME)
#define ham_impl_super_2(derived_ptr) (&(derived_ptr)->HAM_SUPER_NAME.HAM_SUPER_NAME)
#define ham_impl_super_3(derived_ptr) (&(derived_ptr)->HAM_SUPER_NAME.HAM_SUPER_NAME.HAM_SUPER_NAME)
#define ham_impl_super_4(derived_ptr) (&(derived_ptr)->HAM_SUPER_NAME HAM_REPEAT(3, .HAM_SUPER_NAME))
#define ham_impl_super_5(derived_ptr) (&(derived_ptr)->HAM_SUPER_NAME HAM_REPEAT(4, .HAM_SUPER_NAME))
#define ham_impl_super_6(derived_ptr) (&(derived_ptr)->HAM_SUPER_NAME HAM_REPEAT(5, .HAM_SUPER_NAME))
#define ham_impl_super_7(derived_ptr) (&(derived_ptr)->HAM_SUPER_NAME HAM_REPEAT(6, .HAM_SUPER_NAME))
#define ham_impl_super_8(derived_ptr) (&(derived_ptr)->HAM_SUPER_NAME HAM_REPEAT(7, .HAM_SUPER_NAME))
//! @endcond

#define ham_super_n(n, derived_ptr) HAM_CONCAT(ham_impl_super_, n)(derived_ptr)

#define ham_super(derived_ptr) ham_super_n(1, derived_ptr)

//! @cond ignore
#define ham_impl_object_vtable_prefix ham_impl_obj_vtable_

#define ham_impl_define_object(obj_depth_, obj_, vtable_depth_, vtable_, ctor_, dtor_, vtable_body_) \
	ham_extern_c ham_public ham_export const ham_object_vtable *HAM_CONCAT(ham_impl_object_vtable_prefix, obj_)(); \
	static const ham_object_info *HAM_CONCAT(ham_impl_obj_info_, obj_)(){ \
		static const ham_object_info ret = (ham_object_info){ \
			.type_id = #obj_, \
			.alignment = alignof(obj_), \
			.size = sizeof(obj_), \
		}; \
		return &ret; \
	} \
	static ham_object *HAM_CONCAT(ham_impl_obj_ctor_, obj_)(ham_object *ptr, va_list va){ \
		ptr->vtable = HAM_CONCAT(ham_impl_obj_vtable_, obj_)(); \
		obj_ *const ret = (ctor_)((obj_*)ptr, va); \
		return ret ? ham_super_n(obj_depth_, ret) : nullptr; \
	} \
	static void HAM_CONCAT(ham_impl_obj_dtor_, obj_)(ham_object *obj){ \
		obj_ *const derived_ptr = (obj_*)obj; \
		(dtor_)(derived_ptr); \
	} \
	const ham_object_vtable *HAM_CONCAT(ham_impl_object_vtable_prefix, obj_)(){\
		static const vtable_ ret = (vtable_){ \
			HAM_REPEAT(vtable_depth_, .HAM_SUPER_NAME) = (ham_object_vtable){ \
				.info = HAM_CONCAT(ham_impl_obj_info_, obj_), \
				.construct = HAM_CONCAT(ham_impl_obj_ctor_, obj_), \
				.destroy = HAM_CONCAT(ham_impl_obj_dtor_, obj_), \
			}, \
			HAM_EAT vtable_body_ \
		}; \
		return ham_super_n(vtable_depth_, &ret); \
	}
//! @endcond

#define ham_define_object_x(obj_depth, obj, vtable_depth, vtable, ctor, dtor, vtable_body) ham_impl_define_object(obj_depth, obj, vtable_depth, vtable, ctor, dtor, vtable_body)

#define ham_define_object(obj, vtable, ctor, dtor, vtable_body) ham_define_object_x(1, obj, 1, vtable, ctor, dtor, vtable_body)

HAM_C_API_END

/**
 * @}
 */

#endif // !HAM_OBJECT_H
