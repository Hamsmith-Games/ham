#ifndef HAM_META_HPP
#define HAM_META_HPP 1

/**
 * @defgroup HAM_META Metaprogramming utilities
 * @ingroup HAM
 * @{
 */

#include "typedefs.h"

namespace ham{
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

			constexpr str8 with_str = "with U = ";

			constexpr usize with_pos = full_fn_name.find(with_str);
			static_assert(with_pos != str8::npos);

			constexpr usize semi_pos = full_fn_name.find(";", with_pos + with_str.len());
			static_assert(semi_pos != str8::npos);

			constexpr usize start_pos = with_pos + with_str.len();

			return full_fn_name.substr(start_pos, semi_pos - start_pos);

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
