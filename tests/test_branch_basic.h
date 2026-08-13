#pragma once
#include "ISA.h"

inline void test_branch_basic() {
    std::cout << "Basic Branch Test\n";
    Wave wave(32);
    std::vector<Instruction> program = { 
        MOV(1,4),
        SETP_LT(2, 0, 1),
        BRANCH(4,5,2),
        MOV(3,5), // false path
        ADD(3, 1, 2), // true path
        MOV(4, 10) // reconv
    };
    wave.run(program);
    wave.dump_regs();
}