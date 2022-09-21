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

#ifndef HAM_LEX_H
#define HAM_LEX_H 1

/**
 * @defgroup HAM_LEX Lexing
 * @ingroup HAM
 * @{
 */

#include "typedefs.h"

HAM_C_API_BEGIN

#define HAM_LEX_ERROR_MAX_LENGTH 512

typedef struct ham_token_utf8  ham_token_utf8;
typedef struct ham_token_utf16 ham_token_utf16;
typedef struct ham_token_utf32 ham_token_utf32;

typedef struct ham_token_range_utf8  ham_token_range_utf8;
typedef struct ham_token_range_utf16 ham_token_range_utf16;
typedef struct ham_token_range_utf32 ham_token_range_utf32;

typedef enum ham_token_kind{
	HAM_TOKEN_EOF,
	HAM_TOKEN_ERROR,
	HAM_TOKEN_NEWLINE,
	HAM_TOKEN_SPACE,
	HAM_TOKEN_ID,
	HAM_TOKEN_NAT,
	HAM_TOKEN_REAL,
	HAM_TOKEN_STR,
	HAM_TOKEN_OP,
	HAM_TOKEN_BRACKET,
	HAM_TOKEN_SEMICOLON,

	HAM_TOKEN_KIND_COUNT,
	HAM_TOKEN_KIND_ERROR = HAM_TOKEN_KIND_COUNT,
} ham_token_kind;

//! @cond ignore
#define HAM_IMPL_TOKEN_KIND_STR_CASE_UTF(n, val) case (val): return HAM_LIT_UTF(n, #val);

#define HAM_IMPL_TOKEN_KIND_STR_CASES_UTF(n) \
	HAM_IMPL_TOKEN_KIND_STR_CASE_UTF(n, HAM_TOKEN_EOF) \
	HAM_IMPL_TOKEN_KIND_STR_CASE_UTF(n, HAM_TOKEN_ERROR) \
	HAM_IMPL_TOKEN_KIND_STR_CASE_UTF(n, HAM_TOKEN_NEWLINE) \
	HAM_IMPL_TOKEN_KIND_STR_CASE_UTF(n, HAM_TOKEN_SPACE) \
	HAM_IMPL_TOKEN_KIND_STR_CASE_UTF(n, HAM_TOKEN_ID) \
	HAM_IMPL_TOKEN_KIND_STR_CASE_UTF(n, HAM_TOKEN_NAT) \
	HAM_IMPL_TOKEN_KIND_STR_CASE_UTF(n, HAM_TOKEN_REAL) \
	HAM_IMPL_TOKEN_KIND_STR_CASE_UTF(n, HAM_TOKEN_STR) \
	HAM_IMPL_TOKEN_KIND_STR_CASE_UTF(n, HAM_TOKEN_OP) \
	HAM_IMPL_TOKEN_KIND_STR_CASE_UTF(n, HAM_TOKEN_BRACKET) \
	default: return HAM_LIT_UTF(n, "UNKNOWN");

#define HAM_IMPL_TOKEN_KIND_STR_BODY_UTF(n, kind) \
	switch(kind){ HAM_IMPL_TOKEN_KIND_STR_CASES_UTF(n) }
//! @endcond

ham_constexpr static inline ham_str8  ham_token_kind_str_utf8 (ham_token_kind kind){ HAM_IMPL_TOKEN_KIND_STR_BODY_UTF(8,  kind) }
ham_constexpr static inline ham_str16 ham_token_kind_str_utf16(ham_token_kind kind){ HAM_IMPL_TOKEN_KIND_STR_BODY_UTF(16, kind) }
ham_constexpr static inline ham_str32 ham_token_kind_str_utf32(ham_token_kind kind){ HAM_IMPL_TOKEN_KIND_STR_BODY_UTF(32, kind) }

#define HAM_TOKEN_KIND_STR_UTF(n) HAM_CONCAT(ham_token_kind_str_utf, n)


struct ham_source_location_utf8 { ham_str8  source_name; ham_usize line, col; };
struct ham_source_location_utf16{ ham_str16 source_name; ham_usize line, col; };
struct ham_source_location_utf32{ ham_str32 source_name; ham_usize line, col; };

#define HAM_SOURCE_LOCATION_UTF(n) HAM_CONCAT(ham_source_location_utf, n)


struct ham_token_range_utf8 { const ham_token_utf8  *beg, *end; };
struct ham_token_range_utf16{ const ham_token_utf16 *beg, *end; };
struct ham_token_range_utf32{ const ham_token_utf32 *beg, *end; };

#define HAM_TOKEN_RANGE_UTF(n) HAM_CONCAT(ham_token_range_utf, n)
#define HAM_EMPTY_TOKEN_RANGE_UTF(n) ((HAM_CONCAT(ham_token_range_utf, n)){ ham_null, ham_null })

#define HAM_EMPTY_TOKEN_RANGE_UTF8  ((ham_token_range_utf8){ ham_null, 0 })
#define HAM_EMPTY_TOKEN_RANGE_UTF16 ((ham_token_range_utf16){ ham_null, 0 })
#define HAM_EMPTY_TOKEN_RANGE_UTF32 ((ham_token_range_utf32){ ham_null, 0 })


struct ham_token_utf8 { ham_token_kind kind; ham_source_location_utf8  loc; ham_str8  str; };
struct ham_token_utf16{ ham_token_kind kind; ham_source_location_utf16 loc; ham_str16 str; };
struct ham_token_utf32{ ham_token_kind kind; ham_source_location_utf32 loc; ham_str32 str; };

#define HAM_TOKEN_UTF(n) HAM_CONCAT(ham_token_utf, n)


ham_api ham_nothrow bool ham_lex_utf8 (ham_source_location_utf8  *loc, ham_str8  src, ham_token_utf8  *ret);
ham_api ham_nothrow bool ham_lex_utf16(ham_source_location_utf16 *loc, ham_str16 src, ham_token_utf16 *ret);
ham_api ham_nothrow bool ham_lex_utf32(ham_source_location_utf32 *loc, ham_str32 src, ham_token_utf32 *ret);

#define HAM_LEX_UTF(n) HAM_CONCAT(ham_lex_utf, n)


#define HAM_EMPTY_TOKEN_RANGE HAM_EMPTY_TOKEN_RANGE(HAM_UTF)

typedef HAM_SOURCE_LOCATION_UTF(HAM_UTF) ham_source_location;
typedef HAM_TOKEN_UTF(HAM_UTF)           ham_token;

#define ham_token_kind_str HAM_TOKEN_KIND_STR_UTF(HAM_UTF)
#define ham_lex HAM_LEX_UTF(HAM_UTF)

HAM_C_API_END

#ifdef __cplusplus

#include "std_vector.hpp"

namespace ham{
	namespace detail{
		template<typename Char>
		using csource_location_t = utf_conditional_t<
			Char,
			ham_source_location_utf8,
			ham_source_location_utf16,
			ham_source_location_utf32
		>;

		template<typename Char>
		using ctoken_t = utf_conditional_t<
			Char,
			ham_token_utf8,
			ham_token_utf16,
			ham_token_utf32
		>;

		template<typename Char>
		using ctoken_range_t = utf_conditional_t<
			Char,
			ham_token_range_utf8,
			ham_token_range_utf16,
			ham_token_range_utf32
		>;
	}

	template<typename Char>
	class basic_source_location;

	template<typename Char>
	class basic_token;

	template<typename Char, bool Mutable = false>
	class basic_token_iterator;

	enum class token_kind: std::underlying_type_t<ham_token_kind>{
		eof,
		error,
		newline,
		space,
		id,
		nat,
		real,
		str,
		op,
		bracket,
	};

	template<typename Char = uchar>
	constexpr inline basic_str<Char> token_kind_str(token_kind kind) noexcept{
		if constexpr(std::is_same_v<Char, char32>){
			switch(kind){
			#define HAM_CASE(val_) case (token_kind::val_): return HAM_LIT_C_UTF(32, #val_);

				HAM_CASE(eof)
				HAM_CASE(error)
				HAM_CASE(newline)
				HAM_CASE(space)
				HAM_CASE(id)
				HAM_CASE(nat)
				HAM_CASE(real)
				HAM_CASE(str)
				HAM_CASE(op)
				HAM_CASE(bracket)

			#undef HAM_CASE
				default: return U"unknown";
			}
		}
		else if constexpr(std::is_same_v<Char, char16>){
			switch(kind){
			#define HAM_CASE(val_) case (token_kind::val_): return HAM_LIT_C_UTF(16, #val_);

				HAM_CASE(eof)
				HAM_CASE(error)
				HAM_CASE(newline)
				HAM_CASE(space)
				HAM_CASE(id)
				HAM_CASE(nat)
				HAM_CASE(real)
				HAM_CASE(str)
				HAM_CASE(op)
				HAM_CASE(bracket)

			#undef HAM_CASE
				default: return u"unknown";
			}
		}
		else if constexpr(std::is_same_v<Char, char8>){
			switch(kind){
			#define HAM_CASE(val_) case (token_kind::val_): return HAM_LIT_C_UTF(8, #val_);

				HAM_CASE(eof)
				HAM_CASE(error)
				HAM_CASE(newline)
				HAM_CASE(space)
				HAM_CASE(id)
				HAM_CASE(nat)
				HAM_CASE(real)
				HAM_CASE(str)
				HAM_CASE(op)
				HAM_CASE(bracket)

			#undef HAM_CASE
				default: return "unknown";
			}
		}
		else{
			return {};
		}
	}

	/**
	 * @brief Source location template class.
	 */
	template<typename Char>
	class basic_source_location{
		public:
			using ctype = detail::csource_location_t<Char>;

			using str_type = basic_str<Char>;

			constexpr basic_source_location() noexcept
				: m_val{ { nullptr, 0 }, 0, 0 }{}

			constexpr basic_source_location(const ctype &cvalue_) noexcept
				: m_val(cvalue_){}

			constexpr basic_source_location(const basic_source_location&) noexcept = default;

			constexpr basic_source_location(str_type source_name_, usize line_, usize column_) noexcept
				: m_val{ source_name_, line_, column_ }{}

			constexpr basic_source_location(usize line_, usize column_) noexcept
				: basic_source_location(str_type(nullptr, 0), line_, column_){}

			constexpr operator const ctype&() const noexcept{ return m_val; }

			constexpr basic_source_location &operator=(const basic_source_location&) noexcept = default;

			constexpr str_type source_name() const noexcept{ return m_val.source_name; }
			constexpr usize line() const noexcept{ return m_val.line; }
			constexpr usize column() const noexcept{ return m_val.col; }

			constexpr void set_source_name(str_type new_name) noexcept{ m_val.source_name = new_name; }
			constexpr void set_line(usize new_line) noexcept{ m_val.line = new_line; }
			constexpr void set_column(usize new_column) noexcept{ m_val.column = new_column; }

			constexpr ctype &cvalue() noexcept{ return m_val; }
			constexpr const ctype &cvalue() const noexcept{ return m_val; }

		private:
			ctype m_val;
	};

	using source_location_utf8  = basic_source_location<char8>;
	using source_location_utf16 = basic_source_location<char16>;
	using source_location_utf32 = basic_source_location<char32>;

	static_assert(layout_is_same_v<source_location_utf8,  ham_source_location_utf8>);
	static_assert(layout_is_same_v<source_location_utf16, ham_source_location_utf16>);
	static_assert(layout_is_same_v<source_location_utf32, ham_source_location_utf32>);

	using source_location = basic_source_location<uchar>;

	/**
	 * @brief Token template class.
	 */
	template<typename Char>
	class basic_token{
		public:
			using ctype = detail::ctoken_t<Char>;

			using str_type = basic_str<Char>;
			using source_location_type = basic_source_location<Char>;

			constexpr basic_token() noexcept
				: m_val{ HAM_TOKEN_KIND_ERROR, source_location_type{}, str_type{} }{}

			constexpr basic_token(const ctype &value_) noexcept
				: m_val(value_){}

			constexpr basic_token(const basic_token&) noexcept = default;

			constexpr basic_token(token_kind kind_, source_location_type source_location_, str_type str_) noexcept
				: m_val{ static_cast<ham_token_kind>(kind_), source_location_, str_ }{}

			constexpr operator ctype&() & noexcept{ return m_val; }
			constexpr operator const ctype&() const& noexcept{ return m_val; }

			constexpr basic_token &operator=(const basic_token&) noexcept = default;

			constexpr bool is_valid() const noexcept{ return m_val.kind < HAM_TOKEN_KIND_COUNT; }

			constexpr bool is_eof() const noexcept{ return m_val.kind == HAM_TOKEN_EOF; }
			constexpr bool is_error() const noexcept{ return m_val.kind == HAM_TOKEN_ERROR; }
			constexpr bool is_newline() const noexcept{ return m_val.kind == HAM_TOKEN_NEWLINE; }
			constexpr bool is_space() const noexcept{ return m_val.kind == HAM_TOKEN_SPACE; }
			constexpr bool is_id() const noexcept{ return m_val.kind == HAM_TOKEN_ID; }
			constexpr bool is_nat() const noexcept{ return m_val.kind == HAM_TOKEN_NAT; }
			constexpr bool is_real() const noexcept{ return m_val.kind == HAM_TOKEN_REAL; }
			constexpr bool is_str() const noexcept{ return m_val.kind == HAM_TOKEN_STR; }
			constexpr bool is_op() const noexcept{ return m_val.kind == HAM_TOKEN_OP; }
			constexpr bool is_bracket() const noexcept{ return m_val.kind == HAM_TOKEN_BRACKET; }

			constexpr token_kind kind() const noexcept{ return static_cast<token_kind>(m_val.kind); }
			constexpr source_location_type source_location() const noexcept{ return m_val.loc; }
			constexpr str_type str() const noexcept{ return m_val.str; }

			constexpr void set_kind(token_kind new_kind) noexcept{ m_val.kind = static_cast<ham_token_kind>(new_kind); }
			constexpr void set_source_location(const source_location_type &new_source_location) noexcept{ m_val.loc = new_source_location; }
			constexpr void set_str(const str_type &new_str) noexcept{ m_val.str = new_str; }

			constexpr ctype &cvalue() noexcept{ return m_val; }
			constexpr const ctype &cvalue() const noexcept{ return m_val; }

		private:
			ctype m_val;
	};

	using token_utf8  = basic_token<char8>;
	using token_utf16 = basic_token<char16>;
	using token_utf32 = basic_token<char32>;

	static_assert(layout_is_same_v<token_utf8,  ham_token_utf8>);
	static_assert(layout_is_same_v<token_utf16, ham_token_utf16>);
	static_assert(layout_is_same_v<token_utf32, ham_token_utf32>);

	using token = basic_token<uchar>;

	/**
	 * @brief Token iterator template class.
	 */
	template<typename Char, bool Mutable>
	class basic_token_iterator{
		public:
			using ctoken_type = detail::ctoken_t<Char>;
			using cpointer = std::conditional_t<Mutable, ctoken_type*, const ctoken_type*>;

			using token_type = basic_token<Char>;
			using reference = std::conditional_t<Mutable, token_type&, const token_type&>;
			using pointer = std::conditional_t<Mutable, token_type*, const token_type*>;

			constexpr basic_token_iterator(cpointer ptr_) noexcept
				: m_val{ .cptr = ptr_ }{}

			constexpr basic_token_iterator(pointer ptr_) noexcept
				: m_val{ .ptr = ptr_ }{}

			constexpr basic_token_iterator(const basic_token_iterator&) noexcept = default;

			constexpr explicit operator bool() const noexcept{ return m_val.cptr != nullptr; }

			constexpr basic_token_iterator &operator=(const basic_token_iterator&) noexcept = default;

			constexpr reference operator*() const noexcept{ return *m_val.ptr; }

			constexpr pointer operator->() const noexcept{ return m_val.ptr; }

			constexpr bool operator==(const basic_token_iterator &other) const noexcept{ return m_val.ptr == other.m_val.ptr; }
			constexpr bool operator!=(const basic_token_iterator &other) const noexcept{ return m_val.ptr != other.m_val.ptr; }
			constexpr bool operator< (const basic_token_iterator &other) const noexcept{ return m_val.ptr < other.m_val.ptr; }
			constexpr bool operator<=(const basic_token_iterator &other) const noexcept{ return m_val.ptr <= other.m_val.ptr; }
			constexpr bool operator> (const basic_token_iterator &other) const noexcept{ return m_val.ptr > other.m_val.ptr; }
			constexpr bool operator>=(const basic_token_iterator &other) const noexcept{ return m_val.ptr >= other.m_val.ptr; }

			constexpr basic_token_iterator operator+(usize n) const noexcept{ return m_val.ptr + n; }
			constexpr basic_token_iterator operator-(usize n) const noexcept{ return m_val.ptr - n; }

			constexpr basic_token_iterator &operator++() noexcept{ ++m_val.ptr; return *this; }

			constexpr basic_token_iterator operator++(int) noexcept{
				const auto old_it = *this;
				++m_val.ptr;
				return old_it;
			}

			constexpr basic_token_iterator &operator--() noexcept{ --m_val.ptr; return *this; }

			constexpr basic_token_iterator operator--(int) noexcept{
				const auto old_it = *this;
				--m_val.ptr;
				return old_it;
			}

			pointer ptr() const noexcept{ return m_val.ptr; }
			cpointer cptr() const noexcept{ return m_val.cptr; }

		//private:
			union {
				cpointer cptr;
				pointer ptr;
			} m_val;
	};

	basic_token_iterator(const ham_token_utf8*)  -> basic_token_iterator<char8>;
	basic_token_iterator(const ham_token_utf16*) -> basic_token_iterator<char16>;
	basic_token_iterator(const ham_token_utf32*) -> basic_token_iterator<char32>;

	using token_iterator_utf8  = basic_token_iterator<char8>;
	using token_iterator_utf16 = basic_token_iterator<char16>;
	using token_iterator_utf32 = basic_token_iterator<char32>;

	static_assert(layout_is_same_v<token_iterator_utf8,  const ham_token_utf8*>);
	static_assert(layout_is_same_v<token_iterator_utf16, const ham_token_utf16*>);
	static_assert(layout_is_same_v<token_iterator_utf32, const ham_token_utf32*>);

	using token_iterator = basic_token_iterator<uchar>;

	/**
	 * @brief Token range template class.
	 */
	template<typename Char>
	class basic_token_range{
		public:
			using ctype = detail::ctoken_range_t<Char>;
			using ctoken_type = detail::ctoken_t<Char>;
			using citerator = const ctoken_type*;

			using token_type = basic_token<Char>;
			using iterator = basic_token_iterator<Char>;

			constexpr basic_token_range() noexcept
				: m_u{ .cval = { nullptr, nullptr } }{}

			constexpr basic_token_range(iterator beg_, iterator end_) noexcept
				: m_u{ .self = { beg_, end_ } }{}

			constexpr basic_token_range(citerator beg_, citerator end_) noexcept
				: m_u{ .cval = { beg_, end_ } }{}

			constexpr basic_token_range(const ctype &val_) noexcept
				: m_u{ .cval = val_ }{}

			constexpr basic_token_range(const basic_token_range&) noexcept = default;

			constexpr operator const ctype&() const& noexcept{ return m_u.cval; }
			constexpr operator ctype&() & noexcept{ return m_u.cval; }

			constexpr basic_token_range &operator=(const basic_token_range&) = default;

			constexpr bool operator==(const basic_token_range &other) const noexcept{
				return (m_u.self.beg == other.m_u.self.beg) && (m_u.end == other.m_u.self.end);
			}

			constexpr bool operator!=(const basic_token_range &other) const noexcept{
				return (m_u.self.beg != other.m_u.self.beg) || (m_u.end != other.m_u.self.end);
			}

			constexpr bool contains(citerator it) const noexcept{
				return (it >= m_u.self.beg) && (it < m_u.self.end);
			}

			constexpr iterator begin() const noexcept{ return m_u.self.beg; }
			constexpr iterator end()   const noexcept{ return m_u.self.end; }

		// TODO: wait for c++ to allow this on POD types
		//private:
			union {
				ctype cval;
				struct {
					iterator beg, end;
				} self;
			} m_u;
	};

	basic_token_range(const ham_token_utf8*,  const ham_token_utf8*)  -> basic_token_range<char8>;
	basic_token_range(const ham_token_utf16*, const ham_token_utf16*) -> basic_token_range<char16>;
	basic_token_range(const ham_token_utf32*, const ham_token_utf32*) -> basic_token_range<char32>;

	basic_token_range(token_iterator_utf8,  token_iterator_utf8)  -> basic_token_range<char8>;
	basic_token_range(token_iterator_utf16, token_iterator_utf16) -> basic_token_range<char16>;
	basic_token_range(token_iterator_utf32, token_iterator_utf32) -> basic_token_range<char32>;

	using token_range_utf8  = basic_token_range<char8>;
	using token_range_utf16 = basic_token_range<char16>;
	using token_range_utf32 = basic_token_range<char32>;

	static_assert(layout_is_same_v<token_range_utf8,  ham_token_range_utf8>);
	static_assert(layout_is_same_v<token_range_utf16, ham_token_range_utf16>);
	static_assert(layout_is_same_v<token_range_utf32, ham_token_range_utf32>);

	using token_range = basic_token_range<uchar>;

	//! @cond ignore
	namespace detail{
		template<typename Char>
		constexpr static inline bool constexpr_lex_utf(csource_location_t<Char> *loc, const basic_str<Char> &str, ctoken_t<Char> *ret) noexcept{
			if(!str.ptr()) return false;

			using source_location_type = csource_location_t<Char>;
			using str_type = basic_str<Char>;
			using token_type = basic_token<Char>;
			using str_iterator_type = const Char*;

			constexpr auto lex_inline = [](
				source_location_type *loc,
				utf_cp head_cp,
				const str_type &tail,
				const auto &head_cond,
				const auto &loop_cond
			)
				-> str_iterator_type
			{
				if(!head_cond(head_cp)){
					return nullptr;
				}
				else if(tail.len() == 0){
					return tail.ptr();
				}

				utf_cp cp;
				usize off = 0;

				while(off < tail.len()){
					const auto nchars = cstr_next_codepoint<Char>(&cp, tail.ptr() + off, tail.len() - off);
					if(nchars == (usize)-1 || !loop_cond(cp)){
						break;
					}

					off += nchars;
					++loc->col;
				}

				return tail.ptr() + off;
			};

			constexpr auto lex_space = [lex_inline](source_location_type *loc, utf_cp head_cp, const str_type &tail) constexpr -> str_iterator_type{
				constexpr auto cond = [](utf_cp cp) constexpr{ return (cp != U'\n') && ham_utf_is_whitespace(cp); };
				return lex_inline(
					loc, head_cp, tail,
					cond, cond
				);
			};

			constexpr auto lex_op = [lex_inline](source_location_type *loc, utf_cp head_cp, const str_type &tail) constexpr -> str_iterator_type{
				return lex_inline(
					loc, head_cp, tail,
					ham_utf_is_op, ham_utf_is_op
				);
			};

			constexpr auto lex_id = [lex_inline](source_location_type *loc, utf_cp head_cp, const str_type &tail) constexpr -> str_iterator_type{
				return lex_inline(
					loc, head_cp, tail,
					[](utf_cp cp) constexpr{ return (cp == U'_') || ham_utf_is_alpha(cp); },
					[](utf_cp cp) constexpr{ return (cp == U'_') || ham_utf_is_alpha(cp) || ham_utf_is_digit(cp); }
				);
			};

			constexpr auto lex_str = [](source_location_type *loc, utf_cp head_cp, const str_type &tail) constexpr -> str_iterator_type{
				if(!ham_utf_is_quote(head_cp)){
					return nullptr;
				}
				else if(!tail.len()){
					return tail.begin();
				}

				utf_cp cp = 0;
				usize off = 0;

				while(off < tail.len()){
					const auto nchars = cstr_next_codepoint<Char>(&cp, tail.ptr() + off, tail.len() - off);
					if(nchars == (usize)-1){
						return tail.ptr() + off;
					}
					else if(cp == head_cp){
						off += nchars;
						++loc->col;
						break;
					}

					++loc->col;

					switch(cp){
						case U'\\':{
							// esacape sequence
							if(off == tail.len()){
								return tail.begin() + off;
							}

							const auto nchars_esacpe = cstr_next_codepoint<Char>(&cp, tail.ptr() + off, tail.len() - off);
							if(nchars_esacpe == (usize)-1){
								return tail.ptr() + off;
							}

							if(cp == '\n'){
								// string line continuation
								++loc->line;
								loc->col = 0;
							}
							else{
								++loc->col;
							}

							off += nchars_esacpe;

							break;
						}

						default: break;
					}

					off += nchars;
				}

				return tail.ptr() + off;
			};

			const source_location_type start_loc = *loc;

			const str_iterator_type src_beg = str.begin();
			const str_iterator_type src_end = str.end();

			if(src_beg == src_end){
				*ret = basic_token<Char>(token_kind::eof, start_loc, str);
				return false;
			}

			utf_cp cp;

			usize tail_off = cstr_next_codepoint<Char>(&cp, str.ptr(), str.len());
			if(tail_off == (usize)-1){
				return false;
			}

			const str_type tail(str.ptr() + tail_off, str.len() - tail_off);

			ret->kind = HAM_TOKEN_KIND_COUNT;

			str_iterator_type tok_end;

			if(cp == U'\n'){
				ret->kind = HAM_TOKEN_NEWLINE;
				++loc->line;
				loc->col = 0;
				tok_end = tail.begin();
			}
			else if(cp == U';'){
				ret->kind = HAM_TOKEN_SEMICOLON;
				tok_end = tail.begin();
			}
			else if(ham_utf_is_bracket(cp)){
				ret->kind = HAM_TOKEN_BRACKET;
				tok_end = tail.begin();
			}
			else if((tok_end = lex_op(loc, cp, tail))){
				ret->kind = HAM_TOKEN_OP;
			}
			else if((tok_end = lex_space(loc, cp, tail))){
				ret->kind = HAM_TOKEN_SPACE;
			}
			else if((tok_end = lex_id(loc, cp, tail))){
				ret->kind = HAM_TOKEN_ID;
			}
			else if((tok_end = lex_str(loc, cp, tail))){
				ret->kind = HAM_TOKEN_STR;
			}
			else if(ham_utf_is_digit(cp)){
				// special number handling
				ret->kind = HAM_TOKEN_NAT;

				const usize tail_len = ((usize)src_end - (usize)tail.ptr())/sizeof(Char);

				usize off = 0;

				while(off < tail_len){
					const auto nchars = cstr_next_codepoint<Char>(&cp, str.ptr() + off, str.len() - off);
					if(nchars == (usize)-1 || !ham_utf_is_digit(cp)){
						break;
					}

					off += nchars;
					++loc->col;
				}

				if(cp == U'.'){
					// real number
					ret->kind = HAM_TOKEN_REAL;

					// skip decimal point
					++off;
					++loc->col;

					while(off < tail_len){
						const auto nchars = cstr_next_codepoint<Char>(&cp, str.ptr() + off, str.len() - off);
						if(nchars == (usize)-1 || !ham_utf_is_digit(cp)){
							break;
						}

						off += nchars;
						++loc->col;
					}
				}

				tok_end = tail.ptr() + off;
			}

			ret->loc = start_loc;

			if(ret->kind != HAM_TOKEN_KIND_COUNT){
				ret->str = str_type(src_beg, ((uptr)tok_end - (uptr)src_beg)/(sizeof(*src_beg)));
				return true;
			}
			else{
				ret->kind = HAM_TOKEN_ERROR;
				if constexpr(std::is_same_v<Char, char8>){
					ret->str = HAM_LIT_UTF8("unrecognized unicode codepoint");
				}
				else if constexpr(std::is_same_v<Char, char16>){
					ret->str = HAM_LIT_UTF16("unrecognized unicode codepoint");
				}
				else if constexpr(std::is_same_v<Char, char32>){
					ret->str = HAM_LIT_UTF32("unrecognized unicode codepoint");
				}

				return false;
			}
		}
	}
	//! @endcond

	template<typename Char>
	constexpr bool lex(basic_source_location<Char> &loc, const basic_str<Char> &str, basic_token<Char> &ret) noexcept{
		return detail::constexpr_lex_utf<Char>(&((detail::csource_location_t<Char>&)loc), str, &(detail::ctoken_t<Char>&)ret);
	}

	template<typename Char>
	std_vector<basic_token<Char>> lex_all(basic_source_location<Char> &loc, const basic_str<Char> &str){
		std_vector<basic_token<Char>> ret;
		ret.reserve(16);

		basic_token<Char> tok;

		basic_str<Char> rem = str;

		while(lex(loc, rem, tok)){
			if(tok.is_eof()) break;

			const auto tok_str = tok.str();

			ret.emplace_back(tok);
			rem = basic_str<Char>(rem.ptr() + tok_str.len(), rem.len() - tok_str.len());
		}

		if(tok.is_error()){
			ret.emplace_back(tok);
		}

		return ret;
	}

	template<typename Char>
	std_vector<basic_token<Char>> lex_all(const basic_str<Char> &str){
		basic_source_location<Char> loc(0, 0);
		return lex_all(loc, str);
	}
}

#endif // __cplusplus

/**
 * @}
 */

#endif // !HAM_LEX_H
