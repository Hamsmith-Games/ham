#include "ham/object.h"
#include "ham/check.h"

#include "tests.hpp"

#include <iostream>

struct ham_object_test{
	ham_derive(ham_object)
	int test_val;
};

struct ham_object_test_vtable{
	ham_derive(ham_object_vtable)

	int(*foo)(ham_object_test*);
	int(*bar)(const ham_object_test*);
};

static ham_object_test *ham_object_test_ctor(ham_object_test *mem, ham_u32 nargs, va_list va){
	(void)nargs; (void)va;
	return new(mem) ham_object_test;
}

static void ham_object_test_dtor(ham_object_test *self){
	std::destroy_at(self);
}

ham_define_object(
	ham_object_test,
	ham_object_test_vtable,
	ham_object_test_ctor,
	ham_object_test_dtor,
	(
		.foo = +[](ham_object_test *obj) -> int{ return obj->test_val += 2; },
		.bar = +[](const ham_object_test *obj) -> int{ return obj->test_val * 4; },
	)
);

bool ham_test_object(){
	std::cout << "Running object test... " << std::flush;

	const auto obj_vtable = ham_impl_obj_vtable_ham_object_test();
	const auto obj_info = obj_vtable->info();

	if(
	   !ham_check(obj_info->size == sizeof(ham_object_test)) ||
	   !ham_check(obj_info->alignment == alignof(ham_object_test))
	){
		std::cout << HAM_TEST_FAILED_STR "\n" << std::flush;
		std::cerr <<
			"    Object info provides incorrect information.\n"
			"        Given:    " << obj_info->size << " " << obj_info->alignment << "\n" <<
			"        Expected: " << sizeof(ham_object_test) << " " << alignof(ham_object_test) << '\n';
		return false;
	}

	const auto obj_man = ham_object_manager_create(obj_vtable);
	if(!obj_man){
		std::cout << HAM_TEST_FAILED_STR "\n" << std::flush;
		std::cerr << "    Error in ham_object_manager_create\n";
		return false;
	}

	ham_object *objs[4096];

	for(ham_usize i = 0; i < std::size(objs); i++){
		objs[i] = ham_object_new(obj_man);

		const auto obj = objs[i];

		if(!obj){
			std::cout << HAM_TEST_FAILED_STR "\n" << std::flush;
			std::cerr << "    Failed to create " << i << "'th object instance.\n";
			ham_object_manager_destroy(obj_man);
			return false;
		}
		else if(obj->vtable != obj_vtable){
			std::cout << HAM_TEST_FAILED_STR "\n" << std::flush;
			std::cerr << "    " << i << "'th object has bad vtable.\n";
			ham_object_manager_destroy(obj_man);
			return false;
		}

		((ham_object_test*)obj)->test_val = i + 1;
	}

	for(ham_usize i = 0; i < std::size(objs); i++){
		const auto test_obj = (ham_object_test*)objs[i];
		const auto test_vt  = (ham_object_test_vtable*)objs[i]->vtable;
		if(test_vt->foo(test_obj) != i + 3){
			std::cout << HAM_TEST_FAILED_STR "\n" << std::flush;
			std::cerr << "    Function call bad result from object " << i << " `foo(obj)`\n"
						 "        Given:    " << test_obj->test_val << "\n"
						 "        Expected: " << i + 3 << '\n';
			ham_object_manager_destroy(obj_man);
			return false;
		}

		const auto bar_res = test_vt->bar(test_obj);
		if(bar_res != (i + 3) * 4){
			std::cout << HAM_TEST_FAILED_STR "\n" << std::flush;
			std::cerr << "    Function call bad result from object " << i << " `bar(obj)`\n"
						 "        Given:    " << bar_res << "\n"
						 "        Expected: " << (i + 3) * 4 << '\n';
			ham_object_manager_destroy(obj_man);
			return false;
		}
	}

	for(ham_usize i = std::size(objs); i >= 1; i--){
		const auto idx = i - 1;
		if(!ham_object_delete(obj_man, objs[idx])){
			std::cout << HAM_TEST_FAILED_STR "\n" << std::flush;
			std::cerr << "    Failed to destroy " << i << "'th object instance.\n";
			ham_object_manager_destroy(obj_man);
			return false;
		}
	}

	std::cout << HAM_TEST_PASSED_STR "\n" << std::flush;

	return true;
}
