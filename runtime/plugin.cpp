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
#include "ham/log.h"
#include "ham/str_buffer.h"

#include "robin_hood.h"

using namespace ham::typedefs;

HAM_C_API_BEGIN

struct ham_plugin{
	const ham_allocator *allocator = nullptr;
	robin_hood::unordered_flat_map<ham::uuid, const ham_plugin_vtable*> vtable_map;
};

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
