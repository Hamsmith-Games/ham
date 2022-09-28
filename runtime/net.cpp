#include "ham/net-object.h"

#include "ham/plugin.h"
#include "ham/memory.h"
#include "ham/check.h"

HAM_C_API_BEGIN

ham_net *ham_net_create(const char *plugin_id, const char *obj_id){
	if(!ham_check(obj_id != NULL)) return nullptr;

	if(!plugin_id) plugin_id = HAM_NET_DEFAULT_PLUGIN_NAME;

	ham_plugin *plugin = nullptr;
	ham_dso_handle dll = nullptr;

	if(!ham_plugin_find(plugin_id, HAM_EMPTY_STR8, &plugin, &dll)){
		ham_logapierrorf("Error finding net plugin with id: %s", plugin_id);
		return nullptr;
	}

	if(ham_plugin_category(plugin) != ham::str8(HAM_NET_PLUGIN_CATEGORY)){
		const auto plugin_name = ham_plugin_name(plugin);
		ham_logapiwarnf("Plugin is not a networking plugin: %.*s", (int)plugin_name.len, plugin_name.ptr);
	}

	const ham_object_vtable *obj_vt = ham_plugin_object(plugin, ham::str8(obj_id));
	if(!obj_vt){
		const auto plugin_name = ham_plugin_name(plugin);
		ham_logapierrorf("Could not get object '%s' from plugin '%s'", obj_id, plugin_name.ptr);
		ham_plugin_unload(plugin);
		ham_dso_close(dll);
		return nullptr;
	}

	if(!ham_plugin_init(plugin)){
		const auto plugin_name = ham_plugin_name(plugin);
		ham_logapierrorf("Error loading plugin: %.*s", (int)plugin_name.len, plugin_name.ptr);
		ham_plugin_unload(plugin);
		ham_dso_close(dll);
		return nullptr;
	}

	const auto obj_info = obj_vt->info();

	const auto allocator = ham_current_allocator();

	const auto mem = (ham_object*)ham_allocator_alloc(allocator, obj_info->alignment, obj_info->size);
	if(!mem){
		ham_logapierrorf("Failed to allocate memory for object '%s'", obj_info->type_id);
		ham_plugin_unload(plugin);
		ham_dso_close(dll);
	}

	mem->vtable = obj_vt;

	const auto ptr = (ham_net*)obj_vt->construct((ham_object*)mem, 0, nullptr);
	if(!ptr){
		ham_logapierrorf("Failed to construct object '%s'", obj_info->type_id);
		ham_allocator_free(allocator, mem);
		ham_plugin_unload(plugin);
		ham_dso_close(dll);
	}

	ptr->allocator = allocator;
	ptr->dso = dll;

	const auto net_vt = (const ham_net_vtable*)obj_vt;

	if(!net_vt->init(ptr)){
		ham_logapierrorf("Failed to initialize net object '%s'", obj_info->type_id);
		obj_vt->destroy(mem);
		ham_allocator_free(allocator, mem);
		ham_plugin_unload(plugin);
		ham_dso_close(dll);
		return nullptr;
	}

	return ptr;
}

void ham_net_destroy(ham_net *net){
	if(ham_unlikely(!net)) return;

	const auto vtable = ham_super(net)->vtable;
	const auto dso = net->dso;

	((const ham_net_vtable*)vtable)->fini(net);
	vtable->destroy((ham_object*)net);

	ham_dso_close(dso);
}

void ham_net_loop(ham_net *net, ham_f64 dt){
	if(ham_unlikely(!net)) return;

	const auto vtable = (const ham_net_vtable*)ham_super(net)->vtable;

	vtable->loop(net, dt);
}

HAM_C_API_END
