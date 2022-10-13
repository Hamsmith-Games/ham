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

#ifndef HAM_DSO_H
#define HAM_DSO_H 1

/**
 * @defgroup HAM_DSO Dynamic shared objects
 * @ingroup HAM
 * @{
 */

#include "typedefs.h"

HAM_C_API_BEGIN

//! An opaque handle to a DSO.
typedef void *ham_dso_handle;

typedef enum ham_dso_flags{
	//! Lazily load symbols as required.
	HAM_DSO_LAZY = 0,

	//! Load all symbols upon opening. Overrides \ref HAM_DSO_LAZY .
	HAM_DSO_NOW  = 1,

	//! Make symbols local to a DSO handle.
	HAM_DSO_LOCAL = 1 << 1,

	//! Make symbols resolvable by other DSOs. Overrides \ref HAM_DSO_LOCAL .
	HAM_DSO_GLOBAL = 1 << 2,
} ham_dso_flags;

/**
 * Open a dynamic shared object.
 * @param path null-terminated path to the DSO
 * @param flags bitwise OR of \ref ham_dso_flags or ``0`` for defaults
 * @returns newly opened handle to the DSO or ``NULL`` on error
 * @see ham_dso_close
 */
ham_api ham_nothrow ham_dso_handle ham_dso_open_c(const char *path, ham_u32 flags);

/**
 * Open a dynamic shared object.
 * @param path path to the DSO
 * @param flags bitwise OR of \ref ham_dso_flags or ``0`` for defaults
 * @return newly opened handle to the DSO or ``NULL`` on error
 * @see ham_dso_close
 */
ham_nothrow static inline ham_dso_handle ham_dso_open(ham_str8 path, ham_u32 flags){
	if(path.ptr && path.len){
		if(path.len >= HAM_PATH_BUFFER_SIZE){
			return ham_null;
		}

		ham_path_buffer_utf8 path_buf;
		memcpy(path_buf, path.ptr, path.len);
		path_buf[path.len] = '\0';

		return ham_dso_open_c(path_buf, flags);
	}
	else{
		return ham_dso_open_c(ham_null, flags);
	}
}

/**
 * Close a dynamic shared object.
 * @param handle handle to the DSO to close
 */
ham_api ham_nothrow void ham_dso_close(ham_dso_handle handle);

ham_api ham_nothrow bool ham_dso_path(ham_dso_handle handle, ham_usize buf_size, ham_char8 *buf);

/**
 * Try to get a symbol from a DSO.
 * @param handle handle of the DSO
 * @param id null-terminated symbol id to retrieve
 * @returns symbol by name \p id or ``NULL`` on error
 * @see ham_dso_iterate_symbols
 */
ham_api ham_nothrow void *ham_dso_symbol_c(ham_dso_handle handle, const char *id);

/**
 * Try to get a symbol from a DSO.
 * @param handle handle of the DSO
 * @param id symbol id to retrieve
 * @returns symbol by name \p id or ``NULL`` on error
 * @see ham_dso_iterate_symbols
 */
ham_nothrow static inline void *ham_dso_symbol(ham_dso_handle handle, ham_str8 id){
	if(!id.ptr || id.len >= HAM_NAME_BUFFER_SIZE){
		return ham_null;
	}

	ham_name_buffer_utf8 id_buf;
	memcpy(id_buf, id.ptr, id.len);
	id_buf[id.len] = '\0';

	return ham_dso_symbol_c(handle, id_buf);
}

typedef bool(*ham_dso_iterate_symbols_fn)(ham_dso_handle handle, ham_str8 name, void *user);

/**
 * Iterate all symbols in a DSO, passing a null-terminated string for each symbol name to \p fn .
 * @param handle handle of the DSO
 * @param fn function to call on each iteration or ``NULL`` to only count symbols
 * @param user data passed on each call to \p fn
 * @returns number of symbols iterated when \p fn returned ``false`` or we reached the end; or ``(ham_usize)-1`` on error
 * @see ham_dso_get_num_symbols
 */
ham_api ham_usize ham_dso_iterate_symbols(ham_dso_handle handle, ham_dso_iterate_symbols_fn fn, void *user);

/**
 * Helper function for querying the number of symbols in a DSO.
 * Equivalent to ``ham_dso_iterate_symbols(handle, NULL, NULL)``.
 * @param handle handle of the DSO
 * @returns number of symbols in \p handle or ``(ham_usize)-1`` on error
 */
static inline ham_usize ham_dso_get_num_symbols(ham_dso_handle handle){ return ham_dso_iterate_symbols(handle, ham_null, ham_null); }

HAM_C_API_END

namespace ham{
	class dso{
		public:
			dso() noexcept: m_handle(nullptr){}

			explicit dso(const str8 &path, ham_dso_flags flags = HAM_DSO_LAZY) noexcept
				: m_handle(ham_dso_open(path, flags)){}

			operator bool() const noexcept{ return !!m_handle; }

			usize num_symbols() const noexcept{ return ham_dso_get_num_symbols(m_handle.get()); }

			void *symbol(const str8 &name) noexcept{ return ham_dso_symbol(m_handle.get(), name); }

			usize iterate_symbols(ham_dso_iterate_symbols_fn fn, void *user){
				return ham_dso_iterate_symbols(m_handle.get(), fn, user);
			}

		private:
			unique_handle<ham_dso_handle, ham_dso_close> m_handle;
	};
}

/**
 * @}
 */

#endif // !HAM_DSO_H
