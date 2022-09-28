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

struct ham_shader{
	ham_derive(ham_object)

	ham_renderer *r;
	ham_shader_kind kind;
};

typedef struct ham_shader_vtable{
	ham_derive(ham_object_vtable)
} ham_shader_vtable;

#endif // !HAM_RENDERER_VTABLE_H
