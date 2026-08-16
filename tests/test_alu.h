#pragma once
#include "ISA.h"
#include "check.h"

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
    wave.stats();

    EXPECT_REG(wave, 0,  0, 1, 2, 3, 4, 5, 6, 7); // R0 is preloaded with the lane id
    EXPECT_REG(wave, 1, 10,10,10,10,10,10,10,10); // R3 + R4 = 5 + 5
    EXPECT_REG(wave, 2,  3, 3, 3, 3, 3, 3, 3, 3);
    EXPECT_REG(wave, 3,  5, 5, 5, 5, 5, 5, 5, 5); // R1 + R2 = 2 + 3
    EXPECT_REG(wave, 4,  5, 5, 5, 5, 5, 5, 5, 5);
}
