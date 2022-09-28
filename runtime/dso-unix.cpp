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

#include "ham/dso.h"
#include "ham/check.h"

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

ham_nothrow ham_dso_handle ham_dso_open_c(const char *path, ham_u32 flags){
	int mode = RTLD_LAZY;

	if(flags & HAM_DSO_NOW){
		mode = RTLD_NOW;
	}

	if(flags & HAM_DSO_GLOBAL){
		mode |= RTLD_GLOBAL;
	}
	else if(flags & HAM_DSO_LOCAL){
		mode |= RTLD_LOCAL;
	}

	void *const ret = dlopen(path, mode);
	if(!ret) ham_logapierrorf("Error in dlopen: %s", dlerror());
	return ret;
}

ham_nothrow void ham_dso_close(ham_dso_handle handle){
	if(ham_unlikely(handle == NULL)) return;

	const int res = dlclose(handle);
	if(res != 0) ham_logapierrorf("Error in dlclose: %s", dlerror());
}

ham_nothrow bool ham_dso_path(ham_dso_handle handle, ham_usize buf_size, ham_char8 *buf){
	if(
	   !ham_check(handle != NULL) ||
	   !ham_check(buf_size != 0) ||
	   !ham_check(buf != NULL)
	){
		return false;
	}

	struct link_map *lm;
	int res = dlinfo(handle, RTLD_DI_LINKMAP, &lm);
	if(res == -1){
		ham_logapierrorf("Error in dlinfo: %s", dlerror());
		return false;
	}

	const char *abs_path = lm->l_name;
	if(!abs_path){
		ham_logapierrorf("Failed to get executable absolute path");
		return false;
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
			return false;
		}

		abs_path_buf[readlink_res] = '\0';
		abs_path = abs_path_buf;
	}

	const auto path_len = strlen(abs_path);
	if(path_len >= buf_size){
		ham_logapierrorf("DSO path is longer (%zu) than buffer size (%zu)", path_len, buf_size);
		return false;
	}

	memcpy(buf, abs_path, path_len);
	buf[path_len] = '\0';
	return true;
}

ham_nothrow void *ham_dso_symbol_c(ham_dso_handle handle, const char *id){
	if(!ham_check(handle != NULL)) return nullptr;

	void *const ret = dlsym(handle, id);
	if(!ret) ham_logapiverbosef("Error in dlsym: %s", dlerror());
	return ret;
}

ham_usize ham_dso_iterate_symbols(ham_dso_handle handle, ham_dso_iterate_symbols_fn fn, void *user){
	if(!ham_check(handle != NULL)) return (ham_usize)-1;

	if(!ham_impl_bfd_init_flag){
		bfd_init();
		ham_impl_bfd_init_flag = true;
	}

	struct link_map *lm;
	int res = dlinfo(handle, RTLD_DI_LINKMAP, &lm);
	if(res == -1){
		ham_logapierrorf("Error in dlinfo: %s", dlerror());
		return (ham_usize)-1;
	}

	const char *abs_path = lm->l_name;
	if(!abs_path){
		ham_logapierrorf("Failed to get executable absolute path");
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
