#pragma once
#include "ISA.h"
#include "check.h"

inline void test_lw() {
    std::cout << "Basic lw Test\n";
    Wave wave(num_regs);
    std::vector<Instruction> program = {
        LW(1, 0, 1)
    };
    wave.run(program);
    wave.stats();

    // gmem[i] == i * 10, so lane t loads gmem[tid + 1]
    EXPECT_REG(wave, 1, 10, 20, 30, 40, 50, 60, 70, 80);
}
