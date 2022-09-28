#include "ham/renderer-object.h"
#include "ham/check.h"

HAM_C_API_BEGIN

ham_renderer *ham_renderer_vcreate(const char *plugin_id, const char *object_id, ham_usize nargs, va_list va){
	if(!ham_check(object_id != NULL)) return nullptr;

	ham_plugin *plugin = nullptr;
	ham_dso_handle dso = nullptr;

	if(!ham_plugin_find(plugin_id, HAM_EMPTY_STR8, &plugin, &dso)){
		ham_logapierrorf("Could not find plugin by id '%s'", plugin_id);
		return nullptr;
	}

	const auto obj_vtable = ham_plugin_object(plugin, ham::str8(object_id));
	if(!obj_vtable){
		ham_logapierrorf("Could not find object by id '%s'", object_id);
		ham_plugin_unload(plugin);
		ham_dso_close(dso);
		return nullptr;
	}

	const auto obj_info = obj_vtable->info();

	const auto allocator = ham_current_allocator();

	const auto obj = (ham_object*)ham_allocator_alloc(allocator, obj_info->alignment, obj_info->size);
	if(!obj){
		ham_logapierrorf("Error allocating memory for renderer object '%s'", object_id);
		ham_plugin_unload(plugin);
		ham_dso_close(dso);
		return nullptr;
	}

	obj->vtable = obj_vtable;

	if(!ham_plugin_init(plugin)){
		ham_logapierrorf("Error initializing plugin '%s'", ham_plugin_name(plugin).ptr);
		ham_allocator_free(allocator, obj);
		ham_plugin_unload(plugin);
		ham_dso_close(dso);
		return nullptr;
	}

	const auto ret = (ham_renderer*)obj_vtable->construct(obj, nargs, va);
	if(!ret){
		ham_logapierrorf("Failed to construct renderer object '%s'", object_id);
		ham_allocator_free(allocator, obj);
		ham_plugin_unload(plugin);
		ham_dso_close(dso);
		return nullptr;
	}

	ret->allocator = allocator;
	ret->dso = dso;
	ret->plugin = plugin;

	const auto renderer_vt = (const ham_renderer_vtable*)obj_vtable;
	if(!renderer_vt->init(ret)){
		ham_logapierrorf("Failed to initialize renderer object '%s'", object_id);
		obj_vtable->destroy(obj);
		ham_allocator_free(allocator, obj);
		ham_plugin_unload(plugin);
		ham_dso_close(dso);
		return nullptr;
	}

	return ret;
}

void ham_renderer_destroy(ham_renderer *renderer){
	if(ham_unlikely(!renderer)) return;

	const auto allocator = renderer->allocator;
	const auto obj_vt = ham_super(renderer)->vtable;
	const auto renderer_vt = (const ham_renderer_vtable*)obj_vt;

	const auto plugin = renderer->plugin;
	const auto dso = renderer->dso;

	renderer_vt->fini(renderer);
	obj_vt->destroy(ham_super(renderer));

	ham_allocator_free(allocator, renderer);

	ham_plugin_unload(plugin);
	ham_dso_close(dso);
}

void ham_renderer_loop(ham_renderer *renderer, ham_f64 dt){
	if(ham_unlikely(!renderer)) return;

	const auto renderer_vt = (const ham_renderer_vtable*)ham_super(renderer)->vtable;

	renderer_vt->loop(renderer, dt);
}

HAM_C_API_END
