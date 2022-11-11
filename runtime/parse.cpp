/*
 * Ham Runtime
 * Copyright (C) 2022 Keith Hammond
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

#include "ham/parse.h"
#include "ham/hash.h" // IWYU pragma: keep
#include "ham/str_buffer.h"

#include "ham/std_vector.hpp"

#include "robin_hood.h"

#include <stdarg.h>
#include <uchar.h>

using namespace ham::typedefs;
using namespace ham::literals;

namespace ham{
	namespace detail{
		//
		// Scope implementation
		//

		template<typename Char>
		static inline parse_scope_ctype_t<Char> *impl_parse_scope_create(parse_context_ctype_t<Char> *ctx, parse_scope_ctype_t<Char> *parent){
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
		static inline parse_scope_ctype_t<Char> *impl_parse_scope_parent(parse_scope_ctype_t<Char> *scope){
			return scope ? scope->parent : nullptr;
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

		//
		// Context implementation
		//

		template<typename Char>
		static inline parse_context_ctype_t<Char> *impl_parse_context_create(){
			const allocator<parse_context_ctype_t<Char>> ctx_allocator;

			const auto mem = ctx_allocator.allocate(1);
			if(!mem) return nullptr;

			const auto ptr = ctx_allocator.construct(mem);
			if(!ptr){
				ctx_allocator.deallocate(mem);
				return nullptr;
			}

			ptr->exprs = std_vector<expr_base_ctype_t<Char>*>(ctx_allocator);
			ptr->error_messages = std_vector<str_buffer_utf8>(ctx_allocator);
			ptr->root_scope.ctx = ptr;

			return ptr;
		}

		template<typename Char>
		static inline void impl_parse_context_destroy(parse_context_ctype_t<Char> *ctx){
			if(!ctx) return;

			const allocator<parse_context_ctype_t<Char>> ctx_allocator(ctx->exprs.get_allocator());

			for(auto expr : ctx->exprs){
				switch(expr->kind){
				#define HAM_CASE(kind_, type_) \
					case (kind_):{ \
						const auto inner_expr = (type_*)expr; \
						ctx_allocator.destroy(inner_expr); \
						ctx_allocator.deallocate(inner_expr); \
						break; \
					}

					HAM_CASE(HAM_EXPR_ERROR,      expr_error_ctype_t<Char>)
					HAM_CASE(HAM_EXPR_BINDING,    expr_binding_ctype_t<Char>)
					HAM_CASE(HAM_EXPR_REF,        expr_ref_ctype_t<Char>)
					HAM_CASE(HAM_EXPR_UNRESOLVED, expr_unresolved_ctype_t<Char>)

					HAM_CASE(HAM_EXPR_FN,    expr_fn_ctype_t<Char>)
					HAM_CASE(HAM_EXPR_CALL,  expr_call_ctype_t<Char>)
					HAM_CASE(HAM_EXPR_BLOCK, expr_block_ctype_t<Char>)

					HAM_CASE(HAM_EXPR_UNARY_OP, expr_unary_op_ctype_t<Char>)
					HAM_CASE(HAM_EXPR_BINARY_OP, expr_binary_op_ctype_t<Char>)

					case HAM_EXPR_LIT_INT:{
						const auto int_expr = (expr_lit_int_ctype_t<Char>*)expr;
						ham_aint_finish(&int_expr->value);
						ctx_allocator.destroy(int_expr);
						ctx_allocator.deallocate(int_expr);
						break;
					}

					case HAM_EXPR_LIT_REAL:{
						const auto real_expr = (expr_lit_real_ctype_t<Char>*)expr;
						ham_areal_finish(&real_expr->value);
						ctx_allocator.destroy(real_expr);
						ctx_allocator.deallocate(real_expr);
						break;
					}

					HAM_CASE(HAM_EXPR_LIT_STR,  expr_lit_str_ctype_t<Char>)

				#undef HAM_CASE

					default:{
						// TODO: signal warning about unimplemented expression type
						ctx_allocator.destroy(expr);
						ctx_allocator.deallocate(expr);
						break;
					}
				}
			}

			ctx_allocator.destroy(ctx);
			ctx_allocator.deallocate(ctx);
		}

		template<typename Char>
		static inline expr_base_ctype_t<Char> *impl_parse_context_new_expr(
			basic_parse_context_view<Char> ctx,
			expr_kind kind,
			const basic_token_range<Char> &tokens
		){
			if(!ctx || static_cast<ham_expr_kind>(kind) >= HAM_EXPR_KIND_COUNT){
				return nullptr;
			}

			const auto allocator = ctx.handle()->exprs.get_allocator();

			switch(kind){
			#define HAM_CASE(kind_, t_) \
				case (kind_):{ \
					const auto mem = allocator.template allocate<t_>(); \
					if(!mem) return nullptr; \
					const auto ptr = allocator.construct(mem); \
					if(!ptr){ \
						allocator.deallocate(mem); \
						return nullptr; \
					} \
					ptr->super.kind = static_cast<ham_expr_kind>(kind); \
					ptr->super.tokens = tokens; \
					return &ptr->super; \
					break; \
				}

				HAM_CASE(expr_kind::error,      expr_error_ctype_t<Char>)
				HAM_CASE(expr_kind::binding,    expr_binding_ctype_t<Char>)
				HAM_CASE(expr_kind::ref,        expr_ref_ctype_t<Char>)
				HAM_CASE(expr_kind::unresolved, expr_unresolved_ctype_t<Char>)

				HAM_CASE(expr_kind::fn,    expr_fn_ctype_t<Char>)
				HAM_CASE(expr_kind::call,  expr_call_ctype_t<Char>)
				HAM_CASE(expr_kind::block, expr_block_ctype_t<Char>)

				HAM_CASE(expr_kind::unary_op,  expr_unary_op_ctype_t<Char>)
				HAM_CASE(expr_kind::binary_op, expr_binary_op_ctype_t<Char>)

				HAM_CASE(expr_kind::lit_int,  expr_lit_int_ctype_t<Char>)
				HAM_CASE(expr_kind::lit_real, expr_lit_real_ctype_t<Char>)
				HAM_CASE(expr_kind::lit_str,  expr_lit_str_ctype_t<Char>)

			#undef HAM_CASE

				default: return nullptr;
			}
		}

		template<typename Char>
		static inline const expr_error_ctype_t<Char> *impl_parse_context_new_error(
			basic_parse_context_view<Char> ctx,
			const basic_token_range<Char> &tokens,
			const basic_expr<Char, expr_kind::base> &prev,
			const char *fmt_str,
			va_list va
		){
			va_list va1;
			va_copy(va1, va);
			int req_len = vsnprintf(nullptr, 0, fmt_str, va1);
			va_end(va1);

			if(req_len <= 0) return nullptr;

			ham::str_buffer_utf8 msg_buf(ctx.handle()->exprs.get_allocator());
			msg_buf.resize(req_len);

			vsnprintf(msg_buf.ptr(), req_len+1, fmt_str, va);

			auto new_err = ctx.template new_expr<expr_kind::error>(tokens);
			if(!new_err) return nullptr;

			new_err.set_message(msg_buf.get());
			if(!new_err.message().ptr() || !new_err.message().len()){
				fprintf(stderr, "! INTERNAL ERROR ! could not create new error message\n");
				return nullptr;
			}

			ctx.handle()->error_messages.emplace_back(std::move(msg_buf));

			return new_err;
		}

		template<typename Char>
		static inline const expr_binding_ctype_t<Char> *impl_parse_context_new_binding(
			basic_parse_context_view<Char> ctx,
			const basic_token_range<Char> &tokens,
			const basic_str<Char> &name,
			const basic_expr<Char, expr_kind::base> &type,
			const basic_expr<Char, expr_kind::base> &value
		){
			const auto new_expr = (expr_binding_ctype_t<Char>*)impl_parse_context_new_expr<Char>(ctx, expr_kind::binding, tokens);
			if(!new_expr) return nullptr;

			new_expr->name = name;
			new_expr->type = type;
			new_expr->value = value;
			return new_expr;
		}

		template<typename Char>
		static inline const expr_ref_ctype_t<Char> *impl_parse_context_new_ref(
			basic_parse_context_view<Char> ctx,
			const basic_token_range<Char> &tokens,
			const basic_expr<Char, expr_kind::binding> &refed
		){
			const auto new_expr = (expr_ref_ctype_t<Char>*)impl_parse_context_new_expr<Char>(ctx, expr_kind::ref, tokens);
			if(!new_expr) return nullptr;

			new_expr->refed = refed;
			return new_expr;
		}

		template<typename Char>
		static inline const expr_unresolved_ctype_t<Char> *impl_parse_context_new_unresolved(
			basic_parse_context_view<Char> ctx,
			const basic_token_range<Char> &tokens,
			const basic_str<Char> &id
		){
			const auto new_expr = (expr_unresolved_ctype_t<Char>*)impl_parse_context_new_expr<Char>(ctx, expr_kind::unresolved, tokens);
			if(!new_expr) return nullptr;

			new_expr->id = id;
			return new_expr;
		}

		template<typename Char>
		static inline const expr_unary_op_ctype_t<Char> *impl_parse_context_new_unary_op(
			basic_parse_context_view<Char> ctx,
			const basic_token_range<Char> &tokens,
			const basic_str<Char> &op,
			const basic_expr<Char> &expr
		){
			const auto new_expr = (expr_unary_op_ctype_t<Char>*)impl_parse_context_new_expr<Char>(ctx, expr_kind::unary_op, tokens);
			if(!new_expr) return nullptr;

			new_expr->op = op;
			new_expr->expr = expr;
			return new_expr;
		}

		template<typename Char>
		static inline const expr_binary_op_ctype_t<Char> *impl_parse_context_new_binary_op(
			basic_parse_context_view<Char> ctx,
			const basic_token_range<Char> &tokens,
			const basic_str<Char> &op,
			const basic_expr<Char> &lhs,
			const basic_expr<Char> &rhs
		){
			const auto new_expr = (expr_binary_op_ctype_t<Char>*)impl_parse_context_new_expr<Char>(ctx, expr_kind::binary_op, tokens);
			if(!new_expr) return nullptr;

			new_expr->op = op;
			new_expr->lhs = lhs;
			new_expr->rhs = rhs;
			return new_expr;
		}

		template<typename Char>
		static inline const expr_lit_int_ctype_t<Char> *impl_parse_context_new_lit_int(
			basic_parse_context_view<Char> ctx,
			const basic_token_range<Char> &tokens,
			const basic_str<Char> &value
		){
			const auto new_expr = (expr_lit_int_ctype_t<Char>*)impl_parse_context_new_expr<Char>(ctx, expr_kind::lit_int, tokens);
			if(!new_expr) return nullptr;

			if(!aint_ctype_init_str<Char>(&new_expr->value, value, 10)){
				// TODO: return error
				return nullptr;
			}

			return new_expr;
		}

		template<typename Char>
		static inline const expr_lit_real_ctype_t<Char> *impl_parse_context_new_lit_real(
			basic_parse_context_view<Char> ctx,
			basic_token_range<Char> tokens,
			basic_str<Char> value
		){
			const auto new_expr = (expr_lit_real_ctype_t<Char>*)impl_parse_context_new_expr<Char>(ctx, expr_kind::lit_real, tokens);
			if(!new_expr) return nullptr;

			if(!areal_ctype_init_str<Char>(&new_expr->value, value, 10)){
				// TODO: return error
				return nullptr;
			}

			return new_expr;
		}

		template<typename Char>
		static inline const expr_lit_str_ctype_t<Char> *impl_parse_context_new_lit_str(
			basic_parse_context_view<Char> ctx,
			basic_token_range<Char> tokens,
			basic_str<Char> value
		){
			const auto new_expr = (expr_lit_str_ctype_t<Char>*)impl_parse_context_new_expr<Char>(ctx, expr_kind::lit_str, tokens);
			if(!new_expr) return nullptr;

			new_expr->value = value;

			return new_expr;
		}

		//
		// Parsing implementation
		//

		template<typename Char>
		static inline basic_expr<Char, expr_kind::base> impl_parse_root(
			const basic_parse_scope_view<Char, true> &scope,
			const basic_token_iterator<Char> &head,
			const basic_token_range<Char> &tail
		);

		template<typename Char>
		static inline basic_expr<Char, expr_kind::base> impl_parse_leading(
			const basic_parse_scope_view<Char, true> &scope,
			const basic_expr<Char, expr_kind::base> &expr,
			const basic_token_range<Char> &tail
		);

		template<typename Char>
		static inline basic_expr<Char, expr_kind::base> impl_parse_id(
			const basic_parse_scope_view<Char, true> &scope,
			const basic_token_iterator<Char> &head,
			const basic_token_range<Char> &tail
		){
			if(head->kind() != token_kind::id) return {};

			basic_expr<Char, expr_kind::base> resolved = scope.resolve(head->str());
			if(!resolved){
				resolved = scope.context().new_unresolved(basic_token_range{ head.cptr(), head.cptr() + 1 }, head->str());
				if(!resolved){
					return nullptr;
				}
			}

			if(tail.begin() == tail.end()) return resolved;

			auto toks_it = tail.begin();
			if(toks_it->str()[0] == Char('(')){
				// TODO: implement function definitions
				return scope.context().new_error(basic_token_range{ head.cptr(), tail.end() }, resolved, "Function definitions unimplemented");
			}

			return impl_parse_leading(scope, resolved, tail);
		}

		template<typename Char>
		static inline basic_expr<Char, expr_kind::base> impl_parse_nat(
			const basic_parse_scope_view<Char, true> &scope,
			const basic_token_iterator<Char> &head,
			const basic_token_range<Char> &tail
		){
			if(head->kind() != token_kind::nat) return {};

			const auto nat_expr = scope.context().new_lit_int(basic_token_range{ head.cptr(), head.cptr() + 1 }, (*head).str());
			if(!nat_expr){
				return nullptr;
			}

			return impl_parse_leading<Char>(scope, nat_expr, tail);
		}

		template<typename Char>
		static inline basic_expr<Char, expr_kind::base> impl_parse_str(
			const basic_parse_scope_view<Char, true> &scope,
			const basic_token_iterator<Char> &head,
			const basic_token_range<Char> &tail
		){
			if(head->kind() != token_kind::str) return {};

			const auto str_expr = scope.context().new_lit_str(basic_token_range{ head.cptr(), head.cptr() + 1 }, (*head).str());
			if(!str_expr){
				// TODO: signal error
				return {};
			}

			return impl_parse_leading<Char>(scope, str_expr, tail);
		}

		template<typename Char>
		static inline basic_expr<Char, expr_kind::base> impl_parse_newline(
			const basic_parse_scope_view<Char, true> &scope,
			const basic_token_iterator<Char> &head,
			const basic_token_range<Char> &tail,
			const basic_expr<Char, expr_kind::base> &expr
		){
			if(head->kind() != token_kind::newline) return {};

			auto tail_it = tail.begin();
			while(tail_it != tail.end() && tail_it->kind() == token_kind::newline){
				++tail_it;
			}

			if(tail_it == tail.end() || tail_it->kind() == token_kind::eof) return expr; // just a bunch of newlines then eof

			return scope.context().new_error(tail, expr, "parser mostly unimplemented");
		}

		template<typename Char>
		static inline basic_expr<Char, expr_kind::base> impl_parse_call(
			const basic_parse_scope_view<Char, true> &scope,
			const basic_expr<Char, expr_kind::base> &fn_expr,
			const basic_token_range<Char> &tail
		){
			if(tail.begin() == tail.end() || tail.begin()->is_eof()) return {};

			auto tok_it = tail.begin();
			if(tok_it->kind() != token_kind::space) return fn_expr; // TODO: support newline then indented arguments

			return scope.context().new_error(tail, fn_expr, "parser mostly unimplemented");
		}

		template<typename Char>
		static inline basic_expr<Char, expr_kind::base> impl_parse_unary_op(
			const basic_parse_scope_view<Char, true> &scope,
			const basic_token_iterator<Char> &head,
			const basic_token_range<Char> &tail
		){
			if(tail.begin() == tail.end() || tail.begin()->is_eof()){
				return scope.context().new_error(basic_token_range{ head, tail.end() }, {}, "Unexpected end of source in unary operator expression.");
			}

			const auto val_expr = impl_parse_root(scope, tail.begin(), { tail.begin() + 1, tail.end() });
			if(!val_expr){
				return scope.context().new_error(basic_token_range{ head, tail.end() }, {}, "Error parsing unary operator value expression.");
			}

			return scope.context().new_error(basic_token_range{ head, tail.end() }, val_expr, "Unary operators not implemented.");
		}

		template<typename Char>
		static inline basic_expr<Char, expr_kind::base> impl_parse_binary_op(
				const basic_parse_scope_view<Char, true> &scope,
				const basic_expr<Char, expr_kind::base> &lhs_expr,
				const basic_token_iterator<Char> &head,
				const basic_token_range<Char> &tail
		){
			if(tail.begin() == tail.end() || tail.begin()->is_eof()){
				return scope.context().new_error(basic_token_range{ head, tail.end() }, lhs_expr, "Unexpected end of source in binary operator expression.");
			}

			const auto rhs_expr = impl_parse_root(scope, tail.begin(), { tail.begin() + 1, tail.end() });
			if(!rhs_expr){
				return scope.context().new_error(basic_token_range{ head, tail.end() }, lhs_expr, "Error parsing right hand side of binary operator expression.");
			}

			return scope.context().new_binary_op(basic_token_range{ head, rhs_expr.tokens().end() }, head->str(), lhs_expr, rhs_expr);
		}

		template<typename Char>
		static inline basic_expr<Char, expr_kind::base> impl_parse_leading(
			const basic_parse_scope_view<Char, true> &scope,
			const basic_expr<Char, expr_kind::base> &expr,
			const basic_token_range<Char> &tail
		){
			if(tail.begin() == tail.end() || tail.begin()->kind() == token_kind::eof) return expr;

			const auto toks_beg = tail.begin();
			const auto toks_end = tail.end();

			auto tok_it = toks_beg;

			if(tok_it->kind() == token_kind::space){
				// this is an application/call or binary operator
				do{
					++tok_it;
				} while(tok_it != toks_end && tok_it->kind() == token_kind::space); // some funny bugger could manually put multiple space tokens together

				// TODO: support newline continuation

				if(tok_it->kind() == token_kind::op){
					return impl_parse_binary_op(scope, expr, tok_it, basic_token_range{ (tok_it == toks_end ? tok_it : tok_it + 1), toks_end });
				}
				else{
					return impl_parse_call(scope, expr, basic_token_range{ tok_it, toks_end });
				}
			}
			else{
				return scope.context().new_error(tail, expr, "parser mostly unimplemented");
			}
		}

		template<typename Char>
		static inline basic_expr<Char, expr_kind::base> impl_parse_root(
			const basic_parse_scope_view<Char, true> &scope,
			const basic_token_iterator<Char> &head,
			const basic_token_range<Char> &tail
		){
			//using parse_scope_view_type = basic_parse_scope_view<Char, true>;

			const auto toks_beg = head;
			const auto toks_end = tail.end();
			auto tok_it = toks_beg;

			if(!head || tok_it == toks_end || head->kind() == token_kind::eof) return {};

			basic_str<Char> indent_str;

			if(tok_it->kind() == token_kind::space){
				//const auto indent_head = tok_it;
				indent_str = tok_it->str();
				++tok_it;

				while(tok_it < toks_end && tok_it->kind() == token_kind::space){
					indent_str = basic_str(indent_str.ptr(), indent_str.len() + tok_it->str().len());
					++tok_it;
				}

				// newline after only spaces is ignored line
				if(tok_it < toks_end && tok_it->kind() == token_kind::newline){
					++tok_it;
					if(tok_it == toks_end){
						// nothing after the empty indented line ,':^)
						return {};
					}
					else{
						// TODO: make this part not recurse
						return impl_parse_root(scope, tok_it, { tok_it + 1, toks_end });
					}
				}
			}

			const basic_token_range new_tail = { (tok_it == toks_end ? tok_it : tok_it + 1), toks_end };

			const auto cur_indent = scope.get_indent();
			if(indent_str != cur_indent){
				// ooh, we found an indented expression

				if(indent_str.len() < cur_indent.len()){
					// indent should be a subset of it's parent

					auto parent = scope.parent();
					while(true){
						if(!parent){
							return scope.context().new_error(basic_token_range{ toks_beg, tok_it }, {}, "mismatched indentation");
						}

						const auto parent_indent = parent.get_indent();

						// it ain't no subset :^(
						if(parent_indent.len() < indent_str.len()){
							return scope.context().new_error(basic_token_range{ toks_beg, tok_it }, {}, "mismatched indentation");
						}

						if(parent_indent == indent_str) break;

						parent = parent.parent();
					}

					return impl_parse_root(parent, tok_it, new_tail);
				}
				else{
					// indent should be the a superset of it's parent

					// not equal but same length: mismatch
					if(cur_indent.len() == indent_str.len()){
						return scope.context().new_error(basic_token_range{ toks_beg, tok_it }, {}, "mismatched indentation");
					}

					// lets try to create a new scope with the indentation and use that
					basic_parse_scope<Char> new_scope(scope);
					if(!new_scope){
						return scope.context().new_error(basic_token_range{ toks_beg, tok_it }, {}, "! INTERNAL ERROR ! failed to create new scope");
					}

					if(!new_scope.set_indent(indent_str)){
						return scope.context().new_error(basic_token_range{ toks_beg, tok_it }, {}, "mismatched indentation");
					}

					return impl_parse_root(new_scope.view(), tok_it, new_tail);
				}
			}

			basic_expr<Char, expr_kind::base> ret;

			if(
				!(ret = impl_parse_id(scope, tok_it, new_tail)) &&
				!(ret = impl_parse_nat(scope, tok_it, new_tail)) &&
				!(ret = impl_parse_str(scope, tok_it, new_tail))
			){
				ret = scope.context().new_error(basic_token_range{ tok_it, toks_end }, {}, "unrecognized expression");
			}

			scope.context().handle()->root_exprs.emplace_back(ret);

			return ret;
		}
	}
}

namespace ham::detail{
	// scope data members
	template<typename Char>
	struct parse_scope_data_utf{
		using str_type = basic_str<Char>;

		parse_context_ctype_t<Char> *ctx = nullptr;
		parse_scope_ctype_t<Char> *parent = nullptr;
		str_type indent;
		robin_hood::unordered_flat_map<str_type, ham::std_vector<const expr_binding_ctype_t<Char>*>> bindings;
	};

	//
	template<typename Char>
	struct parse_context_data_utf{
		ham::std_vector<expr_base_ctype_t<Char>*> exprs; // use allocator from here
		ham::std_vector<const expr_base_ctype_t<Char>*> root_exprs; // use allocator from here
		ham::std_vector<ham::str_buffer_utf8> error_messages; // parsing errors are always utf-8
		parse_scope_ctype_t<Char> root_scope;
	};
}

HAM_C_API_BEGIN

struct ham_parse_scope_utf8:  public ham::detail::parse_scope_data_utf<char8>{};
struct ham_parse_scope_utf16: public ham::detail::parse_scope_data_utf<char16>{};
struct ham_parse_scope_utf32: public ham::detail::parse_scope_data_utf<char32>{};

struct ham_parse_context_utf8:  public ham::detail::parse_context_data_utf<char8>{};
struct ham_parse_context_utf16: public ham::detail::parse_context_data_utf<char16>{};
struct ham_parse_context_utf32: public ham::detail::parse_context_data_utf<char32>{};

//
// Parse scopes
//

ham_parse_scope_utf8  *ham_parse_scope_create_utf8 (ham_parse_context_utf8  *ctx, ham_parse_scope_utf8  *parent){ return ham::detail::impl_parse_scope_create<char8> (ctx, parent); }
ham_parse_scope_utf16 *ham_parse_scope_create_utf16(ham_parse_context_utf16 *ctx, ham_parse_scope_utf16 *parent){ return ham::detail::impl_parse_scope_create<char16>(ctx, parent); }
ham_parse_scope_utf32 *ham_parse_scope_create_utf32(ham_parse_context_utf32 *ctx, ham_parse_scope_utf32 *parent){ return ham::detail::impl_parse_scope_create<char32>(ctx, parent); }

void ham_parse_scope_destroy_utf8 (ham_parse_scope_utf8 *scope) { ham::detail::impl_parse_scope_destroy<char8> (scope); }
void ham_parse_scope_destroy_utf16(ham_parse_scope_utf16 *scope){ ham::detail::impl_parse_scope_destroy<char16>(scope); }
void ham_parse_scope_destroy_utf32(ham_parse_scope_utf32 *scope){ ham::detail::impl_parse_scope_destroy<char32>(scope); }

ham_parse_scope_utf8  *ham_parse_scope_parent_utf8 (ham_parse_scope_utf8  *scope){ return ham::detail::impl_parse_scope_parent<char8> (scope); }
ham_parse_scope_utf16 *ham_parse_scope_parent_utf16(ham_parse_scope_utf16 *scope){ return ham::detail::impl_parse_scope_parent<char16>(scope); }
ham_parse_scope_utf32 *ham_parse_scope_parent_utf32(ham_parse_scope_utf32 *scope){ return ham::detail::impl_parse_scope_parent<char32>(scope); }

ham_parse_context_utf8  *ham_parse_scope_context_utf8 (const ham_parse_scope_utf8  *scope){ return ham::detail::impl_parse_scope_context<char8> (scope); }
ham_parse_context_utf16 *ham_parse_scope_context_utf16(const ham_parse_scope_utf16 *scope){ return ham::detail::impl_parse_scope_context<char16>(scope); }
ham_parse_context_utf32 *ham_parse_scope_context_utf32(const ham_parse_scope_utf32 *scope){ return ham::detail::impl_parse_scope_context<char32>(scope); }

ham_str8  ham_parse_scope_get_indent_utf8 (const ham_parse_scope_utf8  *scope){ return ham::detail::impl_parse_scope_get_indent<char8> (scope); }
ham_str16 ham_parse_scope_get_indent_utf16(const ham_parse_scope_utf16 *scope){ return ham::detail::impl_parse_scope_get_indent<char16>(scope); }
ham_str32 ham_parse_scope_get_indent_utf32(const ham_parse_scope_utf32 *scope){ return ham::detail::impl_parse_scope_get_indent<char32>(scope); }

bool ham_parse_scope_set_indent_utf8 (ham_parse_scope_utf8  *scope, ham_str8  new_indent){ return ham::detail::impl_parse_scope_set_indent<char8> (scope, new_indent); }
bool ham_parse_scope_set_indent_utf16(ham_parse_scope_utf16 *scope, ham_str16 new_indent){ return ham::detail::impl_parse_scope_set_indent<char16>(scope, new_indent); }
bool ham_parse_scope_set_indent_utf32(ham_parse_scope_utf32 *scope, ham_str32 new_indent){ return ham::detail::impl_parse_scope_set_indent<char32>(scope, new_indent); }

const ham_expr_binding_utf8  *ham_parse_scope_resolve_utf8 (const ham_parse_scope_utf8  *scope, ham_str8  name){ return ham::detail::impl_parse_scope_resolve<char8> (scope, name); }
const ham_expr_binding_utf16 *ham_parse_scope_resolve_utf16(const ham_parse_scope_utf16 *scope, ham_str16 name){ return ham::detail::impl_parse_scope_resolve<char16>(scope, name); }
const ham_expr_binding_utf32 *ham_parse_scope_resolve_utf32(const ham_parse_scope_utf32 *scope, ham_str32 name){ return ham::detail::impl_parse_scope_resolve<char32>(scope, name); }

bool ham_parse_scope_bind_utf8 (ham_parse_scope_utf8  *scope, const ham_expr_binding_utf8  *binding){ return ham::detail::impl_parse_scope_bind<char8> (scope, binding); }
bool ham_parse_scope_bind_utf16(ham_parse_scope_utf16 *scope, const ham_expr_binding_utf16 *binding){ return ham::detail::impl_parse_scope_bind<char16>(scope, binding); }
bool ham_parse_scope_bind_utf32(ham_parse_scope_utf32 *scope, const ham_expr_binding_utf32 *binding){ return ham::detail::impl_parse_scope_bind<char32>(scope, binding); }

//
// Parse contexts
//

ham_parse_context_utf8  *ham_parse_context_create_utf8 (){ return ham::detail::impl_parse_context_create<char8> (); }
ham_parse_context_utf16 *ham_parse_context_create_utf16(){ return ham::detail::impl_parse_context_create<char16>(); }
ham_parse_context_utf32 *ham_parse_context_create_utf32(){ return ham::detail::impl_parse_context_create<char32>(); }

void ham_parse_context_destroy_utf8 (ham_parse_context_utf8  *ctx){ ham::detail::impl_parse_context_destroy<char8> (ctx); }
void ham_parse_context_destroy_utf16(ham_parse_context_utf16 *ctx){ ham::detail::impl_parse_context_destroy<char16>(ctx); }
void ham_parse_context_destroy_utf32(ham_parse_context_utf32 *ctx){ ham::detail::impl_parse_context_destroy<char32>(ctx); }

// generic expression creation

ham_expr_base_utf8 *ham_parse_context_new_expr_utf8(ham_parse_context_utf8 *ctx, ham_expr_kind kind, ham_token_range_utf8 tokens){
	return ham::detail::impl_parse_context_new_expr<char8>(ctx, static_cast<ham::expr_kind>(kind), tokens);
}

ham_expr_base_utf16 *ham_parse_context_new_expr_utf16(ham_parse_context_utf16 *ctx, ham_expr_kind kind, ham_token_range_utf16 tokens){
	return ham::detail::impl_parse_context_new_expr<char16>(ctx, static_cast<ham::expr_kind>(kind), tokens);
}

ham_expr_base_utf32 *ham_parse_context_new_expr_utf32(ham_parse_context_utf32 *ctx, ham_expr_kind kind, ham_token_range_utf32 tokens){
	return ham::detail::impl_parse_context_new_expr<char32>(ctx, static_cast<ham::expr_kind>(kind), tokens);
}

// errors

const ham_expr_error_utf8 *ham_parse_context_new_error_utf8(ham_parse_context_utf8 *ctx, ham_token_range_utf8 tokens, const ham_expr_base_utf8 *prev, const char *fmt_str, ...){
	va_list va;
	va_start(va, fmt_str);
	const auto ret = ham::detail::impl_parse_context_new_error<char8>(ctx, tokens, prev, fmt_str, va);
	va_end(va);
	return ret;
}

const ham_expr_error_utf16 *ham_parse_context_new_error_utf16(ham_parse_context_utf16 *ctx, ham_token_range_utf16 tokens, const ham_expr_base_utf16 *prev, const char *fmt_str, ...){
	va_list va;
	va_start(va, fmt_str);
	const auto ret = ham::detail::impl_parse_context_new_error<char16>(ctx, tokens, prev, fmt_str, va);
	va_end(va);
	return ret;
}

const ham_expr_error_utf32 *ham_parse_context_new_error_utf32(ham_parse_context_utf32 *ctx, ham_token_range_utf32 tokens, const ham_expr_base_utf32 *prev, const char *fmt_str, ...){
	va_list va;
	va_start(va, fmt_str);
	const auto ret = ham::detail::impl_parse_context_new_error<char32>(ctx, tokens, prev, fmt_str, va);
	va_end(va);
	return ret;
}

// bindings

const ham_expr_binding_utf8  *ham_parse_context_new_binding_utf8 (ham_parse_context_utf8  *ctx, ham_token_range_utf8  tokens, ham_str8  name, const ham_expr_base_utf8  *type, const ham_expr_base_utf8  *value){
	return ham::detail::impl_parse_context_new_binding<char8>(ctx, tokens, name, type, value);
}

const ham_expr_binding_utf16 *ham_parse_context_new_binding_utf16(ham_parse_context_utf16 *ctx, ham_token_range_utf16 tokens, ham_str16 name, const ham_expr_base_utf16 *type, const ham_expr_base_utf16 *value){
	return ham::detail::impl_parse_context_new_binding<char16>(ctx, tokens, name, type, value);
}

const ham_expr_binding_utf32 *ham_parse_context_new_binding_utf32(ham_parse_context_utf32 *ctx, ham_token_range_utf32 tokens, ham_str32 name, const ham_expr_base_utf32 *type, const ham_expr_base_utf32 *value){
	return ham::detail::impl_parse_context_new_binding<char32>(ctx, tokens, name, type, value);
}

// refs

const ham_expr_ref_utf8  *ham_parse_context_new_ref_utf8 (ham_parse_context_utf8  *ctx, ham_token_range_utf8  tokens, const ham_expr_binding_utf8  *refed){ return ham::detail::impl_parse_context_new_ref<char8> (ctx, tokens, refed); }
const ham_expr_ref_utf16 *ham_parse_context_new_ref_utf16(ham_parse_context_utf16 *ctx, ham_token_range_utf16 tokens, const ham_expr_binding_utf16 *refed){ return ham::detail::impl_parse_context_new_ref<char16>(ctx, tokens, refed); }
const ham_expr_ref_utf32 *ham_parse_context_new_ref_utf32(ham_parse_context_utf32 *ctx, ham_token_range_utf32 tokens, const ham_expr_binding_utf32 *refed){ return ham::detail::impl_parse_context_new_ref<char32>(ctx, tokens, refed); }

// unresolved refs

const ham_expr_unresolved_utf8  *ham_parse_context_new_unresolved_utf8 (ham_parse_context_utf8  *ctx, ham_token_range_utf8  tokens, ham_str8  id){ return ham::detail::impl_parse_context_new_unresolved<char8> (ctx, tokens, id); }
const ham_expr_unresolved_utf16 *ham_parse_context_new_unresolved_utf16(ham_parse_context_utf16 *ctx, ham_token_range_utf16 tokens, ham_str16 id){ return ham::detail::impl_parse_context_new_unresolved<char16>(ctx, tokens, id); }
const ham_expr_unresolved_utf32 *ham_parse_context_new_unresolved_utf32(ham_parse_context_utf32 *ctx, ham_token_range_utf32 tokens, ham_str32 id){ return ham::detail::impl_parse_context_new_unresolved<char32>(ctx, tokens, id); }

// unary ops

const ham_expr_unary_op_utf8  *ham_parse_context_new_unary_op_utf8 (ham_parse_context_utf8  *ctx, ham_token_range_utf8  tokens, ham_str8  op, const ham_expr_base_utf8  *expr){ return ham::detail::impl_parse_context_new_unary_op<char8> (ctx, tokens, op, expr); }
const ham_expr_unary_op_utf16 *ham_parse_context_new_unary_op_utf16(ham_parse_context_utf16 *ctx, ham_token_range_utf16 tokens, ham_str16 op, const ham_expr_base_utf16 *expr){ return ham::detail::impl_parse_context_new_unary_op<char16>(ctx, tokens, op, expr); }
const ham_expr_unary_op_utf32 *ham_parse_context_new_unary_op_utf32(ham_parse_context_utf32 *ctx, ham_token_range_utf32 tokens, ham_str32 op, const ham_expr_base_utf32 *expr){ return ham::detail::impl_parse_context_new_unary_op<char32>(ctx, tokens, op, expr); }

// binary ops

const ham_expr_binary_op_utf8 *ham_parse_context_new_binary_op_utf8 (ham_parse_context_utf8  *ctx, ham_token_range_utf8  tokens, ham_str8 op, const ham_expr_base_utf8 *lhs, const ham_expr_base_utf8 *rhs){
	return ham::detail::impl_parse_context_new_binary_op<char8> (ctx, tokens, op, lhs, rhs);
}

const ham_expr_binary_op_utf16 *ham_parse_context_new_binary_op_utf16(ham_parse_context_utf16 *ctx, ham_token_range_utf16 tokens, ham_str16 op, const ham_expr_base_utf16 *lhs, const ham_expr_base_utf16 *rhs){
	return ham::detail::impl_parse_context_new_binary_op<char16>(ctx, tokens, op, lhs, rhs);
}

const ham_expr_binary_op_utf32 *ham_parse_context_new_binary_op_utf32(ham_parse_context_utf32 *ctx, ham_token_range_utf32 tokens, ham_str32 op, const ham_expr_base_utf32 *lhs, const ham_expr_base_utf32 *rhs){
	return ham::detail::impl_parse_context_new_binary_op<char32>(ctx, tokens, op, lhs, rhs);
}

// literal ints

const ham_expr_lit_int_utf8  *ham_parse_context_new_lit_int_utf8 (ham_parse_context_utf8  *ctx, ham_token_range_utf8  tokens, ham_str8  value){ return ham::detail::impl_parse_context_new_lit_int<char8> (ctx, tokens, value); }
const ham_expr_lit_int_utf16 *ham_parse_context_new_lit_int_utf16(ham_parse_context_utf16 *ctx, ham_token_range_utf16 tokens, ham_str16 value){ return ham::detail::impl_parse_context_new_lit_int<char16>(ctx, tokens, value); }
const ham_expr_lit_int_utf32 *ham_parse_context_new_lit_int_utf32(ham_parse_context_utf32 *ctx, ham_token_range_utf32 tokens, ham_str32 value){ return ham::detail::impl_parse_context_new_lit_int<char32>(ctx, tokens, value); }

// literal reals

const ham_expr_lit_real_utf8  *ham_parse_context_new_lit_real_utf8 (ham_parse_context_utf8  *ctx, ham_token_range_utf8  tokens, ham_str8  value){ return ham::detail::impl_parse_context_new_lit_real<char8> (ctx, tokens, value); }
const ham_expr_lit_real_utf16 *ham_parse_context_new_lit_real_utf16(ham_parse_context_utf16 *ctx, ham_token_range_utf16 tokens, ham_str16 value){ return ham::detail::impl_parse_context_new_lit_real<char16>(ctx, tokens, value); }
const ham_expr_lit_real_utf32 *ham_parse_context_new_lit_real_utf32(ham_parse_context_utf32 *ctx, ham_token_range_utf32 tokens, ham_str32 value){ return ham::detail::impl_parse_context_new_lit_real<char32>(ctx, tokens, value); }

// literal reals

const ham_expr_lit_str_utf8  *ham_parse_context_new_lit_str_utf8 (ham_parse_context_utf8  *ctx, ham_token_range_utf8  tokens, ham_str8  value){ return ham::detail::impl_parse_context_new_lit_str<char8> (ctx, tokens, value); }
const ham_expr_lit_str_utf16 *ham_parse_context_new_lit_str_utf16(ham_parse_context_utf16 *ctx, ham_token_range_utf16 tokens, ham_str16 value){ return ham::detail::impl_parse_context_new_lit_str<char16>(ctx, tokens, value); }
const ham_expr_lit_str_utf32 *ham_parse_context_new_lit_str_utf32(ham_parse_context_utf32 *ctx, ham_token_range_utf32 tokens, ham_str32 value){ return ham::detail::impl_parse_context_new_lit_str<char32>(ctx, tokens, value); }

// parsing

const ham_expr_base_utf8 *ham_parse_utf8 (ham_parse_context_utf8 *ctx, ham_token_range_utf8 tokens){
	return ham::detail::impl_parse_root<char8>(&ctx->root_scope, tokens.beg, ham::basic_token_range{ (tokens.beg == tokens.end ? tokens.end : tokens.beg + 1), tokens.end });
}

const ham_expr_base_utf16 *ham_parse_utf16(ham_parse_context_utf16 *ctx, ham_token_range_utf16 tokens){
	return ham::detail::impl_parse_root<char16>(&ctx->root_scope, tokens.beg, ham::basic_token_range{ (tokens.beg == tokens.end ? tokens.end : tokens.beg + 1), tokens.end });
}

const ham_expr_base_utf32 *ham_parse_utf32(ham_parse_context_utf32 *ctx, ham_token_range_utf32 tokens){
	return ham::detail::impl_parse_root<char32>(&ctx->root_scope, tokens.beg, ham::basic_token_range{ (tokens.beg == tokens.end ? tokens.end : tokens.beg + 1), tokens.end });
}

HAM_C_API_END
