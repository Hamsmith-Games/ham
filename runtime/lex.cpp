/*
 * Ham Runtime
 * Copyright (C) 2022 Hamsmith Ltd.
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

#include "ham/lex.h"

HAM_C_API_BEGIN

ham_nothrow bool ham_lex_utf8 (ham_source_location_utf8  *loc, ham_str8  src, ham_token_utf8  *ret){ return ham::detail::constexpr_lex_utf<ham_char8> (loc, src, ret); }
ham_nothrow bool ham_lex_utf16(ham_source_location_utf16 *loc, ham_str16 src, ham_token_utf16 *ret){ return ham::detail::constexpr_lex_utf<ham_char16>(loc, src, ret); }
ham_nothrow bool ham_lex_utf32(ham_source_location_utf32 *loc, ham_str32 src, ham_token_utf32 *ret){ return ham::detail::constexpr_lex_utf<ham_char32>(loc, src, ret); }

HAM_C_API_END
