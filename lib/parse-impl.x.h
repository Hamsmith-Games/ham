#ifndef HAM_PARSE_IMPL_X_H
#define HAM_PARSE_IMPL_X_H 1

#ifndef HAM_PARSE_IMPL_X_UTF
#	error "HAM_PARSE_IMPL_X_UTF not defined before including parse-impl.x.h"
#endif

// types

#define HAM_PARSE_IMPL_X_CHAR HAM_CHAR_UTF(HAM_PARSE_IMPL_X_UTF)
#define HAM_PARSE_IMPL_X_STR HAM_STR_UTF(HAM_PARSE_IMPL_X_UTF)
#define HAM_PARSE_IMPL_X_TOKEN HAM_TOKEN_UTF(HAM_PARSE_IMPL_X_UTF)
#define HAM_PARSE_IMPL_X_TOKEN_RANGE HAM_TOKEN_RANGE_UTF(HAM_PARSE_IMPL_X_UTF)

#define HAM_PARSE_IMPL_X_EMPTY_STR HAM_EMPTY_STR_UTF(HAM_PARSE_IMPL_X_UTF)

#define HAM_PARSE_IMPL_X_PARSE_CONTEXT HAM_PARSE_CONTEXT_UTF(HAM_PARSE_IMPL_X_UTF)
#define HAM_PARSE_IMPL_X_PARSE_SCOPE HAM_PARSE_SCOPE_UTF(HAM_PARSE_IMPL_X_UTF)

#define HAM_PARSE_IMPL_X_EXPR_BASE HAM_EXPR_UTF(HAM_PARSE_IMPL_X_UTF, base)
#define HAM_PARSE_IMPL_X_EXPR_ERROR HAM_EXPR_UTF(HAM_PARSE_IMPL_X_UTF, error)
#define HAM_PARSE_IMPL_X_EXPR_BINDING HAM_EXPR_UTF(HAM_PARSE_IMPL_X_UTF, binding)
#define HAM_PARSE_IMPL_X_EXPR_REF HAM_EXPR_UTF(HAM_PARSE_IMPL_X_UTF, ref)
#define HAM_PARSE_IMPL_X_EXPR_UNRESOLVED HAM_EXPR_UTF(HAM_PARSE_IMPL_X_UTF, unresolved)
#define HAM_PARSE_IMPL_X_EXPR_LIT_INT HAM_EXPR_UTF(HAM_PARSE_IMPL_X_UTF, lit_int)
#define HAM_PARSE_IMPL_X_EXPR_LIT_REAL HAM_EXPR_UTF(HAM_PARSE_IMPL_X_UTF, lit_real)
#define HAM_PARSE_IMPL_X_EXPR_LIT_STR HAM_EXPR_UTF(HAM_PARSE_IMPL_X_UTF, lit_str)

#define HAM_PARSE_IMPL_X_AINT_INIT_STR HAM_AINT_INIT_STR_UTF(HAM_PARSE_IMPL_X_UTF)

// functions

//#define HAM_PARSE_IMPL_X_PARSE_FN(name) HAM_CONCAT(ham_parse_, name), HAM_PARSE_IMPL_X_UTF);

#define HAM_PARSE_IMPL_X_PARSE_CONTEXT_CREATE HAM_PARSE_CONTEXT_CREATE_UTF(HAM_PARSE_IMPL_X_UTF)
#define HAM_PARSE_IMPL_X_PARSE_CONTEXT_DESTROY HAM_PARSE_CONTEXT_DESTROY_UTF(HAM_PARSE_IMPL_X_UTF)
#define HAM_PARSE_IMPL_X_PARSE_CONTEXT_NEW_EXPR HAM_PARSE_CONTEXT_NEW_EXPR_UTF(HAM_PARSE_IMPL_X_UTF)
#define HAM_PARSE_IMPL_X_PARSE_CONTEXT_NEW_ERROR HAM_PARSE_CONTEXT_NEW_ERROR_UTF(HAM_PARSE_IMPL_X_UTF)
#define HAM_PARSE_IMPL_X_PARSE HAM_PARSE_UTF(HAM_PARSE_IMPL_X_UTF)

#define HAM_PARSE_IMPL_X_PARSE_SCOPE_CREATE HAM_PARSE_SCOPE_CREATE_UTF(HAM_PARSE_IMPL_X_UTF)
#define HAM_PARSE_IMPL_X_PARSE_SCOPE_DESTROY HAM_PARSE_SCOPE_DESTROY_UTF(HAM_PARSE_IMPL_X_UTF)
#define HAM_PARSE_IMPL_X_PARSE_SCOPE_BIND HAM_PARSE_SCOPE_BIND_UTF(HAM_PARSE_IMPL_X_UTF)
#define HAM_PARSE_IMPL_X_PARSE_SCOPE_RESOLVE HAM_PARSE_SCOPE_RESOLVE_UTF(HAM_PARSE_IMPL_X_UTF)

HAM_C_API_BEGIN

struct HAM_PARSE_IMPL_X_PARSE_SCOPE{
	HAM_PARSE_IMPL_X_PARSE_CONTEXT *ctx = nullptr;
	const HAM_PARSE_IMPL_X_PARSE_SCOPE *parent = nullptr;
	ham::basic_str<HAM_PARSE_IMPL_X_CHAR> indent;
	robin_hood::unordered_flat_map<ham::basic_str<HAM_PARSE_IMPL_X_CHAR>, ham::std_vector<const HAM_PARSE_IMPL_X_EXPR_BINDING*>> bindings;
};

struct HAM_PARSE_IMPL_X_PARSE_CONTEXT{
	ham::std_vector<HAM_PARSE_IMPL_X_EXPR_BASE*> exprs; // use allocator from here
	HAM_PARSE_IMPL_X_PARSE_SCOPE root_scope;
};

HAM_C_API_END

namespace ham{
	template<typename T, typename ... Args>
	static T *context_new(HAM_PARSE_IMPL_X_PARSE_CONTEXT *ctx, Args &&... args){
		const ham::allocator<T> allocator = ctx->exprs.get_allocator();
		const auto mem = allocator.allocate(1);
		return mem ? allocator.construct(mem, std::forward<Args>(args)...) : nullptr;
	}

	template<typename T>
	static void context_delete(HAM_PARSE_IMPL_X_PARSE_CONTEXT *ctx, T *ptr){
		const ham::allocator<T> allocator = ctx->exprs.get_allocator();
		allocator.destroy(ptr);
		allocator.deallocate(ptr, 1);
	}
}

HAM_C_API_BEGIN

HAM_PARSE_IMPL_X_EXPR_BASE *HAM_PARSE_IMPL_X_PARSE_CONTEXT_NEW_EXPR(
	HAM_PARSE_IMPL_X_PARSE_CONTEXT *ctx,
	ham_expr_kind kind,
	HAM_PARSE_IMPL_X_TOKEN_RANGE tokens
){
	HAM_PARSE_IMPL_X_EXPR_BASE *ret = nullptr;

	switch(kind){
	#define HAM_CASE(kind_) case (HAM_CONCAT(HAM_EXPR_, kind_)):{ \
		ret = (HAM_PARSE_IMPL_X_EXPR_BASE*)ham::context_new<HAM_CONCAT(HAM_PARSE_IMPL_X_EXPR_, kind_)>(ctx); \
		ret->kind = kind; \
		ret->tokens = tokens; \
		break; }

		HAM_CASE(ERROR)
		HAM_CASE(BINDING)
		HAM_CASE(REF)
		HAM_CASE(UNRESOLVED)
		HAM_CASE(LIT_INT)
		HAM_CASE(LIT_REAL)
		HAM_CASE(LIT_STR)

	#undef HAM_CASE
		default: break;
	}

	return ret;
}

HAM_C_API_END

namespace ham{
	const HAM_PARSE_IMPL_X_EXPR_BINDING *scope_resolve(const HAM_PARSE_IMPL_X_PARSE_SCOPE *scope, HAM_PARSE_IMPL_X_STR name){
		const auto res = scope->bindings.find(name);
		if(res != scope->bindings.end()) return res->second.back();
		else return scope->parent ? scope_resolve(scope->parent, name) : nullptr;
	}

// 	const HAM_PARSE_IMPL_X_EXPR_BASE *parse_id(
// 		HAM_PARSE_IMPL_X_PARSE_CONTEXT *ctx,
// 		const HAM_PARSE_IMPL_X_STR id,
// 		const HAM_PARSE_IMPL_X_TOKEN *it,
// 		const HAM_PARSE_IMPL_X_TOKEN *end
// 	){
//
// 	}

	static const HAM_PARSE_IMPL_X_EXPR_BASE *parse_leading(
		HAM_PARSE_IMPL_X_PARSE_SCOPE *scope,
		const HAM_PARSE_IMPL_X_EXPR_BASE *value,
		HAM_PARSE_IMPL_X_TOKEN_RANGE tail
	);

	static const HAM_PARSE_IMPL_X_EXPR_BASE *parse_id(
		HAM_PARSE_IMPL_X_PARSE_SCOPE *scope,
		const HAM_PARSE_IMPL_X_TOKEN *it,
		HAM_PARSE_IMPL_X_TOKEN_RANGE tail
	){
		if(it->kind != HAM_TOKEN_ID) return nullptr;

		const HAM_PARSE_IMPL_X_EXPR_BASE *ref = (const HAM_PARSE_IMPL_X_EXPR_BASE*)scope_resolve(scope, it->str);
		if(!ref){
			const auto unresolved = context_new<HAM_PARSE_IMPL_X_EXPR_UNRESOLVED>(scope->ctx);
			unresolved->super.kind = HAM_EXPR_UNRESOLVED;
			unresolved->super.tokens = { it, tail.beg };
			unresolved->id = it->str;

			ref = &unresolved->super;
		}

		return parse_leading(scope, ref, tail);
	}

	static const HAM_PARSE_IMPL_X_EXPR_BASE *parse_nat(
		HAM_PARSE_IMPL_X_PARSE_SCOPE *scope,
		const HAM_PARSE_IMPL_X_TOKEN *it,
		HAM_PARSE_IMPL_X_TOKEN_RANGE tail
	){
		if(it->kind != HAM_TOKEN_NAT) return nullptr;

		const auto lit_int = context_new<HAM_PARSE_IMPL_X_EXPR_LIT_INT>(scope->ctx);

		lit_int->super.kind = HAM_EXPR_LIT_INT;
		lit_int->super.tokens = { it, tail.beg };

		if(!HAM_PARSE_IMPL_X_AINT_INIT_STR(&lit_int->value, it->str, 10)){
			context_delete(scope->ctx, lit_int);
			return nullptr;
		}

		return parse_leading(scope, &lit_int->super, tail);
	}

	static const HAM_PARSE_IMPL_X_EXPR_BASE *parse_str(
		HAM_PARSE_IMPL_X_PARSE_SCOPE *scope,
		const HAM_PARSE_IMPL_X_TOKEN *it,
		HAM_PARSE_IMPL_X_TOKEN_RANGE tail
	){
		if(it->kind != HAM_TOKEN_STR) return nullptr;

		const auto lit_str = context_new<HAM_PARSE_IMPL_X_EXPR_LIT_STR>(scope->ctx);

		lit_str->super.kind = HAM_EXPR_LIT_STR;
		lit_str->super.tokens = { it, tail.beg };

		lit_str->value = it->str;

		return parse_leading(scope, &lit_str->super, tail);
	}

	static const HAM_PARSE_IMPL_X_EXPR_BASE *parse_leading(
		HAM_PARSE_IMPL_X_PARSE_SCOPE *scope,
		const HAM_PARSE_IMPL_X_EXPR_BASE *value,
		HAM_PARSE_IMPL_X_TOKEN_RANGE tail
	){
		if(tail.beg == tail.end) return value;

		return nullptr;
	}
}

HAM_C_API_BEGIN

HAM_PARSE_IMPL_X_PARSE_SCOPE *HAM_PARSE_IMPL_X_PARSE_SCOPE_CREATE(
	HAM_PARSE_IMPL_X_PARSE_CONTEXT *ctx,
	const HAM_PARSE_IMPL_X_PARSE_SCOPE *parent
){
	if(!ctx) return nullptr;

	const auto allocator = ctx->exprs.get_allocator();

	const auto mem = allocator.allocate<HAM_PARSE_IMPL_X_PARSE_SCOPE>();
	if(!mem) return nullptr;

	const auto ptr = allocator.construct(mem);
	if(!ptr){
		allocator.deallocate(mem);
		return nullptr;
	}

	ptr->ctx = ctx;
	ptr->parent = parent;

	return ptr;
}

void HAM_PARSE_IMPL_X_PARSE_SCOPE_DESTROY(HAM_PARSE_IMPL_X_PARSE_SCOPE *scope){
	if(!scope) return;

	const auto allocator = scope->ctx->exprs.get_allocator();

	allocator.destroy(scope);
	allocator.deallocate(scope);
}

bool HAM_PARSE_IMPL_X_PARSE_SCOPE_BIND(
	HAM_PARSE_IMPL_X_PARSE_SCOPE *scope,
	const HAM_PARSE_IMPL_X_EXPR_BINDING *binding
){
	const auto find_res = scope->bindings.find(binding->name);
	if(find_res != scope->bindings.end()){
		find_res->second.emplace_back(binding);
	}
	else{
		scope->bindings[binding->name] = ham::std_vector<const HAM_PARSE_IMPL_X_EXPR_BINDING*>({ binding }, scope->ctx->exprs.get_allocator());
	}

	return true;
}

const HAM_PARSE_IMPL_X_EXPR_BINDING *HAM_PARSE_IMPL_X_PARSE_SCOPE_RESOLVE(
	const HAM_PARSE_IMPL_X_PARSE_SCOPE *scope,
	HAM_PARSE_IMPL_X_STR name
){
	if(!scope) return nullptr;

	const auto res = scope->bindings.find(name);
	return (res != scope->bindings.end()) ? res->second.back() : HAM_PARSE_IMPL_X_PARSE_SCOPE_RESOLVE(scope->parent, name);
}

HAM_PARSE_IMPL_X_PARSE_CONTEXT *HAM_PARSE_IMPL_X_PARSE_CONTEXT_CREATE(){
	const ham::allocator<HAM_PARSE_IMPL_X_PARSE_CONTEXT> allocator;

	const auto mem = allocator.allocate();
	if(!mem) return nullptr;

	const auto ret = allocator.construct(mem);
	if(!ret){
		allocator.deallocate(mem);
		return nullptr;
	}

	ret->exprs = ham::std_vector<HAM_PARSE_IMPL_X_EXPR_BASE*>(ham::allocator<HAM_PARSE_IMPL_X_EXPR_BASE*>(allocator));
	ret->root_scope.ctx = ret;

	return ret;
}

void HAM_PARSE_IMPL_X_PARSE_CONTEXT_DESTROY(HAM_PARSE_IMPL_X_PARSE_CONTEXT *ctx){
	if(!ctx) return;
	const auto allocator = ctx->exprs.get_allocator();

	for(auto expr : ctx->exprs){
		// these objects do not need destroying as they are POD
		switch(expr->kind){
			case HAM_EXPR_LIT_INT:{
				const auto lit_int = (HAM_PARSE_IMPL_X_EXPR_LIT_INT*)expr;
				ham_aint_finish(&lit_int->value);
				allocator.deallocate(lit_int);
				break;
			}

			case HAM_EXPR_LIT_REAL:{
				const auto lit_real = (HAM_PARSE_IMPL_X_EXPR_LIT_REAL*)expr;
				ham_areal_finish(&lit_real->value);
				allocator.deallocate(lit_real);
				break;
			}

			default: allocator.deallocate(expr); break;
		}
	}

	allocator.destroy(ctx);
	allocator.deallocate(ctx);
}

const HAM_PARSE_IMPL_X_EXPR_BASE *HAM_PARSE_IMPL_X_PARSE(
	HAM_PARSE_IMPL_X_PARSE_CONTEXT *ctx,
	HAM_PARSE_IMPL_X_PARSE_SCOPE *scope,
	HAM_PARSE_IMPL_X_TOKEN_RANGE tokens
){
	if(tokens.beg == tokens.end) return nullptr;

	using tok_iter  = const HAM_PARSE_IMPL_X_TOKEN*;
	using tok_range = HAM_PARSE_IMPL_X_TOKEN_RANGE;

	const HAM_PARSE_IMPL_X_EXPR_BASE *ret = nullptr;

	const tok_iter  tok_beg = tokens.beg;
	const tok_range tail    = { tokens.beg + 1, tokens.end };

	if(!scope) scope = &ctx->root_scope;

	return
		(
			(ret = ham::parse_id (scope, tok_beg, tail)) ||
			(ret = ham::parse_nat(scope, tok_beg, tail)) ||
			(ret = ham::parse_str(scope, tok_beg, tail))
		)
		? ret
		: nullptr
	;
}

HAM_C_API_END

#undef HAM_PARSE_IMPL_X_CHAR
#undef HAM_PARSE_IMPL_X_STR
#undef HAM_PARSE_IMPL_X_TOKEN
#undef HAM_PARSE_IMPL_X_TOKEN_RANGE

#undef HAM_PARSE_IMPL_X_EMPTY_STR

#undef HAM_PARSE_IMPL_X_EXPR_BASE
#undef HAM_PARSE_IMPL_X_EXPR_ERROR
#undef HAM_PARSE_IMPL_X_EXPR_BINDING
#undef HAM_PARSE_IMPL_X_EXPR_REF
#undef HAM_PARSE_IMPL_X_EXPR_UNRESOLVED
#undef HAM_PARSE_IMPL_X_EXPR_LIT_INT
#undef HAM_PARSE_IMPL_X_EXPR_LIT_REAL
#undef HAM_PARSE_IMPL_X_EXPR_LIT_STR

#undef HAM_PARSE_IMPL_X_AINT_INIT_STR

#undef HAM_PARSE_IMPL_X_PARSE
#undef HAM_PARSE_IMPL_X_PARSE_CONTEXT_NEW_ERROR
#undef HAM_PARSE_IMPL_X_PARSE_CONTEXT_NEW_EXPR
#undef HAM_PARSE_IMPL_X_PARSE_CONTEXT_DESTROY
#undef HAM_PARSE_IMPL_X_PARSE_CONTEXT_CREATE

#undef HAM_PARSE_IMPL_X_PARSE_SCOPE_RESOLVE
#undef HAM_PARSE_IMPL_X_PARSE_SCOPE_BIND
#undef HAM_PARSE_IMPL_X_PARSE_SCOPE_DESTROY
#undef HAM_PARSE_IMPL_X_PARSE_SCOPE_CREATE

#undef HAM_PARSE_IMPL_X_PARSE_CONTEXT
#undef HAM_PARSE_IMPL_X_PARSE_SCOPE

#undef HAM_PARSE_IMPL_X_UTF
#undef HAM_PARSE_IMPL_X_H

#endif // !HAM_PARSE_IMPL_X_H
