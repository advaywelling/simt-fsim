#include "wave.h"

int main() {
    constexpr size_t num_regs = 4;
    Wave wave(num_regs);
    std::array<Operand, 3> operands1 = {{{Operand::Kind::Reg, 3}, {Operand::Kind::Imm, 5}, {Operand::Kind::Reg, 2}}};
    std::array<Operand, 3> operands2 = {{{Operand::Kind::Reg, 2}, {Operand::Kind::Imm, 1}, {Operand::Kind::Reg, 2}}};
    std::array<Operand, 3> operands3 = {{{Operand::Kind::Reg, 1}, {Operand::Kind::Imm, 2}, {Operand::Kind::Reg, 3}}};
    std::array<Operand, 3> operands4 = {{{Operand::Kind::Reg, 0}, {Operand::Kind::Reg, 0}, {Operand::Kind::Reg, 1}}};
    Instruction instr1 = {Opcode::MOV_IMM_U32, operands1};
    Instruction instr2 = {Opcode::MOV_IMM_U32, operands2};
    Instruction instr3 = {Opcode::ADD_U32, operands3};
    Instruction instr4 = {Opcode::ADD_U32, operands4};
    std::vector<Instruction> program = {instr1, instr2, instr3, instr4};
    wave.run(program);
    wave.dump_regs();
}
