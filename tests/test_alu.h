#pragma once
#include "ISA.h"

inline void test_alu() {
    std::cout << "Basic ALU Test\n";
    Wave wave(num_regs);
    std::vector<Instruction> program = { 
        MOV(1, 2),
        MOV(2, 3),
        ADD(3, 1, 2),
        MOV(4, 5),
        ADD(1, 3, 4)
    };
    wave.run(program);
    wave.dump_regs();
    wave.simd_stats();
}