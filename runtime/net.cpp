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

	const auto ptr = (ham_net*)obj_vt->ctor((ham_object*)mem, 0, nullptr);
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
		obj_vt->dtor(mem);
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
	vtable->dtor((ham_object*)net);

	ham_dso_close(dso);
}

void ham_net_loop(ham_net *net, ham_f64 dt){
	if(!ham_check(net != NULL)) return;

	const auto vtable = (const ham_net_vtable*)ham_super(net)->vtable;

	vtable->loop(net, dt);
}

bool ham_net_find_peer(ham_net *net, ham_net_peer *ret, ham_str8 query){
	if(
	   !ham_check(net != NULL) ||
	   !ham_check(ret != NULL) ||
	   !ham_check((query.len && query.ptr) || (!query.len && !query.ptr))
	){
		return false;
	}

	const auto vtable = (const ham_net_vtable*)ham_super(net)->vtable;

	return vtable->find_peer(net, ret, query);
}

static inline ham_net_socket *ham_impl_net_socket_construct(ham_net_socket *sock, ...){
	const auto obj_vt = ham_super(sock)->vtable;
	const auto obj_info = obj_vt->info();

	va_list va;
	va_start(va, sock);
	const auto ret = (ham_net_socket*)obj_vt->ctor(ham_super(sock), 2, va);
	va_end(va);

	return ret;
}

ham_net_socket *ham_net_socket_create(ham_net *net, ham_net_peer peer, ham_u16 port){
	if(!ham_check(net != NULL)) return nullptr;

	const auto vtable = ((const ham_net_vtable*)ham_super(net)->vtable)->socket_vtable();
	const auto obj_vt = ham_super(vtable);

	const auto obj_info = obj_vt->info();

	const auto obj = (ham_net_socket*)ham_allocator_alloc(net->allocator, obj_info->alignment, obj_info->size);
	if(!obj){
		ham_logapierrorf("Error allocating memory for socket object \"%s\"", obj_info->type_id);
		return nullptr;
	}

	ham_super(obj)->vtable = obj_vt;
	obj->net = net;
	obj->peer = peer;
	obj->port = port;
	obj->is_listen = ham_net_peer_cmp(peer, HAM_NET_EMPTY_PEER) == 0;

	const auto ret = ham_impl_net_socket_construct(obj, peer, port);
	if(!ret){
		ham_logapierrorf("Error constructing socket object \"%s\"", obj_info->type_id);
		ham_allocator_free(net->allocator, obj);
		return nullptr;
	}

	return ret;
}

void ham_net_socket_destroy(ham_net_socket *socket){
	if(ham_unlikely(socket == NULL)) return;

	const auto obj_vt = ham_super(socket)->vtable;
	const auto allocator = socket->net->allocator;

	obj_vt->dtor(ham_super(socket));
	ham_allocator_free(allocator, socket);
}

HAM_C_API_END
