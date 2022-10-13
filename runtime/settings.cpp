#include "ham/settings.h"

#include "robin_hood.h"

HAM_C_API_BEGIN

struct ham_settings{
	const ham_allocator *allocator;
	robin_hood::unordered_node_map<ham::str8, ham_settings_value> vals;
};

ham_settings *ham_settings_create(ham_usize nvals, const ham_settings_value *vals){
	const auto allocator = ham_current_allocator();

	const auto ptr = ham_allocator_new(allocator, ham_settings);

	ptr->allocator = allocator;



	return ptr;
}

void ham_settings_destroy(ham_settings *settings){
	if(ham_unlikely(!settings)) return;

	const auto allocator = settings->allocator;

	ham_allocator_delete(allocator, settings);
}

HAM_C_API_END
