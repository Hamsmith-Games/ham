#include "ham/lex.h"

#include <string.h>

#define U_CHARSET_IS_UTF8 1
#define U_NO_DEFAULT_INCLUDE_UTF_HEADERS 1
#include "unicode/uchar.h"

constexpr static inline bool ham_u_isbracket(UChar32 cp) noexcept{
	switch(cp){
		case U'{':
		case U'}':
		case U'[':
		case U']':
		case U'(':
		case U')':
			return true;

		default:
			return false;
	}
}

constexpr static inline bool ham_u_isop(UChar32 cp) noexcept{
	switch(cp){
		case '+':
		case '-':
		case '*':
		case '/':
		case '=':
		case '<':
		case '>':
		case '%':
		case '^':
		case '~':
			return true;

		default:
			return false;
	}
}

#define HAM_LEX_IMPL_X_H_UTF 8
#include "lex-impl.x.h"

#define HAM_LEX_IMPL_X_H_UTF 16
#include "lex-impl.x.h"

#define HAM_LEX_IMPL_X_H_UTF 32
#include "lex-impl.x.h"
