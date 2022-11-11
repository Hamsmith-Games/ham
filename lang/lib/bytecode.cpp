/*
 * Ham Programming Language Runtime
 * Copyright (C) 2022 Keith Hammond
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "ham/math.h"

HAM_C_API_BEGIN

typedef union ham_vm_reg{
	ham_u8  _8;
	ham_u16 _16;
	ham_u32 _32;
	ham_u64 _64;
#ifdef HAM_INT128
	ham_u128 _128;
#else
	struct {
		ham_u64 hi, lo;
	} _128;
#endif

#ifdef HAM_SIMD
	ham_v4f32 v4f32;
	ham_v4i32 v4i32;

	ham_v2f64 v2f64;
	ham_v2i64 v2i64;
#endif
} ham_vm_reg;

typedef struct ham_vm{
	ham_vm_reg regs[16];
	void *stack_p_beg, *stack_p;
} ham_vm;

typedef enum ham_bytecode_type{
	HAM_BYTECODE_I8,
	HAM_BYTECODE_I16,
	HAM_BYTECODE_I32,
	HAM_BYTECODE_I64,
	HAM_BYTECODE_I128,

	HAM_BYTECODE_U8,
	HAM_BYTECODE_U16,
	HAM_BYTECODE_U32,
	HAM_BYTECODE_U64,
	HAM_BYTECODE_U128,

	HAM_BYTECODE_Q8,
	HAM_BYTECODE_Q16,
	HAM_BYTECODE_Q32,
	HAM_BYTECODE_Q64,
	HAM_BYTECODE_Q128,
	HAM_BYTECODE_Q256,

	HAM_BYTECODE_F16,
	HAM_BYTECODE_F32,
	HAM_BYTECODE_F64,
	HAM_BYTECODE_F128,

	HAM_BYTECODE_STR,

	HAM_BYTECODE_TYPE_COUNT
} ham_bytecode_type;

typedef enum ham_bytecode_op_kind{
	HAM_BYTECODE_OP_NOOP,
	HAM_BYTECODE_OP_EXIT,
	HAM_BYTECODE_OP_LABEL,

	HAM_BYTECODE_OP_STORE,
	HAM_BYTECODE_OP_LOAD,

	HAM_BYTECODE_OP_CAST,
	HAM_BYTECODE_OP_ADD,
	HAM_BYTECODE_OP_SUB,
	HAM_BYTECODE_OP_MUL,
	HAM_BYTECODE_OP_DIV,

	HAM_BYTECODE_OP_CALL,

	HAM_BYTECODE_OP_KIND_COUNT
} ham_bytecode_op_kind;

enum ham_bytecode_arg_kind{
	HAM_BYTECODE_ARG_REG,
	HAM_BYTECODE_ARG_PTR,
	HAM_BYTECODE_ARG_LIT,

	HAM_BYTECODE_ARG_KIND_COUNT
} ham_bytecode_arg_kind;

typedef union ham_bytecode_op{
	struct {
		unsigned kind: 7;
		unsigned type: 5;
	};

	ham_u16 bits;
} ham_bytecode_op;

typedef union ham_bytecode_arg{
	ham_i8  i8;
	ham_i16 i16;
	ham_i32 i32;
	ham_i64 i64;
#ifdef HAM_INT128
	ham_i128 i128;
#endif

	ham_u8  u8;
	ham_u16 u16;
	ham_u32 u32;
	ham_u64 u64;
#ifdef HAM_INT128
	ham_u128 u128;
#endif

	ham_rat8   q8;
	ham_rat16  q16;
	ham_rat32  q32;
	ham_rat64  q64;
	ham_rat128 q128;
	ham_rat256 q256;

#ifdef HAM_FLOAT16
	ham_f16 f16;
#endif
	ham_f32 f32;
	ham_f64 f64;
#ifdef HAM_FLOAT128
	ham_f128 f128;
#endif

	ham_str8  str8;
	ham_str16 str16;
	ham_str32 str32;

	ham_vec2 v2;
	ham_vec3 v3;
	ham_vec4 v4;

	ham_vec2i v2i;
	ham_vec3i v3i;
	ham_vec4i v4i;
} ham_bytecode_arg;

typedef struct ham_bytecode{
	ham_usize globals_size;
	ham_u8 *globals;

	ham_usize code_size;
	ham_u8 *code;
} ham_bytecode;

ham_used
ham_nothrow static inline ham_usize ham_bytecode_op_min_args(ham_bytecode_op op){
	switch(op.kind){
		case HAM_BYTECODE_OP_NOOP:
		case HAM_BYTECODE_OP_EXIT:
			return 0;

		case HAM_BYTECODE_OP_LABEL:
			return 1;

		case HAM_BYTECODE_OP_STORE:
		case HAM_BYTECODE_OP_LOAD:
			return 2;

		case HAM_BYTECODE_OP_CAST:
			return 1;

		case HAM_BYTECODE_OP_ADD:
		case HAM_BYTECODE_OP_SUB:
		case HAM_BYTECODE_OP_MUL:
		case HAM_BYTECODE_OP_DIV:
			return 3;

		case HAM_BYTECODE_OP_CALL:
			return 1;

		default: return (ham_usize)-1;
	}
}

ham_nothrow bool ham_bytecode_validate(const ham_bytecode *bc);

ham_used
ham_nothrow static inline bool ham_bytecode_exec(const ham_bytecode *bc){
	if(!ham_bytecode_validate(bc)){
		return false;
	}

	const ham_u8 *labels[256];
	ham_u32 label_counter = 0;

	const ham_u8 *instr_p = bc->code;
	const ham_u8 *const instr_p_end = bc->code + bc->code_size;

	while(instr_p < instr_p_end){
		ham_bytecode_op op;
		memcpy(&op, instr_p, sizeof(ham_bytecode_op));
		instr_p += sizeof(ham_bytecode_op);

		switch(op.kind){
			case HAM_BYTECODE_OP_NOOP: continue;
			case HAM_BYTECODE_OP_EXIT: return true;

			case HAM_BYTECODE_OP_LABEL:{
				ham_u8 reg_i;
				memcpy(&reg_i, instr_p, sizeof(reg_i));

				labels[label_counter++] = instr_p;
				break;
			}

			default: break;
		}
	}

	return true;
}

HAM_C_API_END
