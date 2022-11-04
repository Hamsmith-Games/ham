#include "ham/engine/entity.h"

#include "ham/log.h"

using namespace ham::typedefs;

HAM_C_API_BEGIN

ham_entity_component *ham_entity_component_vcreate(ham_entity *ent, const ham_entity_component_vtable *comp_vptr, ham_u32 nargs, va_list va){
	(void)ent; (void)comp_vptr; (void)nargs; (void)va;
	ham::logapierror("Entity components unimplemented");
	return nullptr;
}

HAM_C_API_END
