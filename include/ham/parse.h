#ifndef HAM_PARSE_H
#define HAM_PARSE_H 1

/**
 * @defgroup HAM_PARSE Expression parsing
 * @ingroup HAM
 * @{
 */

#include "lex.h"
#include "math.h"
#include "memory.h"

HAM_C_API_BEGIN

typedef enum ham_expr_kind{
	HAM_EXPR_ERROR,
	HAM_EXPR_BINDING,
	HAM_EXPR_REF,
	HAM_EXPR_UNRESOLVED,

	HAM_EXPR_LIT_INT,
	HAM_EXPR_LIT_REAL,
	HAM_EXPR_LIT_STR,

	HAM_EXPR_KIND_COUNT,
	HAM_EXPR_KIND_ERROR = HAM_EXPR_KIND_COUNT,
} ham_expr_kind;

#define HAM_PARSE_CONTEXT_UTF(n) HAM_CONCAT(ham_parse_context_utf, n)
#define HAM_PARSE_SCOPE_UTF(n) HAM_CONCAT(ham_parse_scope_utf, n)

#define HAM_EXPR_UTF_(n, name) ham_expr_##name##_utf##n
#define HAM_EXPR_UTF(n, name) HAM_EXPR_UTF_(n, name)

#define HAM_PARSE_X_UTF 8
#include "parse.x.h"

#define HAM_PARSE_X_UTF 16
#include "parse.x.h"

#define HAM_PARSE_X_UTF 32
#include "parse.x.h"

//
// Contexts
//

ham_api ham_parse_context_utf8  *ham_parse_context_create_utf8();
ham_api ham_parse_context_utf16 *ham_parse_context_create_utf16();
ham_api ham_parse_context_utf32 *ham_parse_context_create_utf32();

#define HAM_PARSE_CONTEXT_CREATE_UTF(n) HAM_CONCAT(ham_parse_context_create_utf, n)

ham_api void ham_parse_context_destroy_utf8 (ham_parse_context_utf8  *ctx);
ham_api void ham_parse_context_destroy_utf16(ham_parse_context_utf16 *ctx);
ham_api void ham_parse_context_destroy_utf32(ham_parse_context_utf32 *ctx);

#define HAM_PARSE_CONTEXT_DESTROY_UTF(n) HAM_CONCAT(ham_parse_context_destroy_utf, n)

//
// Scopes
//

ham_api ham_parse_scope_utf8  *ham_parse_scope_create_utf8 (ham_parse_context_utf8  *ctx, const ham_parse_scope_utf8  *parent);
ham_api ham_parse_scope_utf16 *ham_parse_scope_create_utf16(ham_parse_context_utf16 *ctx, const ham_parse_scope_utf16 *parent);
ham_api ham_parse_scope_utf32 *ham_parse_scope_create_utf32(ham_parse_context_utf32 *ctx, const ham_parse_scope_utf32 *parent);

#define HAM_PARSE_SCOPE_CREATE_UTF(n) HAM_CONCAT(ham_parse_scope_create_utf, n)

ham_api void ham_parse_scope_destroy_utf8 (ham_parse_scope_utf8  *scope);
ham_api void ham_parse_scope_destroy_utf16(ham_parse_scope_utf16 *scope);
ham_api void ham_parse_scope_destroy_utf32(ham_parse_scope_utf32 *scope);

#define HAM_PARSE_SCOPE_DESTROY_UTF(n) HAM_CONCAT(ham_parse_scope_destroy_utf, n)

ham_api bool ham_parse_scope_bind_utf8 (ham_parse_scope_utf8  *scope, const ham_expr_binding_utf8  *binding);
ham_api bool ham_parse_scope_bind_utf16(ham_parse_scope_utf16 *scope, const ham_expr_binding_utf16 *binding);
ham_api bool ham_parse_scope_bind_utf32(ham_parse_scope_utf32 *scope, const ham_expr_binding_utf32 *binding);

#define HAM_PARSE_SCOPE_BIND_UTF(n) HAM_CONCAT(ham_parse_scope_bind_utf, n)

ham_api const ham_expr_binding_utf8  *ham_parse_scope_resolve_utf8 (const ham_parse_scope_utf8  *scope, ham_str8  name);
ham_api const ham_expr_binding_utf16 *ham_parse_scope_resolve_utf16(const ham_parse_scope_utf16 *scope, ham_str16 name);
ham_api const ham_expr_binding_utf32 *ham_parse_scope_resolve_utf32(const ham_parse_scope_utf32 *scope, ham_str32 name);

#define HAM_PARSE_SCOPE_RESOLVE_UTF(n) HAM_CONCAT(ham_parse_scope_resolve_utf, n)

//
// Expressions
//

ham_api ham_expr_base_utf8  *ham_parse_context_new_expr_utf8 (ham_parse_context_utf8  *ctx, ham_expr_kind kind, ham_token_range_utf8  tokens);
ham_api ham_expr_base_utf16 *ham_parse_context_new_expr_utf16(ham_parse_context_utf16 *ctx, ham_expr_kind kind, ham_token_range_utf16 tokens);
ham_api ham_expr_base_utf32 *ham_parse_context_new_expr_utf32(ham_parse_context_utf32 *ctx, ham_expr_kind kind, ham_token_range_utf32 tokens);

#define HAM_PARSE_CONTEXT_NEW_EXPR_UTF(n) HAM_CONCAT(ham_parse_context_new_expr_utf, n)

ham_api const ham_expr_error_utf8  *ham_parse_context_new_error_utf8 (ham_parse_context_utf8  *ctx, ham_token_range_utf8  tokens, const char *fmt_str, ...);
ham_api const ham_expr_error_utf16 *ham_parse_context_new_error_utf16(ham_parse_context_utf16 *ctx, ham_token_range_utf16 tokens, const char *fmt_str, ...);
ham_api const ham_expr_error_utf32 *ham_parse_context_new_error_utf32(ham_parse_context_utf32 *ctx, ham_token_range_utf32 tokens, const char *fmt_str, ...);

#define HAM_PARSE_CONTEXT_NEW_ERROR_UTF(n) HAM_CONCAT(ham_parse_context_new_error_utf, n)

//
// Parsing
//

ham_api const ham_expr_base_utf8  *ham_parse_utf8 (ham_parse_context_utf8  *ctx, ham_parse_scope_utf8  *scope, ham_token_range_utf8  tokens);
ham_api const ham_expr_base_utf16 *ham_parse_utf16(ham_parse_context_utf16 *ctx, ham_parse_scope_utf16 *scope, ham_token_range_utf16 tokens);
ham_api const ham_expr_base_utf32 *ham_parse_utf32(ham_parse_context_utf32 *ctx, ham_parse_scope_utf32 *scope, ham_token_range_utf32 tokens);

#define HAM_PARSE_UTF(n) HAM_CONCAT(ham_parse_utf, n)

//
// Default aliases
//

typedef HAM_PARSE_CONTEXT_UTF(HAM_UTF) ham_parse_context;
typedef HAM_PARSE_SCOPE_UTF(HAM_UTF)   ham_parse_scope;

typedef HAM_EXPR_UTF(HAM_UTF, base)       ham_expr;
typedef HAM_EXPR_UTF(HAM_UTF, error)      ham_expr_error;
typedef HAM_EXPR_UTF(HAM_UTF, binding)    ham_expr_binding;
typedef HAM_EXPR_UTF(HAM_UTF, ref)        ham_expr_ref;
typedef HAM_EXPR_UTF(HAM_UTF, unresolved) ham_expr_unresolved;

typedef HAM_EXPR_UTF(HAM_UTF, lit_int)    ham_expr_lit_int;
typedef HAM_EXPR_UTF(HAM_UTF, lit_real)   ham_expr_lit_real;
typedef HAM_EXPR_UTF(HAM_UTF, lit_str)    ham_expr_lit_str;

#define ham_parse_context_create HAM_PARSE_CONTEXT_CREATE_UTF(HAM_UTF)
#define ham_parse_context_destroy HAM_PARSE_CONTEXT_DESTROY_UTF(HAM_UTF)

#define ham_parse_scope_create HAM_PARSE_SCOPE_CREATE_UTF(HAM_UTF)
#define ham_parse_scope_destroy HAM_PARSE_SCOPE_DESTROY_UTF(HAM_UTF)
#define ham_parse_scope_bind HAM_PARSE_SCOPE_BIND_UTF(HAM_UTF)
#define ham_parse_scope_resolve HAM_PARSE_SCOPE_RESOLVE_UTF(HAM_UTF)

#define ham_parse_context_new_expr HAM_PARSE_CONTEXT_NEW_EXPR_UTF(HAM_UTF)
#define ham_parse_context_new_error HAM_PARSE_CONTEXT_NEW_ERROR_UTF(HAM_UTF)

#define ham_parse HAM_PARSE_UTF(HAM_UTF)

HAM_C_API_END

#ifdef __cplusplus

namespace ham{
	enum class expr_kind: std::underlying_type_t<ham_expr_kind>{
		error      = HAM_EXPR_ERROR,
		binding    = HAM_EXPR_BINDING,
		ref        = HAM_EXPR_REF,
		unresolved = HAM_EXPR_UNRESOLVED,

		lit_int  = HAM_EXPR_LIT_INT,
		lit_real = HAM_EXPR_LIT_REAL,
		lit_str  = HAM_EXPR_LIT_STR,

		base, // special c++ only kind
	};

	template<typename Char>
	class basic_parse_context;

	template<typename Char>
	class basic_parse_scope;

	template<typename Char, expr_kind Kind, bool Mutable = false>
	class basic_expr;

	//! @cond ignore
	namespace detail{
		template<typename Char>
		using parse_context_ctype_t = utf_conditional_t<
			Char,
			ham_parse_context_utf8,
			ham_parse_context_utf16,
			ham_parse_context_utf32
		>;

		template<typename Char>
		using parse_scope_ctype_t = utf_conditional_t<
			Char,
			ham_parse_scope_utf8,
			ham_parse_scope_utf16,
			ham_parse_scope_utf32
		>;

		template<typename Char>
		using expr_base_ctype_t = utf_conditional_t<
			Char,
			ham_expr_base_utf8,
			ham_expr_base_utf16,
			ham_expr_base_utf32
		>;

		template<typename Char>
		using expr_error_ctype_t = utf_conditional_t<
			Char,
			ham_expr_error_utf8,
			ham_expr_error_utf16,
			ham_expr_error_utf32
		>;

		template<typename Char>
		using expr_binding_ctype_t = utf_conditional_t<
			Char,
			ham_expr_binding_utf8,
			ham_expr_binding_utf16,
			ham_expr_binding_utf32
		>;

		template<typename Char>
		using expr_ref_ctype_t = utf_conditional_t<
			Char,
			ham_expr_ref_utf8,
			ham_expr_ref_utf16,
			ham_expr_ref_utf32
		>;

		template<typename Char>
		using expr_unresolved_ctype_t = utf_conditional_t<
			Char,
			ham_expr_unresolved_utf8,
			ham_expr_unresolved_utf16,
			ham_expr_unresolved_utf32
		>;

		template<typename Char>
		using expr_lit_int_ctype_t = utf_conditional_t<
			Char,
			ham_expr_lit_int_utf8,
			ham_expr_lit_int_utf16,
			ham_expr_lit_int_utf32
		>;

		template<typename Char>
		using expr_lit_real_ctype_t = utf_conditional_t<
			Char,
			ham_expr_lit_real_utf8,
			ham_expr_lit_real_utf16,
			ham_expr_lit_real_utf32
		>;

		template<typename Char>
		using expr_lit_str_ctype_t = utf_conditional_t<
			Char,
			ham_expr_lit_str_utf8,
			ham_expr_lit_str_utf16,
			ham_expr_lit_str_utf32
		>;

		template<typename Char, expr_kind Kind> struct expr_type_from_kind;
		template<typename Char> struct expr_type_from_kind<Char, expr_kind::base>:       id<expr_base_ctype_t<Char>>{};
		template<typename Char> struct expr_type_from_kind<Char, expr_kind::error>:      id<expr_error_ctype_t<Char>>{};
		template<typename Char> struct expr_type_from_kind<Char, expr_kind::binding>:    id<expr_binding_ctype_t<Char>>{};
		template<typename Char> struct expr_type_from_kind<Char, expr_kind::ref>:        id<expr_ref_ctype_t<Char>>{};
		template<typename Char> struct expr_type_from_kind<Char, expr_kind::unresolved>: id<expr_ref_ctype_t<Char>>{};

		template<typename Char> struct expr_type_from_kind<Char, expr_kind::lit_int>:  id<expr_lit_int_ctype_t<Char>>{};
		template<typename Char> struct expr_type_from_kind<Char, expr_kind::lit_real>: id<expr_lit_real_ctype_t<Char>>{};
		template<typename Char> struct expr_type_from_kind<Char, expr_kind::lit_str>:  id<expr_lit_str_ctype_t<Char>>{};

		template<typename Char, expr_kind Kind>
		using expr_type_from_kind_t = typename expr_type_from_kind<Char, Kind>::type;

		template<typename Char>
		constexpr inline auto parse_context_ctype_create = utf_conditional_t<
			Char,
			static_fn<ham_parse_context_create_utf8>,
			static_fn<ham_parse_context_create_utf16>,
			static_fn<ham_parse_context_create_utf32>
		>{};

		template<typename Char>
		constexpr inline auto parse_context_ctype_destroy = utf_conditional_t<
			Char,
			static_fn<ham_parse_context_destroy_utf8>,
			static_fn<ham_parse_context_destroy_utf16>,
			static_fn<ham_parse_context_destroy_utf32>
		>{};

		template<typename Char>
		constexpr inline auto parse_context_ctype_new_expr = utf_conditional_t<
			Char,
			static_fn<ham_parse_context_new_expr_utf8>,
			static_fn<ham_parse_context_new_expr_utf16>,
			static_fn<ham_parse_context_new_expr_utf32>
		>{};

		template<typename Char>
		constexpr inline auto parse_context_ctype_new_error = utf_conditional_t<
			Char,
			static_fn<ham_parse_context_new_error_utf8>,
			static_fn<ham_parse_context_new_error_utf16>,
			static_fn<ham_parse_context_new_error_utf32>
		>{};

		template<typename Char>
		constexpr inline auto parse_scope_ctype_create = utf_conditional_t<
			Char,
			static_fn<ham_parse_scope_create_utf8>,
			static_fn<ham_parse_scope_create_utf16>,
			static_fn<ham_parse_scope_create_utf32>
		>{};

		template<typename Char>
		constexpr inline auto parse_scope_ctype_destroy = utf_conditional_t<
			Char,
			static_fn<ham_parse_scope_destroy_utf8>,
			static_fn<ham_parse_scope_destroy_utf16>,
			static_fn<ham_parse_scope_destroy_utf32>
		>{};

		template<typename Char>
		constexpr inline auto parse_scope_ctype_bind = utf_conditional_t<
			Char,
			static_fn<ham_parse_scope_bind_utf8>,
			static_fn<ham_parse_scope_bind_utf16>,
			static_fn<ham_parse_scope_bind_utf32>
		>{};

		template<typename Char>
		constexpr inline auto parse_scope_ctype_resolve = utf_conditional_t<
			Char,
			static_fn<ham_parse_scope_resolve_utf8>,
			static_fn<ham_parse_scope_resolve_utf16>,
			static_fn<ham_parse_scope_resolve_utf32>
		>{};

		template<typename Self, typename Char, expr_kind Kind>
		class basic_expr_ext{};

		template<typename Self, typename Char>
		class basic_expr_ext<Self, Char, expr_kind::binding>{
			public:
				const Self *self() const noexcept{ return static_cast<Self*>(this); }

				basic_str<Char> name() const noexcept{ return self()->handle()->name; }
				basic_expr<Char, expr_kind::base> value() const noexcept{ return self()->handle()->value; }
		};

		template<typename Self, typename Char>
		class basic_expr_ext<Self, Char, expr_kind::ref>{
			public:
				const Self *self() const noexcept{ return static_cast<Self*>(this); }

				basic_expr<Char, expr_kind::binding> refed() const noexcept{ return self()->handle()->refed; }
		};

		template<typename Self, typename Char>
		class basic_expr_ext<Self, Char, expr_kind::unresolved>{
			public:
				const Self *self() const noexcept{ return static_cast<Self*>(this); }

				basic_str<Char> id() const noexcept{ return self()->handle()->id; }
		};
	} // namespace detail
	//! @endcond

	template<typename Char>
	class basic_parse_context{
		public:
			using ctype = detail::parse_context_ctype_t<Char>;

			using handle_type = ctype*;
			using const_handle_type = const ctype*;

			using token_range_type = basic_token_range<Char>;

			explicit basic_parse_context(handle_type handle_ = nullptr): m_handle(handle_){}

			basic_parse_context(basic_parse_context&&) noexcept = default;

			basic_parse_context &operator=(basic_parse_context&&) noexcept = default;

			operator bool() const& noexcept{ return (bool)m_handle; }

			operator bool() const&& = delete;

			operator handle_type() & noexcept{ return m_handle.get(); }
			operator const_handle_type() const& noexcept{ return m_handle.get(); }

			operator const_handle_type() const&& = delete;

			template<expr_kind Kind>
			basic_expr<Char, Kind, true> new_expr(const token_range_type &tokens = {}){
				return (detail::expr_type_from_kind_t<Char, Kind>*)detail::parse_context_ctype_new_expr<Char>(handle(), Kind, tokens);
			}

			template<typename ... Args>
			basic_expr<Char, expr_kind::error> new_error(const token_range_type &tokens, const char *fmt_str, Args &&... args){
				return detail::parse_context_ctype_new_error<Char>(handle(), tokens, fmt_str, std::forward<Args>(args)...);
			}

			template<typename ... Args>
			basic_expr<Char, expr_kind::error> new_error(const char *fmt_str, Args &&... args){
				return detail::parse_context_ctype_new_error<Char>(handle(), { nullptr, nullptr }, fmt_str, std::forward<Args>(args)...);
			}

			handle_type handle() noexcept{ return m_handle.get(); }
			const_handle_type handle() const noexcept{ return m_handle.get(); }

		private:
			unique_handle<handle_type, detail::parse_context_ctype_destroy<Char>> m_handle;
	};

	template<typename Char>
	class basic_parse_scope{
		public:
			using ctype = detail::parse_scope_ctype_t<Char>;
			using cparse_context = detail::parse_context_ctype_t<Char>;
			using expr_binding = basic_expr<Char, expr_kind::binding>;

			using str_type = basic_str<Char>;

			using handle_type = ctype*;
			using const_handle_type = const ctype*;

			explicit basic_parse_scope(handle_type handle_) noexcept
				: m_handle(handle_){}

			explicit basic_parse_scope(const cparse_context *ctx, const ctype *parent_ = nullptr)
				: m_handle(detail::parse_scope_ctype_create<Char>(ctx, parent_)){}

			explicit basic_parse_scope(basic_parse_context<Char> &ctx, const ctype *parent_ = nullptr)
				: basic_parse_scope(ctx.handle(), parent_){}

			basic_parse_scope(basic_parse_scope&&) noexcept = default;

			operator bool() const& noexcept{ return (bool)m_handle; }

			operator bool() const&& = delete;

			operator handle_type() & noexcept{ return m_handle.get(); }
			operator const_handle_type() const& noexcept{ return m_handle.get(); }

			operator const_handle_type() const&& = delete;

			basic_parse_scope &operator=(basic_parse_scope&&) noexcept = default;

			bool bind(expr_binding binding){
				return detail::parse_scope_ctype_bind<Char>(m_handle.get(), binding);
			}

			expr_binding resolve(str_type name) noexcept{
				return detail::parse_scope_ctype_resolve<Char>(m_handle.get(), name);
			}

			handle_type handle() noexcept{ return m_handle.get(); }
			const_handle_type handle() const noexcept{ return m_handle.get(); }

		private:
			unique_handle<handle_type, detail::parse_scope_ctype_destroy<Char>> m_handle;
	};

	template<typename Char, expr_kind Kind, bool Mutable>
	class basic_expr: public detail::basic_expr_ext<basic_expr<Char, Kind>, Char, Kind>{
		public:
			using ctype = detail::expr_type_from_kind_t<Char, Kind>;

			using cexpr_base_type = std::conditional_t<
				Mutable, detail::expr_base_ctype_t<Char>, const detail::expr_base_ctype_t<Char>
			>;

			using handle_type = std::conditional_t<Mutable, ctype*, const ctype*>;

			basic_expr(handle_type handle_ = nullptr) noexcept
				: m_expr(handle_){}

			operator basic_expr<Char, expr_kind::base, Mutable>() const noexcept{
				return (cexpr_base_type*)m_expr;
			}

			operator bool() const noexcept{ return m_expr != nullptr; }

			operator handle_type() const noexcept{ return m_expr; }

			handle_type handle() const noexcept{ return m_expr; }

			expr_kind kind() const noexcept{
				if constexpr(Kind == expr_kind::base){
					return m_expr->kind;
				}
				else{
					return m_expr->super.kind;
				}
			}

		private:
			handle_type m_expr;
	};

	template<expr_kind Kind, bool Mutable = false>
	using expr_utf8 = basic_expr<char8, Kind, Mutable>;

	using expr_base_utf8       = expr_utf8<expr_kind::base>;
	using expr_error_utf8      = expr_utf8<expr_kind::error>;
	using expr_binding_utf8    = expr_utf8<expr_kind::binding>;
	using expr_ref_utf8        = expr_utf8<expr_kind::ref>;
	using expr_unresolved_utf8 = expr_utf8<expr_kind::unresolved>;

	using expr_lit_int_utf8  = expr_utf8<expr_kind::lit_int>;
	using expr_lit_real_utf8 = expr_utf8<expr_kind::lit_real>;
	using expr_lit_str_utf8  = expr_utf8<expr_kind::lit_str>;

	template<expr_kind Kind, bool Mutable = false>
	using expr_utf16 = basic_expr<char16, Kind, Mutable>;

	using expr_base_utf16       = expr_utf16<expr_kind::base>;
	using expr_error_utf16      = expr_utf16<expr_kind::error>;
	using expr_binding_utf16    = expr_utf16<expr_kind::binding>;
	using expr_ref_utf16        = expr_utf16<expr_kind::ref>;
	using expr_unresolved_utf16 = expr_utf16<expr_kind::unresolved>;

	using expr_lit_int_utf16  = expr_utf16<expr_kind::lit_int>;
	using expr_lit_real_utf16 = expr_utf16<expr_kind::lit_real>;
	using expr_lit_str_utf16  = expr_utf16<expr_kind::lit_str>;

	template<expr_kind Kind, bool Mutable = false>
	using expr_utf32 = basic_expr<char32, Kind, Mutable>;

	using expr_base_utf32       = expr_utf32<expr_kind::base>;
	using expr_error_utf32      = expr_utf32<expr_kind::error>;
	using expr_binding_utf32    = expr_utf32<expr_kind::binding>;
	using expr_ref_utf32        = expr_utf32<expr_kind::ref>;
	using expr_unresolved_utf32 = expr_utf32<expr_kind::unresolved>;

	using expr_lit_int_utf32  = expr_utf32<expr_kind::lit_int>;
	using expr_lit_real_utf32 = expr_utf32<expr_kind::lit_real>;
	using expr_lit_str_utf32  = expr_utf32<expr_kind::lit_str>;

	template<expr_kind Kind, bool Mutable = false>
	using expr = basic_expr<uchar, Kind, Mutable>;

	using expr_base       = expr<expr_kind::base>;
	using expr_error      = expr<expr_kind::error>;
	using expr_binding    = expr<expr_kind::binding>;
	using expr_ref        = expr<expr_kind::ref>;
	using expr_unresolved = expr<expr_kind::unresolved>;

	using expr_lit_int  = expr<expr_kind::lit_int>;
	using expr_lit_real = expr<expr_kind::lit_real>;
	using expr_lit_str  = expr<expr_kind::lit_str>;
}

#endif // __cplusplus

/**
 * @}
 */

#endif // !HAM_PARSE_H
