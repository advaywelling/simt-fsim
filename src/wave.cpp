#include "wave.h"
#include <iostream>
#include <cstdlib>

Wave::Wave(size_t num_regs) : regs(num_regs) {
    if (num_regs > 0) {
        for (size_t i{}; i < WAVE_SIZE; i++) {
            regs[0][i] = i;
        }
    } else {
        std::cout << "Why would you want 0 regs";
        std::abort();
    }
}

void Wave::dump_regs() const {
    for(size_t i{}; i < regs.size(); i++) {
        std::cout << "R" << i << ":";
        for(size_t j{}; j < WAVE_SIZE; j++) {
            std::cout << " " << regs[i][j];
        }
        std::cout << "\n";
    }
    std::cout << " ----------------- " << "\n";
}

Value Wave::resolve(const Operand& operand, size_t lane) const {
    if (operand.kind == Operand::Kind::Imm) {
        return operand.value;
    } else if (operand.value < regs.size()){
        return regs[operand.value][lane];
    }
    std::cout << "Register out of bounds";
    std::abort();
}

void Wave::run(const std::vector<Instruction>& program) {
    pc = 0;
    while (pc < program.size()) {
        const Instruction& instr = program[pc];
        switch (instr.op) {
            case Opcode::MOV_IMM_U32 : {
                if (instr.operands[0].value < regs.size()) {
                    for(size_t i{}; i < WAVE_SIZE; i++) {    
                        regs[instr.operands[0].value][i] = resolve(instr.operands[1], i);
                    }
                } else {
                    std::cout << "R" << instr.operands[0].value << " out of bounds\n";
                }
                break;
            }
            case Opcode::ADD_U32 : {
                if (instr.operands[0].value < regs.size()) {
                    for(size_t i{}; i < WAVE_SIZE; i++) {
                        Value val1 = resolve(instr.operands[1], i);
                        Value val2 = resolve(instr.operands[2], i);
                        regs[instr.operands[0].value][i] = val1 + val2;
                    }
                } else {
                    std::cout << "R" << instr.operands[0].value << " out of bounds\n";
                }
                break;
            }
        }
        pc++;
    }
}
