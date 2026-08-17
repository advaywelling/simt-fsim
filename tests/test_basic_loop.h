#pragma once
#include "ISA.h"
#include "check.h"
#include "reference.h"

// R0 holds the lane id, so comparing against R0 gives each lane a different loop count - use for divergence
// the second BRANCH operand (reconv pc) is ignored now, IPDOM computes it.

inline void test_basic_loop() {
    std::cout << "While Loop Test\n";
    Wave wave(num_regs);
    // for (i = 0; i < tid; i++);   lane t runs t iterations
    std::vector<Instruction> program = {
        MOV(2, 1),        // 0  r2 = 1
        MOV(1, 0),        // 1  i = 0
        SETP_LT(3, 1, 0), // 2  header: i < tid
        BRANCH(5, 0, 3),  // 3  -> body
        BRANCH(7, 0),     // 4  exit
        ADD(1, 1, 2),     // 5  body: i++
        BRANCH(2, 0),     // 6  backedge
        MOV(5, 99),       // 7  after the loop
    };
    wave.run(program);
    wave.stats();

    EXPECT_MATCHES_REFERENCE(wave, program);
    EXPECT_REG_FN(wave, 1, [](size_t t) -> uint32_t { return t; }); // i == tid on exit
    EXPECT_REG_FN(wave, 5, [](size_t) -> uint32_t { return 99; });  // every lane reconverged
}

inline void test_loop_do_while() {
    std::cout << "Do-While Loop Test\n";
    Wave wave(num_regs);
    // do { i++; } while (i < tid);   conditional backedge, body runs at least once
    std::vector<Instruction> program = {
        MOV(2, 1),        // 0
        MOV(1, 0),        // 1  i = 0
        ADD(1, 1, 2),     // 2  body: i++
        SETP_LT(3, 1, 0), // 3  i < tid
        BRANCH(2, 0, 3),  // 4  conditional backedge
        MOV(5, 99),       // 5
    };
    wave.run(program);
    wave.stats();
    EXPECT_MATCHES_REFERENCE(wave, program);
}

inline void test_loop_if_inside() {
    std::cout << "If Inside Loop Test\n";
    Wave wave(num_regs);
    // for (i = 0; i < tid; i++) { if (i < 1) r7 = 43; else r6 = 42; }
    std::vector<Instruction> program = {
        MOV(2, 1),        // 0
        MOV(1, 0),        // 1  i = 0
        SETP_LT(3, 1, 0), // 2  header
        BRANCH(5, 0, 3),  // 3
        BRANCH(12, 0),    // 4  exit
        SETP_LT(4, 1, 2), // 5  i < 1
        BRANCH(9, 0, 4),  // 6
        MOV(6, 42),       // 7  else
        BRANCH(10, 0),    // 8
        MOV(7, 43),       // 9  then
        ADD(1, 1, 2),     // 10 i++
        BRANCH(2, 0),     // 11 backedge
        MOV(5, 99),       // 12
    };
    wave.run(program);
    wave.stats();
    EXPECT_MATCHES_REFERENCE(wave, program);
}

inline void test_loop_nested() {
    std::cout << "Nested Loop Test\n";
    Wave wave(num_regs);
    // for (i = 0; i < tid; i++) for (j = 0; j < tid; j++);
    std::vector<Instruction> program = {
        MOV(2, 1),        // 0
        MOV(1, 0),        // 1  i = 0
        SETP_LT(3, 1, 0), // 2  outer header
        BRANCH(5, 0, 3),  // 3
        BRANCH(13, 0),    // 4  exit outer
        MOV(4, 0),        // 5  j = 0
        SETP_LT(5, 4, 0), // 6  inner header
        BRANCH(9, 0, 5),  // 7
        BRANCH(11, 0),    // 8  exit inner
        ADD(4, 4, 2),     // 9  j++
        BRANCH(6, 0),     // 10 inner backedge
        ADD(1, 1, 2),     // 11 i++
        BRANCH(2, 0),     // 12 outer backedge
        MOV(9, 99),       // 13
    };
    wave.run(program);
    wave.stats();
    EXPECT_MATCHES_REFERENCE(wave, program);
    EXPECT_REG_FN(wave, 9, [](size_t) -> uint32_t { return 99; }); // out of both loops
}

inline void test_loop_sequential() {
    std::cout << "Two Sequential Loops Test\n";
    Wave wave(num_regs);
    // two independent divergent loops back to back - the second must start from a clean stack and a full active mask
    std::vector<Instruction> program = {
        MOV(2, 1),        // 0
        MOV(1, 0),        // 1
        SETP_LT(3, 1, 0), // 2  loop 1 header
        BRANCH(5, 0, 3),  // 3
        BRANCH(7, 0),     // 4
        ADD(1, 1, 2),     // 5
        BRANCH(2, 0),     // 6
        MOV(8, 0),        // 7  loop 2 counter
        SETP_LT(9, 8, 0), // 8  loop 2 header
        BRANCH(11, 0, 9), // 9
        BRANCH(13, 0),    // 10
        ADD(8, 8, 2),     // 11
        BRANCH(8, 0),     // 12
        MOV(5, 99),       // 13
    };
    wave.run(program);
    wave.stats();
    EXPECT_MATCHES_REFERENCE(wave, program);
    EXPECT_REG_FN(wave, 5, [](size_t) -> uint32_t { return 99; });
}

inline void test_loop_long_trip() {
    std::cout << "Long Trip Count Loop Test\n";
    Wave wave(num_regs);
    // limit = tid * 256 (double r4 seven times), then loop to it, so the last lane runs thousands of iterations. checks the SIMT stack does not grow with loop count, only with the number of divergence events.
    std::vector<Instruction> program = {
        MOV(2, 1),        // 0
        MOV(1, 0),        // 1  i = 0
        ADD(4, 0, 0),     // 2  r4 = 2 * tid
        ADD(4, 4, 4),     // 3
        ADD(4, 4, 4),     // 4
        ADD(4, 4, 4),     // 5
        ADD(4, 4, 4),     // 6
        ADD(4, 4, 4),     // 7
        ADD(4, 4, 4),     // 8
        ADD(4, 4, 4),     // 9  r4 = 256 * tid
        SETP_LT(3, 1, 4), // 10 header: i < limit
        BRANCH(13, 0, 3), // 11
        BRANCH(15, 0),    // 12 exit
        ADD(1, 1, 2),     // 13 i++
        BRANCH(10, 0),    // 14 backedge
        MOV(5, 99),       // 15
    };
    wave.run(program);
    wave.stats();
    EXPECT_MATCHES_REFERENCE(wave, program);
    EXPECT_REG_FN(wave, 1, [](size_t t) -> uint32_t { return t * 256; });
    EXPECT_REG_FN(wave, 5, [](size_t) -> uint32_t { return 99; });
}
