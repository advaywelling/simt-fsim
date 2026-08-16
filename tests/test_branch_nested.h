#pragma once
#include "ISA.h"
#include "check.h"

inline void test_branch_nested() {
    std::cout << "Nested Branch Test\n";
    Wave wave(num_regs);
    std::vector<Instruction> program = {
        MOV(1,4),
        SETP_LT(2, 0, 1),
        // branch 1
        BRANCH(5,12,2),
        MOV(3,5), // false path
        BRANCH(12, 12), // tell false lanes to reconverge
        MOV(11, 2), // true path
        ADD(4, 11, 0),
        SETP_LT(2, 4, 1),
        // branch 2 (in true path)
        BRANCH(11, 12, 2),
        MOV(15, 10), // false path
        BRANCH(12, 12), // tell false to reconverge
        MOV(16, 11), // true path
        MOV(5, 10) // everyone reconv
    };
    wave.run(program);
    wave.stats();

    // outer branch: lanes 0-3 take it (tid < 4), lanes 4-7 fall through
    // inner branch runs only on lanes 0-3, where R4 = 2 + tid, so lanes 0-1 take it
    EXPECT_REG(wave, 1,  4, 4, 4, 4, 4, 4, 4, 4);
    EXPECT_REG(wave, 2,  1, 1, 0, 0, 0, 0, 0, 0); // reused: inner setp on 0-3, stale outer setp on 4-7
    EXPECT_REG(wave, 3,  0, 0, 0, 0, 5, 5, 5, 5); // outer false path
    EXPECT_REG(wave, 4,  2, 3, 4, 5, 0, 0, 0, 0); // R11 + R0 = 2 + tid, outer true path only
    EXPECT_REG(wave, 5, 10,10,10,10,10,10,10,10); // everyone reconverges
    EXPECT_REG(wave, 11, 2, 2, 2, 2, 0, 0, 0, 0); // outer true path
    EXPECT_REG(wave, 15, 0, 0,10,10, 0, 0, 0, 0); // inner false path
    EXPECT_REG(wave, 16,11,11, 0, 0, 0, 0, 0, 0); // inner true path
}
