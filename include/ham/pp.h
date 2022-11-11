/*
 * Ham Runtime
 * Copyright (C) 2022 Keith Hammond
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

#ifndef HAM_PP_H
#define HAM_PP_H 1

/**
 * @defgroup HAM_PP Preprocessor helpers
 * @ingroup HAM
 * @{
 */

#if !defined(__GNUC__) && !defined(_MSVC_VER)
#	error "Unsupported compiler"
#endif

//! @cond ignore
#define HAM_IMPL_EXPAND0(...) __VA_ARGS__
#define HAM_IMPL_EXPAND1(...) HAM_IMPL_EXPAND0(__VA_ARGS__)
#define HAM_IMPL_EXPAND2(...) HAM_IMPL_EXPAND1(__VA_ARGS__)
#define HAM_IMPL_EXPAND3(...) HAM_IMPL_EXPAND2(__VA_ARGS__)
//! @endcond

/**
 * @brief Perform explicit macro expansion.
 */
#define HAM_EXPAND(...) HAM_IMPL_EXPAND3(__VA_ARGS__)

//! @cond ignore
#define HAM_IMPL_CONCAT_(a, b) a##b
#define HAM_IMPL_CONCAT(a, b) HAM_IMPL_CONCAT_(a, b)
//! @endcond

/**
 * @brief Concatenate two preprocessor tokens.
 */
#define HAM_CONCAT(a, b) HAM_IMPL_CONCAT(a, b)

/**
 * @brief Eat a set of parentheses.
 */
#define HAM_EAT(...) __VA_ARGS__

//! @cond ignore
#define HAM_IMPL_STRINGIFY_(x) #x
#define HAM_IMPL_STRINGIFY(x) HAM_IMPL_STRINGIFY_(x)
//! @endcond

#define HAM_STRINGIFY(x) HAM_IMPL_STRINGIFY(x)

//! @cond ignore
#define HAM_IMPL_ARG_N( \
		  _1, _2, _3, _4, _5, _6, _7, _8, _9,_10, \
		 _11,_12,_13,_14,_15,_16,_17,_18,_19,_20, \
		 _21,_22,_23,_24,_25,_26,_27,_28,_29,_30, \
		 _31,_32,_33,_34,_35,_36,_37,_38,_39,_40, \
		 _41,_42,_43,_44,_45,_46,_47,_48,_49,_50, \
		 _51,_52,_53,_54,_55,_56,_57,_58,_59,_60, \
		 _61,_62,_63,N,...) N

#define HAM_IMPL_RSEQ_N \
		 63,62,61,60,                   \
		 59,58,57,56,55,54,53,52,51,50, \
		 49,48,47,46,45,44,43,42,41,40, \
		 39,38,37,36,35,34,33,32,31,30, \
		 29,28,27,26,25,24,23,22,21,20, \
		 19,18,17,16,15,14,13,12,11,10, \
		 9,8,7,6,5,4,3,2,1,0

#define HAM_IMPL_NARGS(...) HAM_IMPL_ARG_N(__VA_ARGS__)
//! @endcond

/**
 * @brief Get the number of arguments passed in.
 */
#define HAM_NARGS(...) HAM_IMPL_NARGS(__VA_ARGS__, HAM_IMPL_RSEQ_N)

//! @cond ignore
#define HAM_IMPL_APPLY(f, ...) f(__VA_ARGS__)
//! @endcond

#define HAM_APPLY(f, ...) HAM_IMPL_APPLY(f __VA_OPT__(,) __VA_ARGS__)

//! @cond ignore
#define HAM_IMPL_REPEAT_0(...)
#define HAM_IMPL_REPEAT_1(expr) expr
#define HAM_IMPL_REPEAT_2(expr) expr expr
#define HAM_IMPL_REPEAT_3(expr) expr expr expr
#define HAM_IMPL_REPEAT_4(expr) expr expr expr expr
#define HAM_IMPL_REPEAT_5(expr) expr expr expr expr expr
#define HAM_IMPL_REPEAT_6(expr) expr expr expr expr expr expr
#define HAM_IMPL_REPEAT_7(expr) expr expr expr expr expr expr expr
#define HAM_IMPL_REPEAT_8(expr) expr expr expr expr expr expr expr expr
#define HAM_IMPL_REPEAT_9(expr) expr expr expr expr expr expr expr expr expr
#define HAM_IMPL_REPEAT_10(expr) expr expr expr expr expr expr expr expr expr expr
#define HAM_IMPL_REPEAT_11(expr) HAM_IMPL_REPEAT_10(expr) HAM_IMPL_REPEAT_1(expr)
#define HAM_IMPL_REPEAT_12(expr) HAM_IMPL_REPEAT_10(expr) HAM_IMPL_REPEAT_2(expr)
#define HAM_IMPL_REPEAT_13(expr) HAM_IMPL_REPEAT_10(expr) HAM_IMPL_REPEAT_3(expr)
#define HAM_IMPL_REPEAT_14(expr) HAM_IMPL_REPEAT_10(expr) HAM_IMPL_REPEAT_4(expr)
#define HAM_IMPL_REPEAT_15(expr) HAM_IMPL_REPEAT_10(expr) HAM_IMPL_REPEAT_5(expr)
#define HAM_IMPL_REPEAT_16(expr) HAM_IMPL_REPEAT_10(expr) HAM_IMPL_REPEAT_6(expr)

#define HAM_IMPL_REPEAT_N(n, expr) HAM_APPLY(HAM_CONCAT(HAM_IMPL_REPEAT_, n), expr)
//! @endcond

/**
 * @brief Repeat \p expr \p n times.
 */
#define HAM_REPEAT(n, expr) HAM_IMPL_REPEAT_N(n, expr)

//! @cond ignore
#define HAM_IMPL_MAP_0(...)
#define HAM_IMPL_MAP_1(f, x) HAM_APPLY(f, x)
#define HAM_IMPL_MAP_2(f, x, ...) HAM_APPLY(f, x) HAM_IMPL_MAP_1(f __VA_OPT__(,) __VA_ARGS__)
#define HAM_IMPL_MAP_3(f, x, ...) HAM_APPLY(f, x) HAM_IMPL_MAP_2(f __VA_OPT__(,) __VA_ARGS__)
#define HAM_IMPL_MAP_4(f, x, ...) HAM_APPLY(f, x) HAM_IMPL_MAP_3(f __VA_OPT__(,) __VA_ARGS__)
#define HAM_IMPL_MAP_5(f, x, ...) HAM_APPLY(f, x) HAM_IMPL_MAP_4(f __VA_OPT__(,) __VA_ARGS__)
#define HAM_IMPL_MAP_6(f, x, ...) HAM_APPLY(f, x) HAM_IMPL_MAP_5(f __VA_OPT__(,) __VA_ARGS__)
#define HAM_IMPL_MAP_7(f, x, ...) HAM_APPLY(f, x) HAM_IMPL_MAP_6(f __VA_OPT__(,) __VA_ARGS__)
#define HAM_IMPL_MAP_8(f, x, ...) HAM_APPLY(f, x) HAM_IMPL_MAP_7(f __VA_OPT__(,) __VA_ARGS__)
#define HAM_IMPL_MAP_9(f, x, ...) HAM_APPLY(f, x) HAM_IMPL_MAP_8(f __VA_OPT__(,) __VA_ARGS__)
#define HAM_IMPL_MAP_10(f, x, ...) HAM_APPLY(f, x) HAM_IMPL_MAP_9(f __VA_OPT__(,) __VA_ARGS__)
#define HAM_IMPL_MAP_11(f, x, ...) HAM_APPLY(f, x) HAM_IMPL_MAP_10(f __VA_OPT__(,) __VA_ARGS__)
#define HAM_IMPL_MAP_12(f, x, ...) HAM_APPLY(f, x) HAM_IMPL_MAP_11(f __VA_OPT__(,) __VA_ARGS__)
#define HAM_IMPL_MAP_13(f, x, ...) HAM_APPLY(f, x) HAM_IMPL_MAP_12(f __VA_OPT__(,) __VA_ARGS__)
#define HAM_IMPL_MAP_14(f, x, ...) HAM_APPLY(f, x) HAM_IMPL_MAP_13(f __VA_OPT__(,) __VA_ARGS__)
#define HAM_IMPL_MAP_15(f, x, ...) HAM_APPLY(f, x) HAM_IMPL_MAP_14(f __VA_OPT__(,) __VA_ARGS__)
#define HAM_IMPL_MAP_16(f, x, ...) HAM_APPLY(f, x) HAM_IMPL_MAP_15(f __VA_OPT__(,) __VA_ARGS__)

#define HAM_IMPL_MAP_N(n, f, ...) HAM_APPLY(HAM_CONCAT(HAM_IMPL_MAP_, n), f __VA_OPT__(,) __VA_ARGS__)
//! @endcond

/**
 * @brief Map \p f to \p n passed arguments.
 */
#define HAM_MAP_N(n, f, ...) HAM_IMPL_MAP_N(n, f __VA_OPT__(,) __VA_ARGS__)

/**
 * @brief Map \p f to all passed arguments.
 */
#define HAM_MAP(f, ...) HAM_MAP_N(HAM_NARGS(__VA_ARGS__), f __VA_OPT__(,) __VA_ARGS__)

/**
 * @}
 */

#endif // !HAM_PP_H
