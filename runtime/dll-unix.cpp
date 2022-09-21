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

#include "ham/dll.h"
#include "ham/log.h"
#include "ham/memory.h"

#ifndef PACKAGE
#	define PACKAGE
#endif

#ifndef PACKAGE_VERSION
#	define PACKAGE_VERSION
#endif

#include <unistd.h>
#include <bfd.h>
#include <link.h>

static bool ham_impl_bfd_init_flag = false;

HAM_C_API_BEGIN

ham_dll_handle ham_dll_open_c(const char *path){
	void *const ret = dlopen(path, RTLD_LAZY);
	if(!ret) ham_logapierrorf("Error in dlopen: %s", dlerror());
	return ret;
}

void ham_dll_close(ham_dll_handle handle){
	if(!handle) return;

	const int res = dlclose(handle);
	if(res != 0) ham_logapierrorf("Error in dlclose: %s", dlerror());
}

void *ham_dll_symbol_c(ham_dll_handle handle, const char *id){
	if(!handle) return ham_null;

	void *const ret = dlsym(handle, id);
	if(!ret) ham_logapiverbosef("Error in dlsym: %s", dlerror());
	return ret;
}

ham_usize ham_dll_iterate_symbols(ham_dll_handle handle, ham_dll_iterate_symbols_fn fn, void *user){
	if(!handle) return (ham_usize)-1;

	struct link_map *lm;
	int res = dlinfo(handle, RTLD_DI_LINKMAP, &lm);
	if(res == -1){
		ham_logapierrorf("Error in dlinfo: %s", dlerror());
		return (ham_usize)-1;
	}

	if(!ham_impl_bfd_init_flag){
		bfd_init();
		ham_impl_bfd_init_flag = true;
	}

	const char *abs_path = lm->l_name;
	if(!abs_path){
		ham_logapierrorf("Failed to get elf executable absolute path");
		return (ham_usize)-1;
	}

	ham_path_buffer abs_path_buf;
	if(!*abs_path){
		// get self executable location
		ssize_t readlink_res = readlink(
		#ifdef __FreeBSD__
			"/proc/curproc/file",
		#else
			"/proc/self/exe",
		#endif
			abs_path_buf,
			sizeof(abs_path_buf)-1
		);

		if(readlink_res == -1){
			ham_logapierrorf("Error in readlink: %s", strerror(errno));
			return (ham_usize)-1;
		}

		abs_path_buf[readlink_res] = '\0';
		abs_path = abs_path_buf;
	}

	//ham_logapiverbosef("bfd_openr \"%s\"", abs_path);
	bfd *abfd = bfd_openr(abs_path, ham_null);
	if(!abfd){
		ham_logapierrorf("Error in bfd_openr: %s", bfd_errmsg(bfd_get_error()));
		return (ham_usize)-1;
	}

	if(!bfd_check_format(abfd, bfd_object)){
		ham_logapierrorf("Error in bfd_check_format: %s", bfd_errmsg(bfd_get_error()));

		if(!bfd_close(abfd)){
			ham_logapiwarnf("Error in bfd_close: %s", bfd_errmsg(bfd_get_error()));
		}

		return (ham_usize)-1;
	}

	long symtab_storage_size = bfd_get_symtab_upper_bound(abfd);

	if(symtab_storage_size < 0){
		ham_logapierrorf("Error in bfd_get_symtab_upper_bound: %s", bfd_errmsg(bfd_get_error()));

		if(!bfd_close(abfd)){
			ham_logapiwarnf("Error in bfd_close: %s", bfd_errmsg(bfd_get_error()));
		}

		return (ham_usize)-1;
	}
	else if(symtab_storage_size == 0){
		if(!bfd_close(abfd)){
			ham_logapiwarnf("Error in bfd_close: %s", bfd_errmsg(bfd_get_error()));
		}

		ham_logapiverbosef("No symbols in: %s", abs_path);
		return 0;
	}

	asymbol **symtab = (asymbol**)bfd_alloc(abfd, symtab_storage_size);
	if(!symtab){
		ham_logapierrorf("Failed to allocate memory for symbol table");

		if(!bfd_close(abfd)){
			ham_logapiwarnf("Error in bfd_close: %s", bfd_errmsg(bfd_get_error()));
		}

		return (ham_usize)-1;
	}

	long num_syms = bfd_canonicalize_symtab(abfd, symtab);
	if(num_syms < 0){
		ham_logapierrorf("Error in bfd_canonicalize_symtab: %s", bfd_errmsg(bfd_get_error()));

		if(!bfd_close(abfd)){
			ham_logapiwarnf("Error in bfd_close: %s", bfd_errmsg(bfd_get_error()));
		}

		return (ham_usize)-1;
	}

	if(fn){
		for(long i = 0; i < num_syms; i++){
			ham_str8 sym_name{ symtab[i]->name, strlen(symtab[i]->name) };
			if(!fn(handle, sym_name, user)) return i+1;
		}
	}

	if(!bfd_close(abfd)){
		ham_logapiwarnf("Error in bfd_close: %s", bfd_errmsg(bfd_get_error()));
	}

	return (ham_usize)num_syms;
}

HAM_C_API_END
