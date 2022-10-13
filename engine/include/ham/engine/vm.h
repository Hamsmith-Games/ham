/*
 * Ham World Engine Runtime
 * Copyright (C) 2022 Hamsmith Ltd.
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

#ifndef HAM_ENGINE_VM_H
#define HAM_ENGINE_VM_H 1

/**
 * @defgroup HAM_ENGINE_VM Virtual Machine
 * @ingroup HAM_ENGINE
 * @{
 */

#include "ham/typedefs.h"

#include "ham/engine/config.h"

HAM_C_API_BEGIN

typedef ham_u16 ham_vm_hword;
typedef ham_u32 ham_vm_word;
typedef ham_u64 ham_vm_dword;

#ifdef HAM_INT128
typedef ham_u128 ham_vm_qword;
#else
typedef struct ham_vm_qword{
	ham_vm_word words[4];
	ham_vm_dword dwords[2];
} ham_vm_qword;
#endif

typedef union ham_vm_register{
	ham_vm_hword hwords[8];
	ham_vm_word  words[4];
	ham_vm_dword dwords[2];
	ham_vm_qword qword;
} ham_vm_register;

typedef union alignas(ham_u32) ham_vm_operand{
	struct{
		ham_u8 flag: 1;
		ham_u32 val: 31;
	};

	ham_u32 bits;
} ham_vm_operand;

static_assert(sizeof(ham_vm_operand) == sizeof(ham_u32), "Invalid ham_vm_operand type");

typedef struct alignas(ham_u32) ham_vm_instr{
	ham_vm_word opcode;
	ham_vm_operand operands[3];
} ham_vm_instr;

static_assert(sizeof(ham_vm_instr) == 16, "Invalid ham_vm_instr type");

typedef struct ham_vm ham_vm;
typedef struct ham_vm_prog ham_vm_prog;

ham_engine_api ham_vm *ham_vm_create();
ham_engine_api void ham_vm_destroy(ham_vm *vm);

ham_engine_api const ham_vm_prog *ham_vm_compile(ham_vm *vm, ham_usize num_instrs, const ham_vm_instr *instrs);

ham_engine_api int ham_vm_exec(const ham_vm_prog *prog, const char *entry_point);

HAM_C_API_END

/**
 * @}
 */

#endif // !HAM_ENGINE_VM_H
