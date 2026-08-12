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

void Wave::execute(const Instruction& instr) {
    switch (instr.op) {
        case Opcode::MOV_IMM_U32: {
            for (size_t i {}; i < WAVE_SIZE; i++) {
                if (!active_mask[i]) continue; // dont execute masked lanes
                regs[instr.operands[0].value][i] = resolve(instr.operands[1], i);
            }
            break;
        }
        case Opcode::ADD_U32: {
            for (size_t i {}; i < WAVE_SIZE; i++) {
                if (!active_mask[i]) continue;
                Value a = resolve(instr.operands[1], i);
                Value b = resolve(instr.operands[2], i);
                regs[instr.operands[0].value][i] = a + b;
            }
            break;
        }
        case Opcode::SETP_LT_U32: {
            for (size_t i {}; i < WAVE_SIZE; i++) {
                if (!active_mask[i]) continue;
                Value a = resolve(instr.operands[1], i);
                Value b = resolve(instr.operands[2], i);
                regs[instr.operands[0].value][i] = (a < b) ? 1 : 0;
            }
            break;
        }
        // BRA handled in run, not here
    }
}

void Wave::run(const std::vector<Instruction>& program) {
    pc = 0;
    for(size_t i{}; i < WAVE_SIZE; i++) {
        active_mask[i] = true; // set all masks active to start with
    }
    simt_stack.clear();
    while (pc < program.size()) {
        if (pc == curr_reconv_pc) {
            ReconvEntry top = simt_stack.back();
            simt_stack.pop_back();
            if (top.entry == ReconvEntry::Entry_Type::PATH) {
                active_mask = top.mask;
                pc = top.resume_pc;
                curr_reconv_pc = top.reconv_pc;
            }
            else { // entry type is JOIN
                active_mask = top.mask;
                curr_reconv_pc = SIZE_MAX;
            }
            continue;
        }
        const Instruction& instr = program[pc];
        if (instr.op == Opcode::BRANCH) {
            Value target = instr.operands[0].value;
            Value reconv_pc = instr.operands[1].value;
            std::array<bool, WAVE_SIZE> taken_mask {};
            std::array<bool, WAVE_SIZE> fall_mask {};
            for(size_t i {}; i < WAVE_SIZE; i++) {
                if (!active_mask[i]) continue;
                bool pred_status = regs[instr.guard][i] != 0;
                if (pred_status) {
                    taken_mask[i] = true;
                } else {
                    fall_mask[i] = true;
                }
            }
            
        }
        execute(instr);
        pc++;
    }
}
