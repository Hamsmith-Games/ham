#ifndef HAM_PARSE_IMPL_X_H
#define HAM_PARSE_IMPL_X_H 1

#ifndef HAM_PARSE_IMPL_X_UTF
#	error "HAM_PARSE_IMPL_X_UTF not defined before including parse-impl.x.h"
#endif

// types

#define HAM_PARSE_IMPL_X_CHAR HAM_CHAR_UTF(HAM_PARSE_IMPL_X_UTF)
#define HAM_PARSE_IMPL_X_STR HAM_STR_UTF(HAM_PARSE_IMPL_X_UTF)
#define HAM_PARSE_IMPL_X_TOKEN HAM_TOKEN_UTF(HAM_PARSE_IMPL_X_UTF)

#define HAM_PARSE_IMPL_X_PARSE_CONTEXT HAM_PARSE_CONTEXT_UTF(HAM_PARSE_IMPL_X_UTF)

#define HAM_PARSE_IMPL_X_EXPR_BASE HAM_EXPR_UTF(HAM_PARSE_IMPL_X_UTF, base)
#define HAM_PARSE_IMPL_X_EXPR_BINDING HAM_EXPR_UTF(HAM_PARSE_IMPL_X_UTF, binding)
#define HAM_PARSE_IMPL_X_EXPR_REF HAM_EXPR_UTF(HAM_PARSE_IMPL_X_UTF, ref)

// functions

//#define HAM_PARSE_IMPL_X_PARSE_FN(name) HAM_CONCAT(ham_parse_, name), HAM_PARSE_IMPL_X_UTF);

#define HAM_PARSE_IMPL_X_PARSE_CONTEXT_CREATE HAM_PARSE_CONTEXT_CREATE_UTF(HAM_PARSE_IMPL_X_UTF)
#define HAM_PARSE_IMPL_X_PARSE_CONTEXT_DESTROY HAM_PARSE_CONTEXT_DESTROY_UTF(HAM_PARSE_IMPL_X_UTF)
#define HAM_PARSE_IMPL_X_PARSE HAM_PARSE_UTF(HAM_PARSE_IMPL_X_UTF)

HAM_C_API_BEGIN

struct HAM_PARSE_IMPL_X_PARSE_CONTEXT{
	ham::std_vector<HAM_PARSE_IMPL_X_EXPR_BASE*> exprs; // use allocator from here
	robin_hood::unordered_flat_map<ham::basic_str<HAM_PARSE_IMPL_X_CHAR>, HAM_PARSE_IMPL_X_EXPR_BINDING*> bindings;
};

HAM_C_API_END

namespace ham{
	template<typename T, typename ... Args>
	T *context_new(HAM_PARSE_IMPL_X_PARSE_CONTEXT *ctx, Args &&... args){
		const ham::allocator<T> allocator = ctx->exprs.get_allocator();
		const auto mem = allocator.allocate(1);
		return mem ? allocator.construct(mem, std::forward<Args>(args)...) : nullptr;
	}

	template<typename T>
	void context_delete(HAM_PARSE_IMPL_X_PARSE_CONTEXT *ctx, T *ptr){
		const ham::allocator<T> allocator = ctx->exprs.get_allocator();
		allocator.destroy(ptr);
		allocator.deallocate(ptr, 1);
	}

// 	const HAM_PARSE_IMPL_X_EXPR_BASE *parse_id(
// 		HAM_PARSE_IMPL_X_PARSE_CONTEXT *ctx,
// 		const HAM_PARSE_IMPL_X_STR id,
// 		const HAM_PARSE_IMPL_X_TOKEN *it,
// 		const HAM_PARSE_IMPL_X_TOKEN *end
// 	){
//
// 	}

	const HAM_PARSE_IMPL_X_EXPR_BASE *parse_nat(
		HAM_PARSE_IMPL_X_PARSE_CONTEXT *ctx,
		const HAM_PARSE_IMPL_X_STR nat,
		const HAM_PARSE_IMPL_X_TOKEN *it,
		const HAM_PARSE_IMPL_X_TOKEN *end
	){


		return nullptr;
	}
}

HAM_C_API_BEGIN

HAM_PARSE_IMPL_X_PARSE_CONTEXT *HAM_PARSE_IMPL_X_PARSE_CONTEXT_CREATE(){
	const ham::allocator<HAM_PARSE_IMPL_X_PARSE_CONTEXT> allocator;

	const auto mem = allocator.allocate();
	if(!mem) return nullptr;

	const auto ret = allocator.construct(mem);
	ret->exprs = ham::std_vector<HAM_PARSE_IMPL_X_EXPR_BASE*>(ham::allocator<HAM_PARSE_IMPL_X_EXPR_BASE*>(allocator));

	return ret;
}

void HAM_PARSE_IMPL_X_PARSE_CONTEXT_DESTROY(HAM_PARSE_IMPL_X_PARSE_CONTEXT *ctx){
	if(!ctx) return;
	const auto allocator = ham::allocator<HAM_PARSE_IMPL_X_PARSE_CONTEXT>(ctx->exprs.get_allocator());

	allocator.destroy(ctx);
	allocator.deallocate(ctx);
}

const HAM_PARSE_IMPL_X_EXPR_BASE *HAM_PARSE_IMPL_X_PARSE(
	HAM_PARSE_IMPL_X_PARSE_CONTEXT *ctx,
	const HAM_PARSE_IMPL_X_TOKEN *tok_beg, const HAM_PARSE_IMPL_X_TOKEN *tok_end
){
	using context = HAM_PARSE_IMPL_X_PARSE_CONTEXT;
	using str = HAM_STR_UTF(HAM_PARSE_IMPL_X_UTF);
	using tok_iter = const HAM_PARSE_IMPL_X_TOKEN*;

	using expr_base = HAM_PARSE_IMPL_X_EXPR_BASE;

	using parse_fn = const expr_base*(*)(context *ctx, str id, tok_iter it, const tok_iter end);

	constexpr static auto make_expr = []<class T>(ham::type_tag<T>, context *ctx) -> T*{
		const auto allocator = ham::allocator<T>(ctx->exprs.get_allocator());
		const auto mem = allocator.allocate();
		return mem ? allocator.construct(mem) : nullptr;
	};

	constexpr static auto parse_id = [](context *ctx, str id, tok_iter it, const tok_iter end){
		const auto ref_expr = make_expr(ham::type_tag_v<HAM_PARSE_IMPL_X_EXPR_REF>, ctx);
		if(it == end){
			return ref_expr;
		}
	};

	switch(tok_beg->kind){
		case HAM_TOKEN_ID:{

		}

		default: return nullptr;
	}
}

HAM_C_API_END

#undef HAM_PARSE_IMPL_X_CHAR
#undef HAM_PARSE_IMPL_X_STR
#undef HAM_PARSE_IMPL_X_TOKEN

#undef HAM_PARSE_IMPL_X_EXPR_BASE
#undef HAM_PARSE_IMPL_X_EXPR_ERROR
#undef HAM_PARSE_IMPL_X_EXPR_BINDING
#undef HAM_PARSE_IMPL_X_EXPR_REF

#undef HAM_PARSE_IMPL_X_PARSE
#undef HAM_PARSE_IMPL_X_PARSE_CONTEXT_DESTROY
#undef HAM_PARSE_IMPL_X_PARSE_CONTEXT_CREATE

#undef HAM_PARSE_IMPL_X_UTF
#undef HAM_PARSE_IMPL_X_H

#endif // !HAM_PARSE_IMPL_X_H
