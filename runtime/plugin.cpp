/*
 * Ham Runtime
 * Copyright (C) 2022  Hamsmith Ltd.
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
#include "ham/str_buffer.h"

#include "robin_hood.h"

#include <dirent.h>

using namespace ham::typedefs;

HAM_C_API_BEGIN

struct ham_plugin{
	const ham_allocator *allocator = nullptr;
	robin_hood::unordered_flat_map<ham::uuid, const ham_plugin_vtable*> vtable_map;
};

bool ham_plugin_find(const char *id, ham_str8 path, ham_plugin **plugin_ret, ham_dll_handle *dll_ret, const ham_plugin_vtable **vtable_ret){
	if(
		!ham_check(id != NULL) ||
		!ham_check(plugin_ret != NULL) ||
		!ham_check(dll_ret != NULL) ||
		!ham_check(vtable_ret != NULL) ||
		!ham_check(!path.len || path.ptr)
	){
		return false;
	}

	ham_path_buffer_utf8 cwd_buf;

	if(!path.len){
		path.ptr = getcwd(cwd_buf, sizeof(cwd_buf));
		path.len = strlen(path.ptr);
	}

	constexpr ham::str8 plugin_mime = HAM_PLATFORM_DLL_MIME;
	constexpr ham::str8 minimum_subpath = "/libx" HAM_PLATFORM_DLL_EXT;

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

		const ham_dll_handle plugin_dll = ham_dll_open_c(plugin_dirent->d_name);
		if(!plugin_dll){
			ham_logapiwarnf("Failed to open plugin: %s", path_buf);
			continue;
		}

		ham_plugin *const plugin = ham_plugin_load(plugin_dll);
		if(!plugin){
			ham_logapiwarnf("Failed to load opened plugin: %s", path_buf);
			ham_dll_close(plugin_dll);
			continue;
		}

		const ham_usize num_vtables = ham_plugin_iterate_vtables(
			plugin,
			[](const ham_plugin_vtable *vtable, void *user){
				const auto data = reinterpret_cast<plugin_iter_data*>(user);

				if(
					(vtable->name()         == ham::str8(data->id)) ||
					(vtable->display_name() == ham::str8(data->id))
				){
					data->vtable = vtable;
					return false;
				}

				return true;
			},
			&data
		);

		if(num_vtables == (ham_usize)-1){
			ham_logapiwarnf("Failed to iterate plugin vtables");
		}
		else if(data.vtable != nullptr){
			*plugin_ret = plugin;
			*dll_ret = plugin_dll;
			*vtable_ret = data.vtable;
			closedir(plugin_dir);
			return true;
		}

		ham_plugin_unload(plugin);
		ham_dll_close(plugin_dll);
	}

	closedir(plugin_dir);
	return false;
}

ham_plugin *ham_plugin_load(ham_dll_handle dll){
	if(!dll){
		dll = ham_dll_open_c(nullptr);
		if(!dll){
			ham_logapierrorf("Error opening main executable as shared object");
			return nullptr;
		}
	}

	const ham::allocator<ham_plugin> allocator;

	const auto mem = allocator.allocate(1);
	if(!mem) return nullptr;

	const auto ptr = allocator.construct(mem);
	if(!ptr){
		allocator.deallocate(mem);
		return nullptr;
	}

	ptr->allocator = allocator;

	ham_usize total_num_syms = ham_dll_iterate_symbols(
		dll,
		[](ham_dll_handle dll, ham_str8 sym_name, void *user){
			const auto plugin = reinterpret_cast<ham_plugin*>(user);

			constexpr ham::str8 vtable_prefix = HAM_STRINGIFY(HAM_IMPL_PLUGIN_VTABLE_NAME_PREFIX);

			const auto sym_str = ham::str8(sym_name);

			if(sym_str.len() <= vtable_prefix.len() || strncmp(sym_str.ptr(), vtable_prefix.ptr(), vtable_prefix.len()) != 0){
				return true;
			}

			using vtable_fptr = const ham_plugin_vtable*(*)();

			const auto vtable_fn = reinterpret_cast<vtable_fptr>(ham_dll_symbol_c(dll, sym_name.ptr));
			if(!vtable_fn){
				ham_logwarnf("ham_plugin_load", "Failed to load vtable symbol: %s", sym_name.ptr);
				return true;
			}

			const auto vtable = vtable_fn();
			if(!vtable){
				ham_logwarnf("ham_plugin_load", "Bad plugin vtable function: %s", sym_name.ptr);
				return true;
			}

			const ham::uuid plugin_uuid = vtable->uuid();

			plugin->vtable_map[plugin_uuid] = vtable;

			return true;
		},
		ptr
	);

	if(total_num_syms == (usize)-1){
		ham_logapierrorf("Failed to iterate dll symbols");
		allocator.destroy(ptr);
		allocator.deallocate(mem);
		return nullptr;
	}

	return ptr;
}

void ham_plugin_unload(ham_plugin *plugin){
	if(!plugin) return;

	const ham::allocator<ham_plugin> allocator;

	allocator.destroy(plugin);
	allocator.deallocate(plugin);
}

ham_usize ham_plugin_iterate_vtables(const ham_plugin *plugin, ham_plugin_iterate_vtables_fn fn, void *user){
	if(!plugin) return (usize)-1;

	if(fn){
		usize num_counted = 0;
		for(auto &&map_pair : plugin->vtable_map){
			++num_counted;
			if(!fn(map_pair.second, user)) break;
		}

		return num_counted;
	}
	else{
		return plugin->vtable_map.size();
	}
}

HAM_C_API_END
