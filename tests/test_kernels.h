#pragma once
#include "ISA.h"
#include "check.h"
#include "reference.h"

// small kernels to test metrics, praying fr

inline void kernel_vector_add() {
    std::cout << "Kernel: vector add, coalesced\n";
    Wave wave(num_regs);
    // c[tid] = a[tid] + b[tid], every lane hitting a consecutive word
    std::vector<Instruction> program = {
        LW(1, 0, 0),    // 0  r1 = a[tid]
        LW(2, 0, 64),   // 1  r2 = b[tid]
        ADD(3, 1, 2),   // 2
        SW(0, 3, 128),  // 3  c[tid] = r3
        LW(4, 0, 128),  // 4  read back to prove the store landed
    };
    wave.run(program);
    wave.stats();
    EXPECT_MATCHES_REFERENCE(wave, program);
    EXPECT_REG_FN(wave, 4, [](size_t t) -> uint32_t {
        return static_cast<uint32_t>((t * 10) + ((t + 64) * 10));
    });
}

inline void kernel_stride_2() {
    std::cout << "Kernel: strided load, stride 2\n";
    Wave wave(num_regs);
    std::vector<Instruction> program = {
        ADD(2, 0, 0),   // 0  r2 = 2 * tid
        LW(1, 2, 0),    // 1  r1 = a[2 * tid]
    };
    wave.run(program);
    wave.stats();
    EXPECT_MATCHES_REFERENCE(wave, program);
}

inline void kernel_stride_32() {
    std::cout << "Kernel: strided load, stride 32 (worst case)\n";
    Wave wave(num_regs);
    // every lane lands in its own segment
    std::vector<Instruction> program = {
        ADD(2, 0, 0),   // 0  2 * tid
        ADD(2, 2, 2),   // 1  4
        ADD(2, 2, 2),   // 2  8
        ADD(2, 2, 2),   // 3  16
        ADD(2, 2, 2),   // 4  r2 = 32 * tid
        LW(1, 2, 0),    // 5
    };
    wave.run(program);
    wave.stats();
    EXPECT_MATCHES_REFERENCE(wave, program);
}

inline void kernel_divergent_branch() {
    std::cout << "Kernel: divergent if/else, half the wave each way\n";
    Wave wave(num_regs);
    std::vector<Instruction> program = {
        MOV(1, WAVE_SIZE / 2), // 0
        SETP_LT(2, 0, 1),      // 1  tid < 16
        BRANCH(5, 0, 2),       // 2
        MOV(3, 100),           // 3  else half
        BRANCH(6, 0),          // 4
        MOV(4, 200),           // 5  then half
        MOV(5, 1),             // 6  reconverged
    };
    wave.run(program);
    wave.stats();
    EXPECT_MATCHES_REFERENCE(wave, program);
}

inline void kernel_divergent_loop() {
    std::cout << "Kernel: divergent loop, lane t runs t iterations\n";
    Wave wave(num_regs);
    std::vector<Instruction> program = {
        MOV(2, 1),        // 0
        MOV(1, 0),        // 1
        SETP_LT(3, 1, 0), // 2  header
        BRANCH(5, 0, 3),  // 3
        BRANCH(7, 0),     // 4  exit
        ADD(1, 1, 2),     // 5  body
        BRANCH(2, 0),     // 6  backedge
        MOV(5, 99),       // 7
    };
    wave.run(program);
    wave.stats();
    EXPECT_MATCHES_REFERENCE(wave, program);
}

inline void test_kernels() {
    kernel_vector_add();
    std::cout << "\n";
    kernel_stride_2();
    std::cout << "\n";
    kernel_stride_32();
    std::cout << "\n";
    kernel_divergent_branch();
    std::cout << "\n";
    kernel_divergent_loop();
}