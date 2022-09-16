#include "ham/parse.h"
#include "ham/hash.h"
#include "ham/str_buffer.h"

#include "ham/std_vector.hpp"

#include "robin_hood.h"

#include <stdarg.h>
#include <uchar.h>

namespace ham{
	namespace detail{
		template<typename Char>
		static inline parse_scope_ctype_t<Char> *impl_parse_scope_create(parse_context_ctype_t<Char> *ctx, const parse_scope_ctype_t<Char> *parent){
			if(!ctx) return nullptr;

			const auto allocator = ctx->exprs.get_allocator();

			const auto mem = allocator.template allocate<parse_scope_ctype_t<Char>>();
			if(!mem) return nullptr;

			const auto ptr = allocator.construct(mem);
			if(!ptr){
				allocator.deallocate(mem);
				return nullptr;
			}

			ptr->ctx = ctx;
			ptr->parent = parent;
			ptr->indent = parent ? parent->indent : basic_str<Char>();

			return ptr;
		}

		template<typename Char>
		static inline void impl_parse_scope_destroy(parse_scope_ctype_t<Char> *scope){
			if(!scope) return;

			const auto allocator = scope->ctx->exprs.get_allocator();

			allocator.destroy(scope);
			allocator.deallocate(scope);
		}

		template<typename Char>
		static inline parse_context_ctype_t<Char> *impl_parse_scope_context(const parse_scope_ctype_t<Char> *scope){
			return scope ? scope->ctx : nullptr;
		}

		template<typename Char>
		static inline str_ctype_t<Char> impl_parse_scope_get_indent(const parse_scope_ctype_t<Char> *scope){
			return scope ? scope->indent : basic_str<Char>();
		}

		template<typename Char>
		static inline bool impl_parse_scope_set_indent(parse_scope_ctype_t<Char> *scope, str_ctype_t<Char> new_indent){
			if(
				!scope ||
				!new_indent.ptr ||
				!new_indent.len ||
				scope->indent.len() >= new_indent.len
			){
				return false;
			}

			const basic_str<Char> subindent(new_indent.ptr, scope->indent.len());

			if(subindent != scope->indent){
				return false;
			}

			scope->indent = new_indent;
			return true;
		}

		template<typename Char>
		static inline const expr_binding_ctype_t<Char> *impl_parse_scope_resolve(const parse_scope_ctype_t<Char> *scope, str_ctype_t<Char> name){
			if(!scope || !name.ptr || !name.len) return nullptr;

			const auto ret_vec = scope->bindings.find(name);
			if(ret_vec != scope->bindings.end()) return ret_vec->second.back();
			else return scope->parent ? impl_parse_scope_resolve<Char>(scope->parent, name) : nullptr;
		}

		template<typename Char>
		static inline bool impl_parse_scope_bind(parse_scope_ctype_t<Char> *scope, const expr_binding_ctype_t<Char> *binding){
			if(!scope || !binding || !binding->name.ptr || !binding->name.len){
				return false;
			}

			auto &binding_vec = scope->bindings[binding->name];
			binding_vec.emplace_back(binding);
			return true;
		}

		template<typename Char>
		static inline basic_expr<Char, expr_kind::base> impl_parse_root(
			const basic_parse_scope_view<Char, true> &scope,
			basic_token_iterator<Char> head,
			basic_token_range<Char> tail
		);

		template<typename Char>
		static inline basic_expr<Char, expr_kind::base> impl_parse_leading(
			const basic_parse_scope_view<Char, true> &scope,
			basic_expr<Char, expr_kind::base> expr,
			basic_token_range<Char> tail
		);

		template<typename Char>
		static inline basic_expr<Char, expr_kind::base> impl_parse_id(
			const basic_parse_scope_view<Char, true> &scope,
			basic_token_iterator<Char> head,
			basic_token_range<Char> tail
		){
			if(head->kind() != token_kind::id){
				return nullptr;
			}

			basic_expr<Char, expr_kind::base> resolved = scope.resolve(head->str());
			if(!resolved){
				resolved = scope.context().new_unresolved((ctoken_range_t<Char>){ head.cptr(), head.cptr() + 1 }, head->str());
				if(!resolved){
					return nullptr;
				}
			}

			return impl_parse_leading(scope, resolved, tail);
		}

		template<typename Char>
		static inline basic_expr<Char, expr_kind::base> impl_parse_nat(
			const basic_parse_scope_view<Char, true> &scope,
			basic_token_iterator<Char> head,
			basic_token_range<Char> tail
		){
			if(head->kind() != token_kind::nat){
				return nullptr;
			}

			const auto nat_expr = scope.context().new_lit_int((ctoken_range_t<Char>){ head.cptr(), head.cptr() + 1 }, (*head).str());
			if(!nat_expr){
				return nullptr;
			}

			return impl_parse_leading<Char>(scope, nat_expr, tail);
		}

		template<typename Char>
		static inline basic_expr<Char, expr_kind::base> impl_parse_leading(
			const basic_parse_scope_view<Char, true> &scope,
			basic_expr<Char, expr_kind::base> expr,
			basic_token_range<Char> tail
		){
			if(tail.begin() == tail.end()) return expr;

			return scope.context().new_error(tail, expr, "parser unimplemented");
		}

		template<typename Char>
		static inline basic_expr<Char, expr_kind::base> impl_parse_root(
			const basic_parse_scope_view<Char, true> &scope,
			basic_token_iterator<Char> head,
			basic_token_range<Char> tail
		){
			using parse_scope_view_type = basic_parse_scope_view<Char, true>;

			if(head == tail.end()) return nullptr;

			if(head->kind() == token_kind::space){
				const auto space_head = head;
				basic_str space_str = head->str();
				++head;

				while(head < tail.end() && head->kind() == token_kind::space){
					space_str = basic_str(space_str.ptr(), space_str.len() + head->str().len());
					++head;
				}

				// newline after only spaces is ignored line
				if(head < tail.end() && head->kind() == token_kind::newline){
					++head;
					return impl_parse_root(scope, head, { (head == tail.end() ? head : head + 1), tail.end() });
				}

				// ooh, we found an indented expression
				// lets try to create a new scope with more indentation and use that
				const auto cur_indent = scope.get_indent();

				// TODO: check if indent is a *subset* of it's parent

				basic_parse_scope<Char> new_scope(scope);
				if(!new_scope.set_indent(space_str)){
					return scope.context().new_error(basic_token_range{ space_head, head }, nullptr, "mismatched indentation");
				}

				return impl_parse_root(new_scope.view(), head, { (head == tail.end() ? head : head + 1), tail.end() });
			}

			basic_expr<Char, expr_kind::base> ret;

			if(
				!(ret = impl_parse_id(scope, head, tail)) &&
				!(ret = impl_parse_nat(scope, head, tail))
			){
				ret = scope.context().new_error(basic_token_range{ head, tail.end() }, nullptr, "unrecognized expression");
			}

			return ret;
		}
	}
}

HAM_C_API_BEGIN

struct ham_parse_scope_utf8{
	ham_parse_context_utf8 *ctx = nullptr;
	const ham_parse_scope_utf8 *parent = nullptr;
	ham::str8 indent;
	robin_hood::unordered_flat_map<ham::str8, ham::std_vector<const ham_expr_binding_utf8*>> bindings;
};

struct ham_parse_scope_utf16{
	ham_parse_context_utf16 *ctx = nullptr;
	const ham_parse_scope_utf16 *parent = nullptr;
	ham::str16 indent;
	robin_hood::unordered_flat_map<ham::str16, ham::std_vector<const ham_expr_binding_utf16*>> bindings;
};

struct ham_parse_scope_utf32{
	ham_parse_context_utf32 *ctx = nullptr;
	const ham_parse_scope_utf32 *parent = nullptr;
	ham::str32 indent;
	robin_hood::unordered_flat_map<ham::str32, ham::std_vector<const ham_expr_binding_utf32*>> bindings;
};

struct ham_parse_context_utf8{
	ham::std_vector<ham_expr_base_utf8*> exprs; // use allocator from here
	ham::std_vector<ham::str_buffer_utf8> error_messages;
	ham_parse_scope_utf8 root_scope;
};

struct ham_parse_context_utf16{
	ham::std_vector<ham_expr_base_utf16*> exprs; // use allocator from here
	ham::std_vector<ham::str_buffer_utf8> error_messages;
	ham_parse_scope_utf16 root_scope;
};

struct ham_parse_context_utf32{
	ham::std_vector<ham_expr_base_utf32*> exprs; // use allocator from here
	ham::std_vector<ham::str_buffer_utf8> error_messages;
	ham_parse_scope_utf32 root_scope;
};

//
// Parse scopes
//

ham_parse_scope_utf8  *ham_parse_scope_create_utf8 (ham_parse_context_utf8  *ctx, const ham_parse_scope_utf8  *parent){ return ham::detail::impl_parse_scope_create<ham_char8> (ctx, parent); }
ham_parse_scope_utf16 *ham_parse_scope_create_utf16(ham_parse_context_utf16 *ctx, const ham_parse_scope_utf16 *parent){ return ham::detail::impl_parse_scope_create<ham_char16>(ctx, parent); }
ham_parse_scope_utf32 *ham_parse_scope_create_utf32(ham_parse_context_utf32 *ctx, const ham_parse_scope_utf32 *parent){ return ham::detail::impl_parse_scope_create<ham_char32>(ctx, parent); }

void ham_parse_scope_destroy_utf8 (ham_parse_scope_utf8 *scope) { ham::detail::impl_parse_scope_destroy<ham_char8> (scope); }
void ham_parse_scope_destroy_utf16(ham_parse_scope_utf16 *scope){ ham::detail::impl_parse_scope_destroy<ham_char16>(scope); }
void ham_parse_scope_destroy_utf32(ham_parse_scope_utf32 *scope){ ham::detail::impl_parse_scope_destroy<ham_char32>(scope); }

ham_parse_context_utf8  *ham_parse_scope_context_utf8 (const ham_parse_scope_utf8  *scope){ return ham::detail::impl_parse_scope_context<ham_char8> (scope); }
ham_parse_context_utf16 *ham_parse_scope_context_utf16(const ham_parse_scope_utf16 *scope){ return ham::detail::impl_parse_scope_context<ham_char16>(scope); }
ham_parse_context_utf32 *ham_parse_scope_context_utf32(const ham_parse_scope_utf32 *scope){ return ham::detail::impl_parse_scope_context<ham_char32>(scope); }

ham_str8  ham_parse_scope_get_indent_utf8 (const ham_parse_scope_utf8  *scope){ return ham::detail::impl_parse_scope_get_indent<ham_char8> (scope); }
ham_str16 ham_parse_scope_get_indent_utf16(const ham_parse_scope_utf16 *scope){ return ham::detail::impl_parse_scope_get_indent<ham_char16>(scope); }
ham_str32 ham_parse_scope_get_indent_utf32(const ham_parse_scope_utf32 *scope){ return ham::detail::impl_parse_scope_get_indent<ham_char32>(scope); }

bool ham_parse_scope_set_indent_utf8 (ham_parse_scope_utf8  *scope, ham_str8  new_indent){ return ham::detail::impl_parse_scope_set_indent<ham_char8> (scope, new_indent); }
bool ham_parse_scope_set_indent_utf16(ham_parse_scope_utf16 *scope, ham_str16 new_indent){ return ham::detail::impl_parse_scope_set_indent<ham_char16>(scope, new_indent); }
bool ham_parse_scope_set_indent_utf32(ham_parse_scope_utf32 *scope, ham_str32 new_indent){ return ham::detail::impl_parse_scope_set_indent<ham_char32>(scope, new_indent); }

const ham_expr_binding_utf8  *ham_parse_scope_resolve_utf8 (const ham_parse_scope_utf8  *scope, ham_str8  name){ return ham::detail::impl_parse_scope_resolve<ham_char8> (scope, name); }
const ham_expr_binding_utf16 *ham_parse_scope_resolve_utf16(const ham_parse_scope_utf16 *scope, ham_str16 name){ return ham::detail::impl_parse_scope_resolve<ham_char16>(scope, name); }
const ham_expr_binding_utf32 *ham_parse_scope_resolve_utf32(const ham_parse_scope_utf32 *scope, ham_str32 name){ return ham::detail::impl_parse_scope_resolve<ham_char32>(scope, name); }

bool ham_parse_scope_bind_utf8 (ham_parse_scope_utf8  *scope, const ham_expr_binding_utf8  *binding){ return ham::detail::impl_parse_scope_bind<ham_char8> (scope, binding); }
bool ham_parse_scope_bind_utf16(ham_parse_scope_utf16 *scope, const ham_expr_binding_utf16 *binding){ return ham::detail::impl_parse_scope_bind<ham_char16>(scope, binding); }
bool ham_parse_scope_bind_utf32(ham_parse_scope_utf32 *scope, const ham_expr_binding_utf32 *binding){ return ham::detail::impl_parse_scope_bind<ham_char32>(scope, binding); }

//
// Parse contexts
//

const ham_expr_base_utf8  *ham_parse_utf8 (ham_parse_context_utf8  *ctx, ham_parse_scope_utf8  *scope, ham_token_range_utf8  tokens){
	return ham::detail::impl_parse_root<ham_char8>(scope, tokens.beg, ham::basic_token_range{ (tokens.beg == tokens.end ? tokens.end : tokens.beg + 1), tokens.end });
}

const ham_expr_base_utf16 *ham_parse_utf16(ham_parse_context_utf16 *ctx, ham_parse_scope_utf16 *scope, ham_token_range_utf16 tokens){
	return ham::detail::impl_parse_root<ham_char16>(scope, tokens.beg, ham::basic_token_range{ (tokens.beg == tokens.end ? tokens.end : tokens.beg + 1), tokens.end });
}

const ham_expr_base_utf32 *ham_parse_utf32(ham_parse_context_utf32 *ctx, ham_parse_scope_utf32 *scope, ham_token_range_utf32 tokens){
	return ham::detail::impl_parse_root<ham_char32>(scope, tokens.beg, ham::basic_token_range{ (tokens.beg == tokens.end ? tokens.end : tokens.beg + 1), tokens.end });
}

HAM_C_API_END
