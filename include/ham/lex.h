#ifndef HAM_LEX_H
#define HAM_LEX_H 1

/**
 * @defgroup HAM_LEX Lexing
 * @ingroup HAM
 * @{
 */

#include "config.h"
#include "typedefs.h"

HAM_C_API_BEGIN

#define HAM_LEX_ERROR_MAX_LENGTH 512

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

	HAM_TOKEN_KIND_COUNT,
	HAM_TOKEN_KIND_ERROR = HAM_TOKEN_KIND_COUNT,
} ham_token_kind;

#define HAM_SOURCE_LOCATION_UTF(n) HAM_CONCAT(ham_source_location_utf, n)
#define HAM_TOKEN_UTF(n) HAM_CONCAT(ham_token_utf, n)
#define HAM_TOKEN_KIND_STR_UTF(n) HAM_CONCAT(ham_token_kind_str_utf, n)

#define HAM_LEX_X_UTF 8
#include "lex.x.h"

#define HAM_LEX_X_UTF 16
#include "lex.x.h"

#define HAM_LEX_X_UTF 32
#include "lex.x.h"

#define HAM_LEX_UTF(n) HAM_CONCAT(ham_lex_utf, n)

ham_api bool ham_lex_utf8 (ham_source_location_utf8  *loc, ham_str8  src, ham_token_utf8  *ret);
ham_api bool ham_lex_utf16(ham_source_location_utf16 *loc, ham_str16 src, ham_token_utf16 *ret);
ham_api bool ham_lex_utf32(ham_source_location_utf32 *loc, ham_str32 src, ham_token_utf32 *ret);

typedef HAM_SOURCE_LOCATION_UTF(HAM_UTF) ham_source_location;
typedef HAM_TOKEN_UTF(HAM_UTF)           ham_token;

#define ham_token_kind_str HAM_TOKEN_KIND_STR_UTF(HAM_UTF)
#define ham_lex HAM_LEX_UTF(HAM_UTF)

HAM_C_API_END

#ifdef __cplusplus

namespace ham{
	namespace detail{
		template<typename Char> struct csource_location;
		template<> struct csource_location<char8>:  id<ham_source_location_utf8>{};
		template<> struct csource_location<char16>: id<ham_source_location_utf16>{};
		template<> struct csource_location<char32>: id<ham_source_location_utf32>{};

		template<typename Char> struct ctoken;
		template<> struct ctoken<char8>:  id<ham_token_utf8>{};
		template<> struct ctoken<char16>: id<ham_token_utf16>{};
		template<> struct ctoken<char32>: id<ham_token_utf32>{};

		template<typename Char>
		using csource_location_t = typename csource_location<Char>::type;

		template<typename Char>
		using ctoken_t = typename ctoken<Char>::type;
	}

	template<typename Char>
	class alignas(detail::csource_location_t<Char>) basic_source_location{
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

	template<typename Char>
	class alignas(detail::ctoken_t<Char>) basic_token{
		public:
			using ctype = detail::ctoken_t<Char>;

			using str_type = basic_str<Char>;
			using source_location_type = basic_source_location<Char>;

			constexpr basic_token() noexcept
				: m_val{ HAM_TOKEN_KIND_ERROR, source_location_type{}, str_type{} }{}

			constexpr basic_token(const ctype &value_) noexcept
				: m_val(value_){}

			constexpr basic_token(const basic_token&) noexcept = default;

			constexpr basic_token(
				token_kind kind_,
				source_location_type source_location_,
				str_type str_
			) noexcept
				: m_val{ static_cast<ham_token_kind>(kind_), source_location_, str_ }{}

			constexpr operator const ctype&() const noexcept{ return m_val; }

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

	using source_location_utf8  = basic_source_location<char8>;
	using source_location_utf16 = basic_source_location<char16>;
	using source_location_utf32 = basic_source_location<char32>;

	using source_location = basic_source_location<ham_uchar>;

	using token_utf8  = basic_token<char8>;
	using token_utf16 = basic_token<char16>;
	using token_utf32 = basic_token<char32>;

	using token = basic_token<ham_uchar>;

	template<typename Char>
	bool lex(basic_source_location<Char> &loc, const basic_str<Char> &str, basic_token<Char> &ret){
		if constexpr(std::is_same_v<Char, char32>){
			return ham_lex_utf32(&loc.cvalue(), str, &ret.cvalue());
		}
		else if constexpr(std::is_same_v<Char, char16>){
			return ham_lex_utf16(&loc.cvalue(), str, &ret.cvalue());
		}
		else if constexpr(std::is_same_v<Char, char8>){
			return ham_lex_utf8(&loc.cvalue(), str, &ret.cvalue());
		}
		else{
			return false;
		}
	}
}

#endif // __cplusplus

/**
 * @}
 */

#endif // !HAM_LEX_H
