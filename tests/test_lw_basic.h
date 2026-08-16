#pragma once
#include "ISA.h"
#include "reference.h"

inline void test_lw() {
    std::cout << "Basic lw Test\n";
    Wave wave(num_regs);
    std::vector<Instruction> program = {
        LW(1, 0, 1)
    };
    wave.run(program);
    wave.stats();

    // gmem[i] == i * 10, so lane t loads gmem[tid + 1]
    EXPECT_MATCHES_REFERENCE(wave, program);
}
