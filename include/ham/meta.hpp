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

namespace ham{
	template<typename T, T Value>
	struct constant_value{
		using type = T;
		constexpr static T value = Value;
	};

	// pretty much useless
	template<unit Value> using constant_unit = constant_value<unit, Value>;

	template<bool Value> using constant_bool = constant_value<bool, Value>;

	template<i8  Value> using constant_i8  = constant_value<i8,  Value>;
	template<u8  Value> using constant_u8  = constant_value<u8,  Value>;
	template<i16 Value> using constant_i16 = constant_value<i16, Value>;
	template<u16 Value> using constant_u16 = constant_value<u16, Value>;
	template<i32 Value> using constant_i32 = constant_value<i32, Value>;
	template<u32 Value> using constant_u32 = constant_value<u32, Value>;
	template<i64 Value> using constant_i64 = constant_value<i64, Value>;
	template<u64 Value> using constant_u64 = constant_value<u64, Value>;

#ifdef HAM_INT128
	template<i128 Value> using constant_i128 = constant_value<i128, Value>;
	template<u128 Value> using constant_u128 = constant_value<u128, Value>;
#endif

	template<iptr Value> using constant_iptr = constant_value<iptr, Value>;
	template<uptr Value> using constant_uptr = constant_value<uptr, Value>;

	template<isize Value> using constant_isize = constant_value<isize, Value>;
	template<usize Value> using constant_usize = constant_value<usize, Value>;

#ifdef HAM_FLOAT16
	template<f16 Value> using constant_f16 = constant_value<f16, Value>;
#endif

	template<f32 Value> using constant_f32 = constant_value<f32, Value>;
	template<f64 Value> using constant_f64 = constant_value<f64, Value>;

#ifdef HAM_FLOAT128
	template<f128 Value> using constant_f128 = constant_value<f128, Value>;
#endif

	template<uuid  Value> using constant_uuid  = constant_value<uuid,  Value>;
	template<str8  Value> using constant_str8  = constant_value<str8,  Value>;
	template<str16 Value> using constant_str16 = constant_value<str16, Value>;
	template<str32 Value> using constant_str32 = constant_value<str32, Value>;

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

	template<typename T>
	constexpr inline auto type_name_v = type_name<T>::value;

	template<> struct type_name<void>: detail::constant_call<detail::type_name_void>{};
	template<> struct type_name<bool>: detail::constant_call<detail::type_name_bool>{};

	template<> struct type_name<char8>:  detail::constant_call<detail::type_name_char8>{};
	template<> struct type_name<char16>: detail::constant_call<detail::type_name_char16>{};
	template<> struct type_name<char32>: detail::constant_call<detail::type_name_char32>{};

	template<> struct type_name<i8>:  detail::constant_call<detail::type_name_i8>{};
	template<> struct type_name<u8>:  detail::constant_call<detail::type_name_u8>{};
	template<> struct type_name<i16>: detail::constant_call<detail::type_name_i16>{};
	template<> struct type_name<u16>: detail::constant_call<detail::type_name_u16>{};
	template<> struct type_name<i32>: detail::constant_call<detail::type_name_i32>{};
	template<> struct type_name<u32>: detail::constant_call<detail::type_name_u32>{};
	template<> struct type_name<i64>: detail::constant_call<detail::type_name_i64>{};
	template<> struct type_name<u64>: detail::constant_call<detail::type_name_u64>{};

#ifdef HAM_INT128
	template<> struct type_name<i128>: detail::constant_call<detail::type_name_i128>{};
	template<> struct type_name<u128>: detail::constant_call<detail::type_name_u128>{};
#endif

#ifdef HAM_FLOAT16
	template<> struct type_name<f16>: detail::constant_call<detail::type_name_f16>{};
#endif

	template<> struct type_name<f32>: detail::constant_call<detail::type_name_f32>{};
	template<> struct type_name<f64>: detail::constant_call<detail::type_name_f64>{};

#ifdef HAM_FLOAT128
	template<> struct type_name<f128>: detail::constant_call<detail::type_name_f128>{};
#endif

	template<> struct type_name<ham_str8>:  detail::constant_call<detail::type_name_str8>{};
	template<> struct type_name<ham_str16>: detail::constant_call<detail::type_name_str16>{};
	template<> struct type_name<ham_str32>: detail::constant_call<detail::type_name_str32>{};

	template<> struct type_name<ham_uuid>: detail::constant_call<detail::type_name_uuid>{};
}

/**
 * @}
 */

#endif // !HAM_META_HPP
