/**
 * The Ham Programming Language
 * Copyright (C) 2022  Hamsmith Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef HAM_META_HPP
#define HAM_META_HPP 1

/**
 * @defgroup HAM_META Metaprogramming utilities
 * @ingroup HAM
 * @{
 */

#include "typedefs.h"

namespace ham::meta{
	template<typename Char, usize N>
	struct cexpr_str{
		public:
			constexpr cexpr_str(const Char (&lit)[N]) noexcept
				: m_ptr(lit){}

			constexpr static usize size() noexcept{ return N-1; }
			constexpr const Char *data() const noexcept{ return m_ptr; }

			constexpr operator basic_str<Char>() const noexcept{ return basic_str(m_ptr, N-1); }

			const Char *const m_ptr;
	};

	template<typename Char, usize N>
	cexpr_str(const Char(&)[N]) -> cexpr_str<Char, N>;

	template<cexpr_str Str>
	using constant_cexpr_str = meta::constant_value<decltype(Str), Str>;
}

namespace ham{
	namespace str_literals{
		template<meta::cexpr_str Str>
		constexpr inline auto operator""_cexpr(){ return Str; }
	}
}

namespace ham::meta{
	//! @cond ignore
	namespace detail{
		constexpr inline str8 type_name_void() noexcept{ return "void"; }
		constexpr inline str8 type_name_bool() noexcept{ return "bool"; }

		constexpr inline str8 type_name_char8()  noexcept{ return "char8"; }
		constexpr inline str8 type_name_char16() noexcept{ return "char16"; }
		constexpr inline str8 type_name_char32() noexcept{ return "char32"; }

		constexpr inline str8 type_name_i8()   noexcept{ return "i8"; }
		constexpr inline str8 type_name_u8()   noexcept{ return "u8"; }
		constexpr inline str8 type_name_i16()  noexcept{ return "i16"; }
		constexpr inline str8 type_name_u16()  noexcept{ return "u16"; }
		constexpr inline str8 type_name_i32()  noexcept{ return "i32"; }
		constexpr inline str8 type_name_u32()  noexcept{ return "u32"; }
		constexpr inline str8 type_name_i64()  noexcept{ return "i64"; }
		constexpr inline str8 type_name_u64()  noexcept{ return "u64"; }
		constexpr inline str8 type_name_i128() noexcept{ return "i128"; }
		constexpr inline str8 type_name_u128() noexcept{ return "u128"; }

		constexpr inline str8 type_name_f16()  noexcept{ return "f16"; }
		constexpr inline str8 type_name_f32()  noexcept{ return "f32"; }
		constexpr inline str8 type_name_f64()  noexcept{ return "f64"; }
		constexpr inline str8 type_name_f128() noexcept{ return "f128"; }

		constexpr inline str8 type_name_str8()  noexcept{ return "str8"; }
		constexpr inline str8 type_name_str16() noexcept{ return "str16"; }
		constexpr inline str8 type_name_str32() noexcept{ return "str32"; }

		constexpr inline str8 type_name_uuid() noexcept{ return "uuid"; }
	}
	//! @endcond

	template<typename T>
	struct type_name{
		template<typename U = T>
		static constexpr str8 get_name() noexcept{
		#ifdef __GNUC__

			constexpr str8 full_fn_name = __PRETTY_FUNCTION__;

		#	ifdef __clang__
				constexpr str8 prefix_str = "[T = ";
		#	else
				constexpr str8 prefix_str = "[with U = ";
		#	endif

			constexpr usize prefix_pos = full_fn_name.find(prefix_str);
			static_assert(prefix_pos != str8::npos);

		#	ifdef __clang__
				constexpr usize sep_pos = full_fn_name.find(", U", prefix_pos + prefix_str.len());
		#	else
				constexpr usize sep_pos = full_fn_name.find(";", prefix_pos + prefix_str.len());
		#	endif

			static_assert(sep_pos != str8::npos);

			constexpr usize start_pos = prefix_pos + prefix_str.len();

			return full_fn_name.substr(start_pos, sep_pos - start_pos);

		#elif defined(_MSVC_VER)

			return __FUNCSIG__;

		#else

		#	error "Ham expects a GCC compatible compiler or MSVC"

		#endif
		}

		constexpr static str8 value = get_name();
	};

	//! @cond ignore
#define HAM_IMPL_TYPE_NAME(typename_) constexpr static str8 value = HAM_CONCAT(detail::type_name_, typename_)()
	//! @endcond

	template<typename T>
	constexpr inline auto type_name_v = type_name<T>::value;

	template<> struct type_name<void>{ HAM_IMPL_TYPE_NAME(void); };
	template<> struct type_name<bool>{ HAM_IMPL_TYPE_NAME(bool); };

	template<> struct type_name<char8> { HAM_IMPL_TYPE_NAME(char8); };
	template<> struct type_name<char16>{ HAM_IMPL_TYPE_NAME(char16); };
	template<> struct type_name<char32>{ HAM_IMPL_TYPE_NAME(char32); };

	template<> struct type_name<i8> { HAM_IMPL_TYPE_NAME(i8); };
	template<> struct type_name<u8> { HAM_IMPL_TYPE_NAME(u8); };
	template<> struct type_name<i16>{ HAM_IMPL_TYPE_NAME(i16); };
	template<> struct type_name<u16>{ HAM_IMPL_TYPE_NAME(u16); };
	template<> struct type_name<i32>{ HAM_IMPL_TYPE_NAME(i32); };
	template<> struct type_name<u32>{ HAM_IMPL_TYPE_NAME(u32); };
	template<> struct type_name<i64>{ HAM_IMPL_TYPE_NAME(i64); };
	template<> struct type_name<u64>{ HAM_IMPL_TYPE_NAME(u64); };

#ifdef HAM_INT128
	template<> struct type_name<i128>{ HAM_IMPL_TYPE_NAME(i128); };
	template<> struct type_name<u128>{ HAM_IMPL_TYPE_NAME(u128); };
#endif

#ifdef HAM_FLOAT16
	template<> struct type_name<f16>{ HAM_IMPL_TYPE_NAME(f16); };
#endif

	template<> struct type_name<f32>{ HAM_IMPL_TYPE_NAME(f32); };
	template<> struct type_name<f64>{ HAM_IMPL_TYPE_NAME(f64); };

#ifdef HAM_FLOAT128
	template<> struct type_name<f128>{ HAM_IMPL_TYPE_NAME(f128); };
#endif

	template<> struct type_name<ham_str8> { HAM_IMPL_TYPE_NAME(str8); };
	template<> struct type_name<ham_str16>{ HAM_IMPL_TYPE_NAME(str16); };
	template<> struct type_name<ham_str32>{ HAM_IMPL_TYPE_NAME(str32); };

	template<> struct type_name<ham_uuid>{ HAM_IMPL_TYPE_NAME(uuid); };
}

/**
 * @}
 */

#endif // !HAM_META_HPP
