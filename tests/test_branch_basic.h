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
    EXPECT_REG_FN(wave, 1, [](size_t) -> uint32_t { return 4; });
    EXPECT_REG_FN(wave, 2, [](size_t t) -> uint32_t { return t < 4 ? 1 : 0; }); // tid < 4
    EXPECT_REG_FN(wave, 3, [](size_t t) -> uint32_t { return t < 4 ? 0 : 5; }); // false path only
    EXPECT_REG_FN(wave, 4, [](size_t t) -> uint32_t { return t < 4 ? 1 : 0; }); // true: R2 + R3
    EXPECT_REG_FN(wave, 5, [](size_t) -> uint32_t { return 10; });              // after reconvergence
}
