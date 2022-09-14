#ifndef LEX_IMPL_X_H
#define LEX_IMPL_X_H 1

#ifndef HAM_LEX_IMPL_X_H_UTF
#	error "HAM_LEX_IMPL_X_H_UTF not defined before include lex-impl.x.h"
#endif

#ifdef HAM_LEX_IMPL_X_H_CHAR
#	undef HAM_LEX_IMPL_X_H_CHAR
#endif

#ifdef HAM_LEX_IMPL_X_H_STR
#	undef HAM_LEX_IMPL_X_H_STR
#endif

#ifdef HAM_LEX_IMPL_X_H_SOURCE_LOCATION
#	undef HAM_LEX_IMPL_X_H_SOURCE_LOCATION
#endif

#ifdef HAM_LEX_IMPL_X_H_TOKEN
#	undef HAM_LEX_IMPL_X_H_TOKEN
#endif

#ifdef HAM_LEX_IMPL_X_H_LIT
#	undef HAM_LEX_IMPL_X_H_LIT
#endif

#ifdef HAM_LEX_IMPL_X_H_U_NEXT
#	undef HAM_LEX_IMPL_X_H_U_NEXT
#endif

#ifdef HAM_LEX_IMPL_X_H_ERROR_BUF
#	undef HAM_LEX_IMPL_X_H_ERROR_BUF
#	undef HAM_LEX_IMPL_X_H_ERROR_BUF_
#	undef HAM_LEX_IMPL_X_H_ERROR_BUF__
#endif

#if HAM_LEX_IMPL_X_H_UTF == 16
#	include "unicode/utf16.h"
#	define HAM_LEX_IMPL_X_H_U_NEXT(str, off, len, ret_cp) U16_NEXT(str, off, len, ret_cp)
#else
#	include "unicode/utf8.h"
#	define HAM_LEX_IMPL_X_H_U_NEXT(str, off, len, ret_cp) U8_NEXT(str, off, len, ret_cp)
#endif

#define HAM_LEX_IMPL_X_H_CHAR HAM_CHAR_UTF(HAM_LEX_IMPL_X_H_UTF)
#define HAM_LEX_IMPL_X_H_STR HAM_STR_UTF(HAM_LEX_IMPL_X_H_UTF)
#define HAM_LEX_IMPL_X_H_SOURCE_LOCATION HAM_SOURCE_LOCATION_UTF(HAM_LEX_IMPL_X_H_UTF)
#define HAM_LEX_IMPL_X_H_TOKEN HAM_TOKEN_UTF(HAM_LEX_IMPL_X_H_UTF)

#define HAM_LEX_IMPL_X_H_LIT(lit) HAM_LIT_UTF(HAM_LEX_IMPL_X_H_UTF, lit)

#define HAM_LEX_IMPL_X_H_ERROR_BUF__(n) ham_lex_error_buf_utf##n
#define HAM_LEX_IMPL_X_H_ERROR_BUF_(n) HAM_LEX_IMPL_X_H_ERROR_BUF__(n)
#define HAM_LEX_IMPL_X_H_ERROR_BUF HAM_LEX_IMPL_X_H_ERROR_BUF_(HAM_LEX_IMPL_X_H_UTF)

static thread_local HAM_LEX_IMPL_X_H_CHAR HAM_LEX_IMPL_X_H_ERROR_BUF[HAM_LEX_ERROR_MAX_LENGTH];

#define HAM_LEX_IMPL_X_H_LEX HAM_LEX_UTF(HAM_LEX_IMPL_X_H_UTF)

HAM_C_API_BEGIN

bool HAM_LEX_IMPL_X_H_LEX(HAM_LEX_IMPL_X_H_SOURCE_LOCATION *loc, HAM_LEX_IMPL_X_H_STR src, HAM_LEX_IMPL_X_H_TOKEN *ret){
	using namespace ham::typedefs;

	using source_location = HAM_LEX_IMPL_X_H_SOURCE_LOCATION;
	using str = HAM_LEX_IMPL_X_H_STR;
	using src_iter = decltype(src.ptr);

	const src_iter src_beg = src.ptr;
	const src_iter src_end = src.ptr + src.len;

	if(src_beg == src_end){
		ret->kind = HAM_TOKEN_EOF;
		ret->loc = *loc;
		ret->str = (str){ nullptr, 0 };
		return true;
	}

	constexpr auto lex_space = [](source_location *loc, utf_cp head_cp, const src_iter tail_beg, const src_iter tail_end) -> src_iter{
		if(head_cp == U'\n' || !u_isUWhiteSpace(head_cp)){
			return nullptr;
		}
		else if(tail_beg == tail_end){
			return tail_beg;
		}

		utf_cp cp = 0;
		i32 off = 0;

	#if HAM_LEX_IMPL_X_H_UTF == 32
		cp = *tail_beg;
		++off;
	#else
		const ham_usize tail_len = ((uptr)tail_end/sizeof(*tail_end)) - ((uptr)tail_beg/sizeof(*tail_beg));
		HAM_LEX_IMPL_X_H_U_NEXT(tail_beg, off, tail_len, cp);
	#endif

		src_iter ret = tail_beg;

		while((cp != U'\n') && u_isUWhiteSpace(cp)){
			ret = tail_beg + off;
			if(ret == tail_end) break;

		#if HAM_LEX_IMPL_X_H_UTF == 32
			cp = tail_beg[off];
			++off;
		#else
			HAM_LEX_IMPL_X_H_U_NEXT(tail_beg, off, tail_len, cp);
		#endif

			++loc->col;
		}

		return ret;
	};

	constexpr auto lex_op = [](source_location *loc, utf_cp head_cp, const src_iter tail_beg, const src_iter tail_end) -> src_iter{
		if(!ham_u_isop(head_cp)){
			return nullptr;
		}
		else if(tail_beg == tail_end){
			return tail_beg;
		}

		utf_cp cp = 0;
		i32 off = 0;

	#if HAM_LEX_IMPL_X_H_UTF == 32
		cp = *tail_beg;
		++off;
	#else
		const ham_usize tail_len = ((uptr)tail_end/sizeof(*tail_end)) - ((uptr)tail_beg/sizeof(*tail_beg));
		HAM_LEX_IMPL_X_H_U_NEXT(tail_beg, off, tail_len, cp);
	#endif

		src_iter ret = tail_beg;

		while(ham_u_isop(cp)){
			ret = tail_beg + off;
			if(ret == tail_end) break;

		#if HAM_LEX_IMPL_X_H_UTF == 32
			cp = tail_beg[off];
			++off;
		#else
			HAM_LEX_IMPL_X_H_U_NEXT(tail_beg, off, tail_len, cp);
		#endif

			++loc->col;
		}

		if(ret != tail_end && cp == U'\n'){
			++loc->line;
			loc->col = 0;
		}

		return ret;
	};

	constexpr auto lex_id = [](source_location *loc, utf_cp head_cp, const src_iter tail_beg, const src_iter tail_end) -> src_iter{
		if(head_cp != U'_' && !u_isUAlphabetic(head_cp)){
			return nullptr;
		}
		else if(tail_beg == tail_end){
			return tail_beg;
		}

		utf_cp cp = 0;
		i32 off = 0;

	#if HAM_LEX_IMPL_X_H_UTF == 32
		cp = *tail_beg;
		++off;
	#else
		const ham_usize tail_len = ((uptr)tail_end/sizeof(*tail_end)) - ((uptr)tail_beg/sizeof(*tail_beg));
		HAM_LEX_IMPL_X_H_U_NEXT(tail_beg, off, tail_len, cp);
	#endif

		src_iter ret = tail_beg;

		while((cp == U'_') || u_isUAlphabetic(cp) || u_isdigit(cp)){
			ret = tail_beg + off;
			if(ret == tail_end) break;

		#if HAM_LEX_IMPL_X_H_UTF == 32
			cp = tail_beg[off];
			++off;
		#else
			HAM_LEX_IMPL_X_H_U_NEXT(tail_beg, off, tail_len, cp);
		#endif

			++loc->col;
		}

		if(ret != tail_end && cp == U'\n'){
			++loc->line;
			loc->col = 0;
		}

		return ret;
	};

	const source_location start_loc = *loc;

	i32 tail_off = 0;
	utf_cp cp = 0;

#if HAM_LEX_IMPL_X_H_UTF == 32
	cp = *src_beg;
	++tail_off;
#else
	HAM_LEX_IMPL_X_H_U_NEXT(src_beg, tail_off, src.len, cp);
#endif

	++loc->col;

	const src_iter tail_beg = src_beg + tail_off;

	src_iter tok_end = nullptr;

	ret->kind = HAM_TOKEN_KIND_COUNT;
	ret->loc = start_loc;

	if(cp == U'\n'){
		ret->kind = HAM_TOKEN_NEWLINE;
		++loc->line;
		loc->col = 0;
		tok_end = tail_beg;
	}
	else if(ham_u_isbracket(cp)){
		ret->kind = HAM_TOKEN_BRACKET;
		tok_end = tail_beg;
	}
	else if((tok_end = lex_op(loc, cp, tail_beg, src_end))){
		ret->kind = HAM_TOKEN_OP;
	}
	else if((tok_end = lex_space(loc, cp, tail_beg, src_end))){
		ret->kind = HAM_TOKEN_SPACE;
	}
	else if((tok_end = lex_id(loc, cp, tail_beg, src_end))){
		ret->kind = HAM_TOKEN_ID;
	}

	ret->loc = start_loc;

	if(ret->kind != HAM_TOKEN_KIND_COUNT){
		ret->str = (str){ src_beg, ((uptr)tok_end/sizeof(*tok_end)) - ((uptr)src_beg/(sizeof(*src_beg))) };
		return true;
	}
	else{
		ret->kind = HAM_TOKEN_ERROR;
		ret->str = HAM_LEX_IMPL_X_H_LIT("unrecognized unicode codepoint");
		return false;
	}
}

HAM_C_API_END

#undef HAM_LEX_IMPL_X_H_UTF
#undef LEX_IMPL_X_H

#endif // !LEX_IMPL_X_H
