#ifndef HAM_NET_OBJECT_H
#define HAM_NET_OBJECT_H 1

/**
 * @defgroup HAM_NET_VTABLE Networking plugin vtables
 * @ingroup HAM_NET
 * @{
 */

#include "net.h"

#include "ham/dso.h"
#include "ham/object.h"
#include "ham/memory.h"

HAM_C_API_BEGIN

typedef struct ham_net_vtable ham_net_vtable;

struct ham_net{
	ham_derive(ham_object)

	const ham_allocator *allocator;
	ham_dso_handle dso;
};

typedef void(*ham_net_loop_fn)(ham_net *net, ham_f64 dt);

struct ham_net_vtable{
	ham_derive(ham_object_vtable)

	bool(*init)(ham_net *net);
	void(*fini)(ham_net *net);
	void(*loop)(ham_net *net, ham_f64 dt);
};

HAM_C_API_END

/**
 * @}
 */

#endif // !HAM_NET_OBJECT_H
