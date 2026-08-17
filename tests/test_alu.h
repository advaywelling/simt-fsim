#pragma once
#include "ISA.h"
#include "check.h"
#include "reference.h"

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

    // covers every register at once
    EXPECT_MATCHES_REFERENCE(wave, program);

    // hand-derived anchors: these pin down what the ISA is supposed to mean,
    // which the reference model cannot do for us (it could be wrong the same way)
    EXPECT_REG_FN(wave, 0, [](size_t lane) -> uint32_t { return lane; }); // preloaded lane id
    EXPECT_REG_FN(wave, 1, [](size_t) -> uint32_t { return 10; });        // R3 + R4 = 5 + 5
    EXPECT_REG_FN(wave, 2, [](size_t) -> uint32_t { return 3; });
    EXPECT_REG_FN(wave, 3, [](size_t) -> uint32_t { return 5; });         // R1 + R2 = 2 + 3
    EXPECT_REG_FN(wave, 4, [](size_t) -> uint32_t { return 5; });
}
