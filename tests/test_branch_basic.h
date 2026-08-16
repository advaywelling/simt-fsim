#pragma once
#include "ISA.h"
#include "check.h"
#include "reference.h"

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
    wave.stats();

    // covers every register at once
    EXPECT_MATCHES_REFERENCE(wave, program);

    // hand-derived anchors: lanes 0-3 take the branch (tid < 4), lanes 4-7 fall through
    EXPECT_REG(wave, 1,  4, 4, 4, 4, 4, 4, 4, 4);
    EXPECT_REG(wave, 2,  1, 1, 1, 1, 0, 0, 0, 0); // tid < 4
    EXPECT_REG(wave, 3,  0, 0, 0, 0, 5, 5, 5, 5); // written on the false path only
    EXPECT_REG(wave, 4,  1, 1, 1, 1, 0, 0, 0, 0); // true path: R2 + R3 = 1 + 0
    EXPECT_REG(wave, 5, 10,10,10,10,10,10,10,10); // after reconvergence, every lane runs this
}
