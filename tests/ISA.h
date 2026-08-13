#pragma once
#include "../src/wave.h"
#include <iostream>

constexpr int num_regs = 32;

// rd = ra + rb
inline Instruction ADD(Value rd, Value ra, Value rb, uint16_t guard = NO_GUARD, bool neg = false) {
    return {Opcode::ADD_U32, {{{Operand::Kind::Reg, rd}, {Operand::Kind::Reg, ra}, {Operand::Kind::Reg, rb}}}, guard, neg};
}

// rd <- imm
inline Instruction MOV(Value rd, Value imm, uint16_t guard = NO_GUARD, bool neg = false) {
    return {Opcode::MOV_IMM_U32, {{{Operand::Kind::Reg, rd}, {Operand::Kind::Imm, imm}, {Operand::Kind::Imm, 0}}}, guard, neg};
}

// rd = ra < rb ? 1 : 0
inline Instruction SETP_LT(Value rd, Value ra, Value rb, uint16_t guard = NO_GUARD, bool neg = false) {
    return {Opcode::SETP_LT_U32, {{{Operand::Kind::Reg, rd}, {Operand::Kind::Reg, ra}, {Operand::Kind::Reg, rb}}}, guard, neg};
}

// pc = target, reconv pc = reconv_pc, guarded by reg[guard]
inline Instruction BRANCH(Value target, Value reconv_pc, uint16_t guard = NO_GUARD) {
    return {Opcode::BRANCH, {{{Operand::Kind::Imm, target}, {Operand::Kind::Imm, reconv_pc}, {Operand::Kind::Imm, 0}}}, guard, false};
}