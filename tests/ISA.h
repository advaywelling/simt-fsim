#pragma once
#include "../src/wave.h"
#include "../src/cfg.h"
#include <iostream>

constexpr int num_regs = 32;

// rd = ra + rb
inline Instruction ADD(uint32_t rd, uint32_t ra, uint32_t rb, uint16_t guard = NO_GUARD, bool neg = false) {
    return {Opcode::ADD_U32, {{{Operand::Kind::Reg, rd}, {Operand::Kind::Reg, ra}, {Operand::Kind::Reg, rb}}}, guard, neg};
}

// rd <- imm
inline Instruction MOV(uint32_t rd, uint32_t imm, uint16_t guard = NO_GUARD, bool neg = false) {
    return {Opcode::MOV_IMM_U32, {{{Operand::Kind::Reg, rd}, {Operand::Kind::Imm, imm}, {Operand::Kind::Imm, 0}}}, guard, neg};
}

// rd = ra < rb ? 1 : 0
inline Instruction SETP_LT(uint32_t rd, uint32_t ra, uint32_t rb, uint16_t guard = NO_GUARD, bool neg = false) {
    return {Opcode::SETP_LT_U32, {{{Operand::Kind::Reg, rd}, {Operand::Kind::Reg, ra}, {Operand::Kind::Reg, rb}}}, guard, neg};
}

// pc = target, reconv pc = reconv_pc, guarded by reg[guard]
inline Instruction BRANCH(uint32_t target, uint32_t reconv_pc, uint16_t guard = NO_GUARD) {
    return {Opcode::BRANCH, {{{Operand::Kind::Imm, target}, {Operand::Kind::Imm, reconv_pc}, {Operand::Kind::Imm, 0}}}, guard, false};
}

// rd = mem[ra + imm]
inline Instruction LW(uint32_t rd, uint32_t ra, uint32_t imm, uint16_t guard = NO_GUARD, bool neg = false) {
    return {Opcode::LW_U32, {{{Operand::Kind::Reg, rd}, {Operand::Kind::Reg, ra}, {Operand::Kind::Imm, imm}}}, guard, neg};
}

// mem[rbase + imm] = rval
inline Instruction SW(uint32_t rbase, uint32_t rval, uint32_t imm, uint16_t guard = NO_GUARD, bool neg = false) {
    return {Opcode::SW_U32, {{{Operand::Kind::Reg, rbase}, {Operand::Kind::Reg, rval}, {Operand::Kind::Imm, imm}}}, guard, neg};
}