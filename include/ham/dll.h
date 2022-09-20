#ifndef HAM_DLL_H
#define HAM_DLL_H 1

/**
 * @defgroup HAM_DLL Dynamically linked libraries
 * @ingroup HAM
 * @{
 */

#include "typedefs.h"

HAM_C_API_BEGIN

typedef void *ham_dll_handle;

ham_api ham_dll_handle ham_dll_open_c(const char *path);

static inline ham_dll_handle ham_dll_open(ham_str8 path){
	if(path.ptr && path.len){
		if(path.len >= HAM_PATH_BUFFER_SIZE){
			return ham_null;
		}

		ham_path_buffer_utf8 path_buf;
		memcpy(path_buf, path.ptr, path.len);
		path_buf[path.len] = '\0';

		return ham_dll_open_c(path_buf);
	}
	else{
		return ham_dll_open_c(ham_null);
	}
}

ham_api void ham_dll_close(ham_dll_handle handle);

ham_api void *ham_dll_symbol_c(ham_dll_handle handle, const char *id);

static inline void *ham_dll_symbol(ham_dll_handle handle, ham_str8 id){
	if(!id.ptr || id.len >= HAM_NAME_BUFFER_SIZE){
		return ham_null;
	}

	ham_name_buffer_utf8 id_buf;
	memcpy(id_buf, id.ptr, id.len);
	id_buf[id.len] = '\0';

	return ham_dll_symbol_c(handle, id_buf);
}

typedef bool(*ham_dll_iterate_symbols_fn)(ham_dll_handle handle, ham_str8 name, void *user);

/**
 * @brief Iterate all symbols in a dynamically linked library.
 * @param handle handle to the dll
 * @param fn function to call on each iteration or ``NULL`` to only count symbols
 * @param user data passed on each call to \p fn
 * @returns number of symbols iterated when \p fn returned ``false`` or we reached the end
 */
ham_api ham_usize ham_dll_iterate_symbols(ham_dll_handle handle, ham_dll_iterate_symbols_fn fn, void *user);

static inline ham_usize ham_dll_get_num_symbols(ham_dll_handle handle){ return ham_dll_iterate_symbols(handle, ham_null, ham_null); }

HAM_C_API_END

/**
 * @}
 */

#endif // !HAM_DLL_H
