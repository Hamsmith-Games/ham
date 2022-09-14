#ifndef LEX_IMPL_X_H
#define LEX_IMPL_X_H 1

#ifndef HAM_LEX_IMPL_X_H_UTF
#	error "HAM_LEX_IMPL_X_H_UTF not defined before include lex-impl.x.h"
#endif

#ifdef HAM_LEX_IMPL_X_H_U_NEXT
#	undef HAM_LEX_IMPL_X_H_U_NEXT
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
#define HAM_LEX_IMPL_X_H_LIT_C(lit) HAM_LIT_C_UTF(HAM_LEX_IMPL_X_H_UTF, lit)

#define HAM_LEX_IMPL_X_H_ERROR_BUF__(n) ham_lex_error_buf_utf##n
#define HAM_LEX_IMPL_X_H_ERROR_BUF_(n) HAM_LEX_IMPL_X_H_ERROR_BUF__(n)
#define HAM_LEX_IMPL_X_H_ERROR_BUF HAM_LEX_IMPL_X_H_ERROR_BUF_(HAM_LEX_IMPL_X_H_UTF)

#define HAM_LEX_IMPL_X_H_ESCAPE_CHAR HAM_CONCAT(ham_escape_char_utf, HAM_LEX_IMPL_X_H_UTF)
#define HAM_LEX_IMPL_X_H_LEX HAM_LEX_UTF(HAM_LEX_IMPL_X_H_UTF)

static thread_local HAM_LEX_IMPL_X_H_CHAR HAM_LEX_IMPL_X_H_ERROR_BUF[HAM_LEX_ERROR_MAX_LENGTH];

HAM_C_API_BEGIN

constexpr static inline HAM_LEX_IMPL_X_H_CHAR HAM_LEX_IMPL_X_H_ESCAPE_CHAR(UChar32 cp){
	switch(cp){
	#define HAM_CASE(val) case (U##val): return HAM_LIT_C_UTF(HAM_LEX_IMPL_X_H_UTF, val);

		HAM_CASE('a')
		HAM_CASE('b')
		HAM_CASE('f')
		HAM_CASE('n')
		HAM_CASE('r')
		HAM_CASE('t')
		HAM_CASE('v')
		HAM_CASE('\\')
		HAM_CASE('\'')
		HAM_CASE('\"')
		HAM_CASE('\?')

	#undef HAM_CASE

		default: return cp;
	}
}

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

	constexpr static auto lex_inline_generic = [](
		source_location *loc, UChar32 head_cp, const src_iter tail_beg, const src_iter tail_end,
		const auto head_cond, const auto loop_cond
	)
		-> src_iter
	{
		if(!head_cond(head_cp)){
			return nullptr;
		}
		else if(tail_beg == tail_end){
			return tail_beg;
		}

		UChar32 cp = 0;
		i32 off = 0;

	#if HAM_LEX_IMPL_X_H_UTF == 32
		cp = *tail_beg;
		++off;
	#else
		const ham_usize tail_len = ((uptr)tail_end/sizeof(*tail_end)) - ((uptr)tail_beg/sizeof(*tail_beg));
		HAM_LEX_IMPL_X_H_U_NEXT(tail_beg, off, tail_len, cp);
	#endif

		src_iter ret = tail_beg;

		while(loop_cond(cp)){
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

	constexpr auto lex_space = [](source_location *loc, UChar32 head_cp, const src_iter tail_beg, const src_iter tail_end) -> src_iter{
		constexpr auto cond = [](UChar32 cp){ return (cp != U'\n') && u_isUWhiteSpace(cp); };
		return lex_inline_generic(
			loc, head_cp, tail_beg, tail_end,
			cond, cond
		);
	};

	constexpr auto lex_op = [](source_location *loc, UChar32 head_cp, const src_iter tail_beg, const src_iter tail_end) -> src_iter{
		constexpr auto cond = [](UChar32 cp){ return (cp != U'\n') && u_isUWhiteSpace(cp); };
		return lex_inline_generic(
			loc, head_cp, tail_beg, tail_end,
			ham_u_isop, ham_u_isop
		);
	};

	constexpr auto lex_id = [](source_location *loc, UChar32 head_cp, const src_iter tail_beg, const src_iter tail_end) -> src_iter{
		constexpr auto cond = [](UChar32 cp){ return (cp != U'\n') && u_isUWhiteSpace(cp); };
		return lex_inline_generic(
			loc, head_cp, tail_beg, tail_end,
			[](UChar32 cp){ return (cp == U'_') || u_isUAlphabetic(cp); },
			[](UChar32 cp){ return (cp == U'_') || u_isdigit(cp) || u_isUAlphabetic(cp); }
		);
	};

	constexpr auto lex_str = [](source_location *loc, UChar32 head_cp, const src_iter tail_beg, const src_iter tail_end) -> src_iter{
		if(head_cp != '"' && head_cp != '\''){
			return nullptr;
		}
		else if(tail_beg == tail_end){
			return tail_beg;
		}

		UChar32 cp = 0;
		i32 off = 0;

	#if HAM_LEX_IMPL_X_H_UTF == 32
		cp = *tail_beg;
		++off;
	#else
		const ham_usize tail_len = ((uptr)tail_end/sizeof(*tail_end)) - ((uptr)tail_beg/sizeof(*tail_beg));
		HAM_LEX_IMPL_X_H_U_NEXT(tail_beg, off, tail_len, cp);
	#endif

		src_iter ret = tail_beg;

		while(cp != head_cp){
			ret = tail_beg + off;

			switch(cp){
				case '\\':{
					// esacpe sequence
					if(ret == tail_end){
						return ret;
					}

				#if HAM_LEX_IMPL_X_H_UTF == 32
					cp = tail_beg[off];
					++off;
				#else
					HAM_LEX_IMPL_X_H_U_NEXT(tail_beg, off, tail_len, cp);
				#endif

					if(cp == '\n'){
						// string line continuation
						++loc->line;
						loc->col = 0;
					}
					else{
						++loc->col;
					}
					//const UChar32 escaped = HAM_LEX_IMPL_X_H_ESCAPE_CHAR(cp);

					break;
				}

				default: break;
			}

			if(ret == tail_end) return tail_end;

		#if HAM_LEX_IMPL_X_H_UTF == 32
			cp = tail_beg[off];
			++off;
		#else
			HAM_LEX_IMPL_X_H_U_NEXT(tail_beg, off, tail_len, cp);
		#endif

			++loc->col;
		}

		// skip ending quote character
		++ret;
		++loc->col;

		return ret;
	};

	const source_location start_loc = *loc;

	i32 tail_off = 0;
	UChar32 cp = 0;

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
	else if((tok_end = lex_str(loc, cp, tail_beg, src_end))){
		ret->kind = HAM_TOKEN_STR;
	}
	else if(u_isdigit(cp)){
		// special number handling
		ret->kind = HAM_TOKEN_NAT;

		const usize tail_len = ((usize)src_end - (usize)tail_beg)/sizeof(*tail_beg);

		i32 off = 0;

		while(off < tail_len){
			tok_end = tail_beg + off;

		#if HAM_LEX_IMPL_X_H_UTF == 32
			cp = tail_beg[off];
			++off;
		#else
			HAM_LEX_IMPL_X_H_U_NEXT(tail_beg, off, tail_len, cp);
		#endif

			if(!u_isdigit(cp)){
				break;
			}

			++loc->col;
		}

		if(off < tail_len && cp == '.'){
			// real number
			ret->kind = HAM_TOKEN_REAL;

			// skip decimal point
			++off;
			++loc->col;

			while(off < tail_len){
				tok_end = tail_beg + off;

			#if HAM_LEX_IMPL_X_H_UTF == 32
				cp = tail_beg[off];
				++off;
			#else
				HAM_LEX_IMPL_X_H_U_NEXT(tail_beg, off, tail_len, cp);
			#endif

				if(!u_isdigit(cp)){
					break;
				}

				++loc->col;
			}
		}
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

#undef HAM_LEX_IMPL_X_H_CHAR
#undef HAM_LEX_IMPL_X_H_STR
#undef HAM_LEX_IMPL_X_H_SOURCE_LOCATION
#undef HAM_LEX_IMPL_X_H_TOKEN
#undef HAM_LEX_IMPL_X_H_LIT
#undef HAM_LEX_IMPL_X_H_LIT_C
#undef HAM_LEX_IMPL_X_H_ERROR_BUF__
#undef HAM_LEX_IMPL_X_H_ERROR_BUF_
#undef HAM_LEX_IMPL_X_H_ERROR_BUF
#undef HAM_LEX_IMPL_X_H_ESCAPE_CHAR
#undef HAM_LEX_IMPL_X_H_LEX
#undef HAM_LEX_IMPL_X_H_UTF
#undef LEX_IMPL_X_H

#endif // !LEX_IMPL_X_H
