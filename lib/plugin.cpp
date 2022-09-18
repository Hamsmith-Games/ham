#include "ham/plugin.h"
#include "ham/memory.h"

#ifdef _WIN32
#	error "Win32 support currently partial"
#endif

#include <dlfcn.h>

HAM_C_API_BEGIN

struct ham_plugin{
	const ham_allocator *allocator = nullptr;
	void *handle = nullptr;
};

ham_plugin *ham_plugin_load(ham_str8 path){
	const ham::allocator<ham_plugin> allocator;

	const auto mem = allocator.allocate(1);
	if(!mem) return nullptr;

	const auto ptr = allocator.construct(mem);
	if(!ptr){
		allocator.deallocate(mem);
		return nullptr;
	}

	ptr->allocator = allocator;

	if(!path.ptr || !path.len){
		ptr->handle = dlopen(nullptr, RTLD_LAZY);
	}
	else{
		if(path.len >= HAM_PATH_BUFFER_SIZE){
			allocator.destroy(ptr);
			allocator.deallocate(mem);
			return nullptr;
		}

		ham_path_buffer_utf8 path_buf;
		memcpy(path_buf, path.ptr, path.len);
		path_buf[path.len] = '\0';

		ptr->handle = dlopen(nullptr, RTLD_LAZY);
	}

	if(!ptr->handle){
		// TODO: signal using dlerror()
		allocator.destroy(ptr);
		allocator.deallocate(mem);
		return nullptr;
	}

	return ptr;
}

void ham_plugin_unload(ham_plugin *plugin){
	if(!plugin) return;

	const ham::allocator<ham_plugin> allocator;

	if(dlclose(plugin->handle) != 0){
		// TODO: signal error
	}

	allocator.destroy(plugin);
	allocator.deallocate(plugin);
}

void *ham_plugin_symbol(const ham_plugin *plugin, ham_str8 name){
	if(
	   !plugin ||
	   !name.ptr || !name.len ||
	   name.len >= HAM_NAME_BUFFER_SIZE
	) return nullptr;

	ham_name_buffer_utf8 name_buf;
	memcpy(name_buf, name.ptr, name.len);
	name_buf[name.len] = '\0';

	return dlsym(plugin->handle, name_buf);
}

HAM_C_API_END
