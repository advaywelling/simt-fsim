#pragma once
#include "ISA.h"

inline void test_branch_basic() {
    std::cout << "Basic Branch Test\n";
    Wave wave(num_regs);
    std::vector<Instruction> program = { 
        MOV(1,4),
        SETP_LT(2, 0, 1),
        BRANCH(5,6,2),
        MOV(3,5), // false path
        BRANCH(6, 6), // tell false lanes to reconverge
        ADD(4, 2, 3), // true path
        MOV(5, 10) // reconv
    };
    wave.run(program);
    wave.dump_regs();
}