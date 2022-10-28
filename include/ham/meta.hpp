/*
 * Ham Runtime
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
 * GNU Lesser General Public License for more details.
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

#include "math.h"

namespace ham::meta{
	template<usize N>
	struct cexpr_str{
		public:
			consteval cexpr_str(const char (&lit)[N+1]) noexcept
				: cexpr_str(lit, make_index_seq<N>()){}

			consteval cexpr_str(const cexpr_str &other) noexcept
				: cexpr_str(other.m_chars, make_index_seq<N>()){}

			constexpr static usize size() noexcept{ return N; }
			constexpr const char *data() const noexcept{ return m_chars; }

			constexpr operator str8() const noexcept{ return basic_str(m_chars, N); }

			constexpr bool operator==(const str8 &other) const noexcept{
				if(other.size() != N) return false;

				for(usize i = 0; i < size(); i++){
					if(m_chars[i] != other[i]) return false;
				}

				return true;
			}

			constexpr bool operator!=(const str8 &other) const noexcept{
				if(other.size() != N) return true;

				for(usize i = 0; i < size(); i++){
					if(m_chars[i] != other[i]) return true;
				}

				return false;
			}

			template<usize M>
			constexpr auto operator+(const cexpr_str<M> &other) const noexcept{
				return concat_impl(other, make_index_seq<N>(), make_index_seq<M>());
			}

			char m_chars[N+1];

		private:
			template<usize ... Is>
			consteval cexpr_str(const char *lit, index_seq<Is...>)
				: m_chars{ lit[Is]..., '\0' }{}

//			template<typename ... Chars>
//			consteval cexpr_str(Chars ... chars) noexcept
//				: m_chars{ chars..., '\0' }
//			{
//				static_assert(sizeof...(Chars) == N);
//			}

			template<usize M, usize ... Is, usize ... Js>
			constexpr auto concat_impl(const cexpr_str<M> &other, index_seq<Is...>, index_seq<Js...>){
				return cexpr_str<N+M>(m_chars[Is]..., other.m_chars[Js]...);
			}
	};

	template<usize N>
	cexpr_str(const char(&)[N]) -> cexpr_str<N-1>;

	template<cexpr_str Str>
	struct constant_str{
		constexpr static auto value = Str;
	};

	template<cexpr_str Str>
	constexpr inline auto constant_str_v = constant_str<Str>::value;
}

namespace ham{
	namespace str_literals{
		template<meta::cexpr_str Str>
		constexpr inline auto operator "" _c() noexcept{ return Str; }
	}
}

namespace ham::meta{
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

	template<> struct type_name<ham_vec2>: constant_str<"vec2">{};
	template<> struct type_name<ham_vec3>: constant_str<"vec3">{};
	template<> struct type_name<ham_vec4>: constant_str<"vec4">{};

	template<> struct type_name<void>: constant_str<"void">{};
	template<> struct type_name<bool>: constant_str<"bool">{};

	template<> struct type_name<char8>:  constant_str<"char8">{};
	template<> struct type_name<char16>: constant_str<"char16">{};
	template<> struct type_name<char32>: constant_str<"char32">{};

	template<> struct type_name<i8>:  constant_str<"i8">{};
	template<> struct type_name<u8>:  constant_str<"u8">{};
	template<> struct type_name<i16>: constant_str<"i16">{};
	template<> struct type_name<u16>: constant_str<"u16">{};
	template<> struct type_name<i32>: constant_str<"i32">{};
	template<> struct type_name<u32>: constant_str<"u32">{};
	template<> struct type_name<i64>: constant_str<"i64">{};
	template<> struct type_name<u64>: constant_str<"u64">{};

#ifdef HAM_INT128
	template<> struct type_name<i128>: constant_str<"i128">{};
	template<> struct type_name<u128>: constant_str<"u128">{};
#endif

#ifdef HAM_FLOAT16
	template<> struct type_name<f16>: constant_str<"f16">{};
#endif

	template<> struct type_name<f32>: constant_str<"f32">{};
	template<> struct type_name<f64>: constant_str<"f64">{};

#ifdef HAM_FLOAT128
	template<> struct type_name<f128>: constant_str<"f128">{};
#endif
}

/**
 * @}
 */

#endif // !HAM_META_HPP
