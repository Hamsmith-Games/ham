#ifndef HAM_RENDERER_VTABLE_H
#define HAM_RENDERER_VTABLE_H 1

#include "renderer.h"
#include "object.h"
#include "memory.h"
#include "plugin.h"

struct ham_renderer{
	ham_derive(ham_object)

	const ham_allocator *allocator;
	ham_dso_handle dso;
	ham_plugin *plugin;
};

typedef struct ham_renderer_vtable{
	ham_derive(ham_object_vtable)

	bool(*init)(ham_renderer *renderer);
	void(*fini)(ham_renderer *renderer);
	void(*loop)(ham_renderer *renderer, ham_f64 dt);
} ham_renderer_vtable;

#endif // !HAM_RENDERER_VTABLE_H
