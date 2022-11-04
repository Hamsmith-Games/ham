/*
 * Ham Runtime
 * Copyright (C) 2022 Hamsmith Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "ham/plugin.h"
#include "ham/memory.h"
#include "ham/check.h"
#include "ham/fs.h"
#include "ham/std_vector.hpp"

#include <dirent.h>

#include <mutex>

using namespace ham::typedefs;

HAM_C_API_BEGIN

struct ham_plugin{
	const ham_allocator *allocator = nullptr;
	ham_path_buffer_utf8 dir_path = { 0 };
	const ham_plugin_vtable *plugin_vtable = nullptr;
	ham::std_vector<const ham_object_vtable*> object_vtables;
	std::mutex mut;
	bool init_flag = false;
};

ham_str8 ham_plugin_default_path(){
	const char *path = getenv("HAM_PLUGIN_PATH");
	if(!path){
		static thread_local ham_path_buffer_utf8 cwd_buf;
		path = getcwd(cwd_buf, sizeof(cwd_buf));
	}

	return (ham_str8){ path, strlen(path) };
}

bool ham_plugin_find(const char *id, ham_str8 path, ham_plugin **plugin_ret, ham_dso_handle *dso_ret){
	if(
		!ham_check(id != NULL) ||
		!ham_check(plugin_ret != NULL) ||
		!ham_check(dso_ret != NULL) ||
		!ham_check(!path.len || path.ptr)
	){
		return false;
	}

	//ham_path_buffer_utf8 cwd_buf;

	if(!path.len){
		path = ham_plugin_default_path();
		if(!path.len || !path.ptr){
			ham_logapierrorf("Error getting default plugin path");
			return false;
		}
	}

	constexpr ham::str8 plugin_mime = HAM_PLATFORM_DSO_MIME;
	constexpr ham::str8 minimum_subpath = "/libx" HAM_PLATFORM_DSO_EXT;

	if((path.len + minimum_subpath.len()) >= HAM_PATH_BUFFER_SIZE){
		ham_logapierrorf("Dir path too long: %zu, max %zu", path.len, HAM_PATH_BUFFER_SIZE - (minimum_subpath.len() + 1));
		return false;
	}

	ham_path_buffer_utf8 path_buf;
	memcpy(path_buf, path.ptr, path.len);
	path_buf[path.len] = '\0';

	DIR *plugin_dir = opendir(path_buf);
	if(!plugin_dir){
		ham_logapierrorf("Error in opendir: %s", strerror(errno));
		return false;
	}

	path_buf[path.len] = '/';

	char *plugin_name_subpath = path_buf + path.len + 1;

	struct plugin_iter_data{
		const char *id = nullptr;
		const char *category = nullptr;
		const ham_plugin_vtable *vtable = nullptr;
	};

	plugin_iter_data data;
	data.id = id;

	for(const struct dirent *plugin_dirent = readdir(plugin_dir); plugin_dirent != nullptr; plugin_dirent = readdir(plugin_dir)){
		if(plugin_dirent->d_type != DT_REG){
			continue;
		}

		ham::str8 plugin_name = plugin_dirent->d_name;

		if(path.len + plugin_name.len() + 1 >= HAM_PATH_BUFFER_SIZE){
			ham_logapiwarnf("Plugin path too long: %zu, max %d", path.len + plugin_name.len() + 1, HAM_PATH_BUFFER_SIZE-1);
			continue;
		}

		memcpy(plugin_name_subpath, plugin_name.ptr(), plugin_name.len());
		plugin_name_subpath[plugin_name.len()] = '\0';

		ham_file_info file_info;
		if(!ham_path_file_info_utf8(ham::str8((const char*)path_buf), &file_info)){
			ham_logapiwarnf("Could not get file info: %s", path_buf);
			continue;
		}
		else if(ham::str8(file_info.mime).substr(0, plugin_mime.len()) != plugin_mime){
			ham_logapiwarnf("Non-plugin found in plugins folder: %s", path_buf);
			continue;
		}

		const ham_dso_handle plugin_dso = ham_dso_open_c(path_buf, HAM_DSO_GLOBAL | HAM_DSO_NOW);
		if(!plugin_dso){
			ham_logapiwarnf("Failed to open plugin: %s", path_buf);
			continue;
		}

		ham_plugin *const plugin = ham_plugin_load(plugin_dso, id);
		if(!plugin){
			ham_dso_close(plugin_dso);
			continue;
		}

		*plugin_ret = plugin;
		*dso_ret = plugin_dso;
		closedir(plugin_dir);
		return true;
	}

	closedir(plugin_dir);
	return false;
}

ham_plugin *ham_plugin_load(ham_dso_handle dso, const char *plugin_id){
	if(!dso){
		dso = ham_dso_open_c(nullptr, 0);
		if(!dso){
			ham_logapierrorf("Error opening main executable as shared object");
			return nullptr;
		}
	}

	const ham_allocator *allocator = ham_current_allocator();

	ham::std_vector<const ham_plugin_vtable*> plug_vtables(allocator);

	struct iter_data{
		const char *req_id;
		const ham_plugin_vtable *plug_vtable = nullptr;
		ham::std_vector<const ham_object_vtable*> obj_vtables;
	} data;

	data.req_id = plugin_id;

	ham_usize total_num_syms = ham_dso_iterate_symbols(
		dso,
		[](ham_dso_handle dso, ham_str8 sym_name, void *user){
			const auto data = reinterpret_cast<iter_data*>(user);

			constexpr ham::str8 obj_vtable_prefix = HAM_STRINGIFY(ham_object_vptr_prefix);
			constexpr ham::str8 vtable_prefix = HAM_STRINGIFY(HAM_IMPL_PLUGIN_VTABLE_NAME_PREFIX);

			const auto sym_str = ham::str8(sym_name);

			if(sym_str.len() > obj_vtable_prefix.len() && strncmp(sym_str.ptr(), obj_vtable_prefix.ptr(), obj_vtable_prefix.len()) == 0){
				using obj_vtable_fn = const ham_object_vtable*(*)();

				const auto obj_vtable_fptr = reinterpret_cast<obj_vtable_fn>(ham_dso_symbol_c(dso, sym_name.ptr));
				if(!obj_vtable_fptr){
					ham_logwarnf("ham_plugin_load", "Failed to load object vtable: %s", sym_name.ptr);
					return true;
				}

				const auto obj_vtable = obj_vtable_fptr();
				if(!obj_vtable){
					ham_logwarnf("ham_plugin_load", "Bad object vtable: %s", sym_name.ptr);
					return true;
				}

				data->obj_vtables.emplace_back(obj_vtable);
			}
			else if(!data->plug_vtable && sym_str.len() > vtable_prefix.len() && strncmp(sym_str.ptr(), vtable_prefix.ptr(), vtable_prefix.len()) == 0){
				using plug_vtable_fn = const ham_plugin_vtable*(*)();

				const auto plug_vtable_fptr = reinterpret_cast<plug_vtable_fn>(ham_dso_symbol_c(dso, sym_name.ptr));
				if(!plug_vtable_fptr){
					ham_logwarnf("ham_plugin_load", "Failed to load plugin vtable: %s", sym_name.ptr);
					return true;
				}

				const auto plug_vtable = plug_vtable_fptr();
				if(!plug_vtable){
					ham_logwarnf("ham_plugin_load", "Bad plugin vtable: %s", sym_name.ptr);
					return true;
				}

				if(strcmp(plug_vtable->name().ptr, data->req_id) == 0){
					ham_logdebugf("ham_plugin_load", "Found plugin vtable: %s", sym_name.ptr);
					data->plug_vtable = plug_vtable;
				}
			}

			return true;
		},
		&data
	);

	if(!data.plug_vtable){
		ham_logapiwarnf("Failed to find plugin by id: %s", plugin_id);
		return nullptr;
	}
	else if(total_num_syms == (usize)-1){
		ham_logapierrorf("Failed to iterate dll symbols");
		return nullptr;
	}

	const auto ptr = ham_allocator_new(allocator, ham_plugin);
	if(!ptr){
		ham_logapierrorf("Error allocating ham_plugin");
		return nullptr;
	}

	ptr->allocator = allocator;
	ptr->plugin_vtable = data.plug_vtable;
	ptr->object_vtables = std::move(data.obj_vtables);

	if(!ham_dso_path(dso, sizeof(ptr->dir_path), ptr->dir_path)){
		ham_logapiwarnf("Failed to find DSO path");
	}
	else{
		const auto slash_idx = ham::str8((const char*)ptr->dir_path).rfind("/");
		if(slash_idx != ham::str8::npos){
			ptr->dir_path[slash_idx] = '\0';
		}
		else{
			ptr->dir_path[0] = '\0';
		}
	}

	return ptr;
}

void ham_plugin_unload(ham_plugin *plugin){
	if(ham_unlikely(!plugin)) return;

	std::scoped_lock lock(plugin->mut);

	if(plugin->init_flag){
		plugin->plugin_vtable->fini();
		plugin->init_flag = false;
	}

	const ham_allocator *allocator = plugin->allocator;

	ham_allocator_delete(allocator, plugin);
}

ham_str8    ham_plugin_dir(const ham_plugin *plugin){ return ham_likely(plugin != NULL) ? (ham_str8)ham::str8((const char*)plugin->dir_path) : HAM_EMPTY_STR8; }
ham_uuid    ham_plugin_uuid(const ham_plugin *plugin){ return ham_likely(plugin != NULL) ? plugin->plugin_vtable->uuid() : (ham_uuid){ .u64s = { (ham_u64)-1, (ham_u64)-1 } }; }
ham_str8    ham_plugin_name(const ham_plugin *plugin){ return ham_likely(plugin != NULL) ? plugin->plugin_vtable->name() : HAM_EMPTY_STR8; }
ham_version ham_plugin_version(const ham_plugin *plugin){ return ham_likely(plugin != NULL) ? plugin->plugin_vtable->version() : (ham_version){ .major = (ham_u16)-1, .minor = (ham_u16)-1, .patch = (ham_u16)-1 }; }
ham_str8    ham_plugin_display_name(const ham_plugin *plugin){ return ham_likely(plugin != NULL) ? plugin->plugin_vtable->display_name() : HAM_EMPTY_STR8; }
ham_str8    ham_plugin_author(const ham_plugin *plugin){ return ham_likely(plugin != NULL) ? plugin->plugin_vtable->author() : HAM_EMPTY_STR8; }
ham_str8    ham_plugin_license(const ham_plugin *plugin){ return ham_likely(plugin != NULL) ? plugin->plugin_vtable->license() : HAM_EMPTY_STR8; }
ham_str8    ham_plugin_category(const ham_plugin *plugin){ return ham_likely(plugin != NULL) ? plugin->plugin_vtable->category() : HAM_EMPTY_STR8; }
ham_str8    ham_plugin_description(const ham_plugin *plugin){ return ham_likely(plugin != NULL) ? plugin->plugin_vtable->description() : HAM_EMPTY_STR8; }

bool ham_plugin_init(ham_plugin *plugin){
	if(!ham_check(plugin != NULL)) return false;

	std::scoped_lock lock(plugin->mut);

	if(!plugin->init_flag){
		plugin->init_flag = plugin->plugin_vtable->init();
	}

	return plugin->init_flag;
}

bool ham_plugin_fini(ham_plugin *plugin){
	if(!ham_check(plugin != NULL)) return false;

	std::scoped_lock lock(plugin->mut);

	if(plugin->init_flag){
		plugin->plugin_vtable->fini();
		plugin->init_flag = false;
	}

	return true;
}

const ham_object_vtable *ham_plugin_object(const ham_plugin *plugin, ham_str8 name){
	if(
	   !ham_check(plugin != NULL) ||
	   !ham_check(name.len && name.ptr)
	){
		return nullptr;
	}

	for(auto obj_vt : plugin->object_vtables){
		const ham::str8 obj_type_id = ham::str8(obj_vt->info->type_id);
		if(obj_type_id == name) return obj_vt;
	}

	return nullptr;
}

ham_usize ham_plugin_iterate_objects(const ham_plugin *plugin, ham_plugin_iterate_objects_fn fn, void *user){
	if(!ham_check(plugin != NULL)) return (usize)-1;

	if(fn){
		for(usize i = 0; i < plugin->object_vtables.size(); i++){
			if(!fn(plugin->object_vtables[i], user)){
				return i;
			}
		}
	}

	return plugin->object_vtables.size();
}

HAM_C_API_END
