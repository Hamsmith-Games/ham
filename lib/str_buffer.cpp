/**
 * The Ham Programming Language
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
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "ham/str_buffer.h"
#include "ham/memory.h"

namespace ham{
	namespace detail{
		template<typename Char>
		struct str_buffer_ctype_impl{
			class allocator<Char> allocator;
			Char *mem;
			usize capacity, stored;
		};

		template<typename Char>
		inline bool str_buffer_impl_reserve(str_buffer_ctype_t<Char> *buf, usize req_capacity){
			if(!buf) return false;

			if(buf->capacity >= req_capacity) return true;

			Char *new_mem = buf->allocator.allocate(req_capacity + 1);
			if(!new_mem) return false;

			if(buf->mem && buf->stored){
				memcpy(new_mem, buf->mem, sizeof(Char) * buf->stored);
				new_mem[buf->stored] = Char(0);
			}

			buf->allocator.deallocate(buf->mem, buf->capacity + 1);
			buf->mem = new_mem;
			buf->capacity = req_capacity;
			return true;
		}

		template<typename Char>
		inline bool str_buffer_impl_resize(str_buffer_ctype_t<Char> *buf, usize req_size, Char fill){
			if(!str_buffer_impl_reserve<Char>(buf, req_size)) return false;

			if(req_size > buf->stored){
				const usize fill_len = req_size - buf->stored;
				for(usize i = 0; i < fill_len; i++){
					buf->mem[buf->stored + i] = fill;
				}
			}

			buf->mem[req_size] = '\0';
			buf->stored = req_size;
			return true;
		}

		template<typename Char>
		inline str_buffer_ctype_t<Char> *str_buffer_impl_create_allocator(const ham_allocator *allocator_, str_ctype_t<Char> str){
			using ctype = str_buffer_ctype_t<Char>;

			allocator<ctype> allocator(allocator_);

			const auto buf_mem = allocator.template allocate<ctype>(1);
			if(!buf_mem) return nullptr;

			const auto buf = allocator.template construct<ctype>(buf_mem);
			if(!buf){
				allocator.deallocate(buf_mem);
				return nullptr;
			}

			buf->allocator = allocator;
			buf->mem = nullptr;
			buf->capacity = 0;
			buf->stored = 0;

			if(str.ptr && str.len){
				if(!str_buffer_ctype_reserve<Char>(buf, str.len)){
					allocator.destroy(buf);
					allocator.deallocate(buf_mem);
					return nullptr;
				}

				memcpy(buf->mem, str.ptr, str.len * sizeof(Char));
				buf->mem[str.len] = '\0';

				buf->stored = str.len;
			}

			return buf;
		}

		template<typename Char>
		inline void str_buffer_impl_destroy(str_buffer_ctype_t<Char> *buf){
			if(!buf) return;

			const auto allocator = buf->allocator;

			if(buf->mem){
				allocator.deallocate(buf->mem, buf->capacity + 1);
			}

			allocator.destroy(buf);
			allocator.deallocate(buf);
		}

		template<typename Char>
		inline str_ctype_t<Char> str_buffer_impl_get(const str_buffer_ctype_t<Char> *buf){
			if(buf) return { buf->mem, buf->stored };
			else    return { nullptr,  0 };
		}

		template<typename Char>
		inline bool str_buffer_impl_set(str_buffer_ctype_t<Char> *buf, str_ctype_t<Char> str){
			if(!buf) return false;

			if(!str.ptr || !str.len){
				if(buf->mem) buf->mem[0] = '\0';
				buf->stored = 0;
				return true;
			}

			if(!str_buffer_impl_reserve<Char>(buf, str.len)){
				return false;
			}

			memcpy(buf->mem, str.ptr, str.len * sizeof(Char));
			buf->mem[str.len] = Char(0);

			buf->stored = str.len;

			return true;
		}
	}
}

HAM_C_API_BEGIN

struct ham_str_buffer_utf8:  ham::detail::str_buffer_ctype_impl<ham_char8 >{};
struct ham_str_buffer_utf16: ham::detail::str_buffer_ctype_impl<ham_char16>{};
struct ham_str_buffer_utf32: ham::detail::str_buffer_ctype_impl<ham_char32>{};

ham_str_buffer_utf8  *ham_str_buffer_create_allocator_utf8 (const ham_allocator *allocator, ham_str8 str) { return ham::detail::str_buffer_impl_create_allocator<ham_char8> (allocator, str); }
ham_str_buffer_utf16 *ham_str_buffer_create_allocator_utf16(const ham_allocator *allocator, ham_str16 str){ return ham::detail::str_buffer_impl_create_allocator<ham_char16>(allocator, str); }
ham_str_buffer_utf32 *ham_str_buffer_create_allocator_utf32(const ham_allocator *allocator, ham_str32 str){ return ham::detail::str_buffer_impl_create_allocator<ham_char32>(allocator, str); }

void ham_str_buffer_destroy_utf8 (ham_str_buffer_utf8  *str_buf){ ham::detail::str_buffer_impl_destroy<ham_char8> (str_buf); }
void ham_str_buffer_destroy_utf16(ham_str_buffer_utf16 *str_buf){ ham::detail::str_buffer_impl_destroy<ham_char16>(str_buf); }
void ham_str_buffer_destroy_utf32(ham_str_buffer_utf32 *str_buf){ ham::detail::str_buffer_impl_destroy<ham_char32>(str_buf); }

bool ham_str_buffer_reserve_utf8 (ham_str_buffer_utf8  *str_buf, ham_usize req_capacity){ return ham::detail::str_buffer_impl_reserve<ham_char8> (str_buf, req_capacity); }
bool ham_str_buffer_reserve_utf16(ham_str_buffer_utf16 *str_buf, ham_usize req_capacity){ return ham::detail::str_buffer_impl_reserve<ham_char16>(str_buf, req_capacity); }
bool ham_str_buffer_reserve_utf32(ham_str_buffer_utf32 *str_buf, ham_usize req_capacity){ return ham::detail::str_buffer_impl_reserve<ham_char32>(str_buf, req_capacity); }

bool ham_str_buffer_resize_utf8 (ham_str_buffer_utf8  *str_buf, ham_usize req_size, ham_char8  fill){ return ham::detail::str_buffer_impl_resize<ham_char8> (str_buf, req_size, fill); }
bool ham_str_buffer_resize_utf16(ham_str_buffer_utf16 *str_buf, ham_usize req_size, ham_char16 fill){ return ham::detail::str_buffer_impl_resize<ham_char16>(str_buf, req_size, fill); }
bool ham_str_buffer_resize_utf32(ham_str_buffer_utf32 *str_buf, ham_usize req_size, ham_char32 fill){ return ham::detail::str_buffer_impl_resize<ham_char32>(str_buf, req_size, fill); }

ham_char8  *ham_str_buffer_ptr_utf8 (ham_str_buffer_utf8  *str_buf){ return str_buf ? str_buf->mem : nullptr; }
ham_char16 *ham_str_buffer_ptr_utf16(ham_str_buffer_utf16 *str_buf){ return str_buf ? str_buf->mem : nullptr; }
ham_char32 *ham_str_buffer_ptr_utf32(ham_str_buffer_utf32 *str_buf){ return str_buf ? str_buf->mem : nullptr; }

const ham_char8  *ham_str_buffer_c_str_utf8 (const ham_str_buffer_utf8  *str_buf){ return str_buf ? str_buf->mem : nullptr; }
const ham_char16 *ham_str_buffer_c_str_utf16(const ham_str_buffer_utf16 *str_buf){ return str_buf ? str_buf->mem : nullptr; }
const ham_char32 *ham_str_buffer_c_str_utf32(const ham_str_buffer_utf32 *str_buf){ return str_buf ? str_buf->mem : nullptr; }

ham_str8  ham_str_buffer_get_utf8 (const ham_str_buffer_utf8  *str_buf){ return ham::detail::str_buffer_impl_get<ham_char8> (str_buf); }
ham_str16 ham_str_buffer_get_utf16(const ham_str_buffer_utf16 *str_buf){ return ham::detail::str_buffer_impl_get<ham_char16>(str_buf); }
ham_str32 ham_str_buffer_get_utf32(const ham_str_buffer_utf32 *str_buf){ return ham::detail::str_buffer_impl_get<ham_char32>(str_buf); }

bool ham_str_buffer_set_utf8 (ham_str_buffer_utf8  *str_buf, ham_str8  str){ return ham::detail::str_buffer_impl_set<ham_char8> (str_buf, str); }
bool ham_str_buffer_set_utf16(ham_str_buffer_utf16 *str_buf, ham_str16 str){ return ham::detail::str_buffer_impl_set<ham_char16>(str_buf, str); }
bool ham_str_buffer_set_utf32(ham_str_buffer_utf32 *str_buf, ham_str32 str){ return ham::detail::str_buffer_impl_set<ham_char32>(str_buf, str); }

HAM_C_API_END
