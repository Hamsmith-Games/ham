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

#ifndef HAM_STR_BUFFER_H
#define HAM_STR_BUFFER_H 1

/**
 * @defgroup HAM_STR_BUFFER String buffers
 * @ingroup HAM
 * @{
 */

#include "memory.h"
#include "hash.h"

HAM_C_API_BEGIN

typedef struct ham_str_buffer_utf8  ham_str_buffer_utf8;
typedef struct ham_str_buffer_utf16 ham_str_buffer_utf16;
typedef struct ham_str_buffer_utf32 ham_str_buffer_utf32;

ham_api ham_str_buffer_utf8  *ham_str_buffer_create_allocator_utf8 (const ham_allocator *allocator, ham_str8 str);
ham_api ham_str_buffer_utf16 *ham_str_buffer_create_allocator_utf16(const ham_allocator *allocator, ham_str16 str);
ham_api ham_str_buffer_utf32 *ham_str_buffer_create_allocator_utf32(const ham_allocator *allocator, ham_str32 str);

static inline ham_str_buffer_utf8  *ham_str_buffer_create_utf8 (ham_str8 str) { return ham_str_buffer_create_allocator_utf8 (ham_current_allocator(), str); }
static inline ham_str_buffer_utf16 *ham_str_buffer_create_utf16(ham_str16 str){ return ham_str_buffer_create_allocator_utf16(ham_current_allocator(), str); }
static inline ham_str_buffer_utf32 *ham_str_buffer_create_utf32(ham_str32 str){ return ham_str_buffer_create_allocator_utf32(ham_current_allocator(), str); }

ham_api void ham_str_buffer_destroy_utf8 (ham_str_buffer_utf8  *str_buf);
ham_api void ham_str_buffer_destroy_utf16(ham_str_buffer_utf16 *str_buf);
ham_api void ham_str_buffer_destroy_utf32(ham_str_buffer_utf32 *str_buf);

ham_api ham_char8  *ham_str_buffer_ptr_utf8 (ham_str_buffer_utf8  *str_buf);
ham_api ham_char16 *ham_str_buffer_ptr_utf16(ham_str_buffer_utf16 *str_buf);
ham_api ham_char32 *ham_str_buffer_ptr_utf32(ham_str_buffer_utf32 *str_buf);

ham_api const ham_char8  *ham_str_buffer_c_str_utf8 (const ham_str_buffer_utf8  *str_buf);
ham_api const ham_char16 *ham_str_buffer_c_str_utf16(const ham_str_buffer_utf16 *str_buf);
ham_api const ham_char32 *ham_str_buffer_c_str_utf32(const ham_str_buffer_utf32 *str_buf);

ham_api ham_str8  ham_str_buffer_get_utf8 (const ham_str_buffer_utf8  *str_buf);
ham_api ham_str16 ham_str_buffer_get_utf16(const ham_str_buffer_utf16 *str_buf);
ham_api ham_str32 ham_str_buffer_get_utf32(const ham_str_buffer_utf32 *str_buf);

ham_api bool ham_str_buffer_set_utf8 (ham_str_buffer_utf8  *str_buf, ham_str8  str);
ham_api bool ham_str_buffer_set_utf16(ham_str_buffer_utf16 *str_buf, ham_str16 str);
ham_api bool ham_str_buffer_set_utf32(ham_str_buffer_utf32 *str_buf, ham_str32 str);

ham_api bool ham_str_buffer_reserve_utf8 (ham_str_buffer_utf8  *str_buf, ham_usize req_capacity);
ham_api bool ham_str_buffer_reserve_utf16(ham_str_buffer_utf16 *str_buf, ham_usize req_capacity);
ham_api bool ham_str_buffer_reserve_utf32(ham_str_buffer_utf32 *str_buf, ham_usize req_capacity);

ham_api bool ham_str_buffer_resize_utf8 (ham_str_buffer_utf8  *str_buf, ham_usize req_size, ham_char8  fill);
ham_api bool ham_str_buffer_resize_utf16(ham_str_buffer_utf16 *str_buf, ham_usize req_size, ham_char16 fill);
ham_api bool ham_str_buffer_resize_utf32(ham_str_buffer_utf32 *str_buf, ham_usize req_size, ham_char32 fill);

ham_api bool ham_str_buffer_append_utf8 (ham_str_buffer_utf8  *str_buf, ham_str8  str);
ham_api bool ham_str_buffer_append_utf16(ham_str_buffer_utf16 *str_buf, ham_str16 str);
ham_api bool ham_str_buffer_append_utf32(ham_str_buffer_utf32 *str_buf, ham_str32 str);

ham_api bool ham_str_buffer_prepend_utf8 (ham_str_buffer_utf8  *str_buf, ham_str8  str);
ham_api bool ham_str_buffer_prepend_utf16(ham_str_buffer_utf16 *str_buf, ham_str16 str);
ham_api bool ham_str_buffer_prepend_utf32(ham_str_buffer_utf32 *str_buf, ham_str32 str);

//
// Default aliases
//

#define ham_str_buffer HAM_CONCAT(ham_str_buffer_utf, HAM_UTF)
#define ham_str_buffer_create HAM_CONCAT(ham_str_buffer_create_utf, HAM_UTF)
#define ham_str_buffer_destroy HAM_CONCAT(ham_str_buffer_destroy_utf, HAM_UTF)
#define ham_str_buffer_get HAM_CONCAT(ham_str_buffer_get_utf, HAM_UTF)

HAM_C_API_END

#ifdef __cplusplus

namespace ham{
	namespace detail{
		template<typename Char> struct str_buffer_ctype;
		template<> struct str_buffer_ctype<char8>:  id<ham_str_buffer_utf8 >{};
		template<> struct str_buffer_ctype<char16>: id<ham_str_buffer_utf16>{};
		template<> struct str_buffer_ctype<char32>: id<ham_str_buffer_utf32>{};

		template<typename Char>
		using str_buffer_ctype_t = typename str_buffer_ctype<Char>::type;

		template<typename Char>
		constexpr inline auto str_buffer_ctype_create_allocator = utf_conditional_t<
			Char,
			meta::static_fn<ham_str_buffer_create_allocator_utf8>,
			meta::static_fn<ham_str_buffer_create_allocator_utf16>,
			meta::static_fn<ham_str_buffer_create_allocator_utf32>
		>{};

		template<typename Char>
		constexpr inline auto str_buffer_ctype_create = utf_conditional_t<
			Char,
			meta::static_fn<ham_str_buffer_create_utf8>,
			meta::static_fn<ham_str_buffer_create_utf16>,
			meta::static_fn<ham_str_buffer_create_utf32>
		>{};

		template<typename Char>
		constexpr inline auto str_buffer_ctype_destroy = utf_conditional_t<
			Char,
			meta::static_fn<ham_str_buffer_destroy_utf8>,
			meta::static_fn<ham_str_buffer_destroy_utf16>,
			meta::static_fn<ham_str_buffer_destroy_utf32>
		>{};

		template<typename Char>
		constexpr inline auto str_buffer_ctype_reserve = utf_conditional_t<
			Char,
			meta::static_fn<ham_str_buffer_reserve_utf8>,
			meta::static_fn<ham_str_buffer_reserve_utf16>,
			meta::static_fn<ham_str_buffer_reserve_utf32>
		>{};

		template<typename Char>
		constexpr inline auto str_buffer_ctype_resize = utf_conditional_t<
			Char,
			meta::static_fn<ham_str_buffer_resize_utf8>,
			meta::static_fn<ham_str_buffer_resize_utf16>,
			meta::static_fn<ham_str_buffer_resize_utf32>
		>{};

		template<typename Char>
		constexpr inline auto str_buffer_ctype_append = utf_conditional_t<
			Char,
			meta::static_fn<ham_str_buffer_append_utf8>,
			meta::static_fn<ham_str_buffer_append_utf16>,
			meta::static_fn<ham_str_buffer_append_utf32>
		>{};

		template<typename Char>
		constexpr inline auto str_buffer_ctype_prepend = utf_conditional_t<
			Char,
			meta::static_fn<ham_str_buffer_prepend_utf8>,
			meta::static_fn<ham_str_buffer_prepend_utf16>,
			meta::static_fn<ham_str_buffer_prepend_utf32>
		>{};

		template<typename Char>
		constexpr inline auto str_buffer_ctype_ptr = utf_conditional_t<
			Char,
			meta::static_fn<ham_str_buffer_ptr_utf8>,
			meta::static_fn<ham_str_buffer_ptr_utf16>,
			meta::static_fn<ham_str_buffer_ptr_utf32>
		>{};

		template<typename Char>
		constexpr inline auto str_buffer_ctype_c_str = utf_conditional_t<
			Char,
			meta::static_fn<ham_str_buffer_c_str_utf8>,
			meta::static_fn<ham_str_buffer_c_str_utf16>,
			meta::static_fn<ham_str_buffer_c_str_utf32>
		>{};

		template<typename Char>
		constexpr inline auto str_buffer_ctype_get = utf_conditional_t<
			Char,
			meta::static_fn<ham_str_buffer_get_utf8>,
			meta::static_fn<ham_str_buffer_get_utf16>,
			meta::static_fn<ham_str_buffer_get_utf32>
		>{};

		template<typename Char>
		constexpr inline auto str_buffer_ctype_set = utf_conditional_t<
			Char,
			meta::static_fn<ham_str_buffer_set_utf8>,
			meta::static_fn<ham_str_buffer_set_utf16>,
			meta::static_fn<ham_str_buffer_set_utf32>
		>{};
	}

	template<typename Char>
	class basic_str_buffer{
		public:
			using ctype = detail::str_buffer_ctype_t<Char>;

			using char_type = Char;
			using str_type = basic_str<Char>;

			basic_str_buffer(): m_handle(detail::str_buffer_ctype_create_allocator<Char>(ham_current_allocator(), str_type{})){}

			explicit basic_str_buffer(const ham_allocator *allocator_)
				: m_handle(detail::str_buffer_ctype_create_allocator<Char>(allocator_, str_type{})){}

			basic_str_buffer(const str_type &str_ , const ham_allocator *allocator_ = ham_current_allocator())
				: m_handle(detail::str_buffer_ctype_create_allocator<Char>(allocator_, str_)){}

			template<usize N>
			basic_str_buffer(const Char(&arr)[N], const ham_allocator *allocator_ = ham_current_allocator())
				: basic_str_buffer(str_type(arr, arr + (N-1)), allocator_){}

			basic_str_buffer(const Char *c_str_, const ham_allocator *allocator_ = ham_current_allocator())
				: basic_str_buffer(str_type(c_str_, detail::strlen_utf(c_str_)), allocator_){}

			basic_str_buffer(basic_str_buffer&&) noexcept = default;

			basic_str_buffer &operator=(basic_str_buffer&&) noexcept = default;

			basic_str_buffer &operator=(const str_type &str_){
				set(str_);
				// TODO: signal an error; exception?
				return *this;
			}

			operator basic_str<Char>() const& noexcept{ return get(); }

			operator basic_str<Char>() const&& = delete;

			int compare(const basic_str<Char> &other) const noexcept{ return get().compare(other); }

			bool operator==(const basic_str<Char> &other) const noexcept{ return get() == other; }
			bool operator!=(const basic_str<Char> &other) const noexcept{ return get() != other; }
			bool operator< (const basic_str<Char> &other) const noexcept{ return get() <  other; }
			bool operator<=(const basic_str<Char> &other) const noexcept{ return get() <= other; }
			bool operator> (const basic_str<Char> &other) const noexcept{ return get() >  other; }
			bool operator>=(const basic_str<Char> &other) const noexcept{ return get() >= other; }

			basic_str_buffer &operator+=(const basic_str<Char> &str){
				if(!append(str)){
					// TODO: throw exception?
				}

				return *this;
			}

			basic_str_buffer operator+(const basic_str<Char> &str) const{
				basic_str_buffer ret = *this;
				ret += str;
				return ret;
			}

			bool reserve(usize req_capacity){
				return detail::str_buffer_ctype_reserve<Char>(m_handle.get(), req_capacity);
			}

			bool resize(usize req_size, char_type fill = char_type(' ')){
				return detail::str_buffer_ctype_resize<Char>(m_handle.get(), req_size, fill);
			}

			bool append(const basic_str<Char> &str){
				return detail::str_buffer_ctype_append<Char>(m_handle.get(), str);
			}

			bool prepend(const basic_str<Char> &str){
				return detail::str_buffer_ctype_prepend<Char>(m_handle.get(), str);
			}

			str_type get() const noexcept{
				return detail::str_buffer_ctype_get<Char>(m_handle.get());
			}

			bool set(const str_type &str_){
				return detail::str_buffer_ctype_set<Char>(m_handle.get(), str_);
			}

			const Char *c_str() const noexcept{
				return detail::str_buffer_ctype_c_str<Char>(m_handle.get());
			}

			Char *ptr() noexcept{
				return detail::str_buffer_ctype_ptr<Char>(m_handle.get());
			}

			const Char *ptr() const noexcept{ return c_str(); }

			usize len() const noexcept{ return get().len(); }

		private:
			unique_handle<ctype*, detail::str_buffer_ctype_destroy<Char>> m_handle;
	};

	using str_buffer_utf8  = basic_str_buffer<char8>;
	using str_buffer_utf16 = basic_str_buffer<char16>;
	using str_buffer_utf32 = basic_str_buffer<char32>;

	using str_buffer = basic_str_buffer<uchar>;

	template<> struct hash_functor<str_buffer_utf8>: hash_functor<str8>{};
	template<> struct hash_functor<str_buffer_utf16>: hash_functor<str16>{};
	template<> struct hash_functor<str_buffer_utf32>: hash_functor<str32>{};
}

template<>
struct std::hash<ham::str_buffer_utf8>: ham::hash_functor<ham::str_buffer_utf8>{};

template<>
struct std::hash<ham::str_buffer_utf16>: ham::hash_functor<ham::str_buffer_utf16>{};

template<>
struct std::hash<ham::str_buffer_utf32>: ham::hash_functor<ham::str_buffer_utf32>{};

template<typename Char>
inline std::basic_ostream<Char> &operator<<(std::basic_ostream<Char> &stream, const ham::basic_str_buffer<Char> &str){
	return stream << str.get();
}

#endif // __cplusplus

/**
 * @}
 */

#endif // !HAM_STR_BUFFER_H
