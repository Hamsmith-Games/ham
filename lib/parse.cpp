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
		static inline const expr_base_ctype_t<Char> *parse_root(
			parse_scope_ctype_t<Char> *scope,
			const ctoken_t<Char> *head,
			ctoken_range_t<Char> tail
		);

		template<typename Char>
		static inline const expr_base_ctype_t<Char> *parse_id(
			parse_scope_ctype_t<Char> *scope,
			const ctoken_t<Char> *head,
			ctoken_range_t<Char> tail
		);

		template<typename Char>
		static inline const expr_base_ctype_t<Char> *parse_nat(
			parse_scope_ctype_t<Char> *scope,
			const ctoken_t<Char> *head,
			ctoken_range_t<Char> tail
		);


	}
}

#define HAM_PARSE_IMPL_X_UTF 8
#include "parse-impl.x.h"

#define HAM_PARSE_IMPL_X_UTF 16
#include "parse-impl.x.h"

#define HAM_PARSE_IMPL_X_UTF 32
#include "parse-impl.x.h"
