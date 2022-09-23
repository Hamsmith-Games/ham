#include "ham/net-vtable.h"

#include "ham/check.h"

HAM_C_API_BEGIN

extern const ham_net_vtable *HAM_IMPL_PLUGIN_VTABLE_NAME(ham_net_vtable)();

ham_net_context *ham_net_context_create(const char *plugin_id){
	if(!plugin_id) plugin_id = HAM_NET_DEFAULT_PLUGIN_NAME;

	ham_plugin *plugin = nullptr;
	ham_dll_handle dll = nullptr;
	const ham_plugin_vtable *plugin_vtable = nullptr;

	if(!ham_plugin_find(plugin_id, HAM_EMPTY_STR8, &plugin, &dll, &plugin_vtable)){
		ham_logapierrorf("Error finding net plugin with id: %s", plugin_id);
		return nullptr;
	}

	if(plugin_vtable->category() != ham::str8("net")){
		const auto plugin_name = plugin_vtable->name();
		ham_logapierrorf("Plugin is not a networking plugin: %.*s", (int)plugin_name.len, plugin_name.ptr);
		ham_plugin_unload(plugin);
		ham_dll_close(dll);
	}

	if(!plugin_vtable->on_load()){
		const auto plugin_name = plugin_vtable->name();
		ham_logapierrorf("Error loading plugin: %.*s", (int)plugin_name.len, plugin_name.ptr);
		ham_plugin_unload(plugin);
		ham_dll_close(dll);
	}

	const auto vtable = (const ham_net_vtable*)plugin_vtable;

	const auto allocator = ham_current_allocator();

	const auto ptr = vtable->context_alloc(allocator);
	if(!ptr){
		const auto plugin_name = plugin_vtable->name();
		ham_logapierrorf("Failed to allocate context for plugin: %.*s", (int)plugin_name.len, plugin_name.ptr);
		plugin_vtable->on_unload();
		ham_plugin_unload(plugin);
		ham_dll_close(dll);
	}

	ptr->allocator = allocator;
	ptr->dll = dll;
	ptr->vtable = vtable;

	if(!vtable->context_init(ptr)){
		const auto plugin_name = plugin_vtable->name();
		ham_logapierrorf("Failed to initialize context for plugin: %.*s", (int)plugin_name.len, plugin_name.ptr);
		vtable->context_free(ptr);
		plugin_vtable->on_unload();
		ham_plugin_unload(plugin);
		ham_dll_close(dll);
	}

	return ptr;
}

void ham_net_context_destroy(ham_net_context *net){
	if(ham_unlikely(!net)) return;

	const auto vtable = net->vtable;
	const auto dll = net->dll;

	vtable->context_finish(net);
	vtable->context_free(net);

	ham_dll_close(dll);;
}

void ham_net_context_loop(ham_net_context *net, ham_f64 dt){
	if(ham_unlikely(!net)) return;

	const auto vtable = net->vtable;

	vtable->context_loop(net, dt);
}

HAM_C_API_END
