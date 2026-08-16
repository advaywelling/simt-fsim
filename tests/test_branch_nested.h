#pragma once
#include "ISA.h"
#include "reference.h"

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
    EXPECT_MATCHES_REFERENCE(wave, program);
}
