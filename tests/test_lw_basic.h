#pragma once
#include "ISA.h"

inline void test_lw() {
    std::cout << "Basic lw Test\n";
    Wave wave(num_regs);
    std::vector<Instruction> program = { 
        LW(1, 0, 1)
    };
    wave.run(program);
    wave.dump_regs();
    wave.stats();
}