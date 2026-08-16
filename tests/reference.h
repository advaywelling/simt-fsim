#pragma once
#include <vector>
#include <cstdint>
#include "check.h"

// Golden model. Runs ONE lane at a time: its own PC, its own registers, no active
// mask, no SIMT stack, no CFG. Got tired of handwriting correct reg states 

struct Reference {
    RegFile regs;
    bool completed = true; // false if a lane blew the step budget (infinite loop?)
};

inline bool ref_guard_passes(const Instruction& in, const std::vector<uint32_t>& r) {
    if (in.guard == NO_GUARD) return true;
    bool pred = r[in.guard] != 0;
    return pred != in.guard_negate;
}

inline uint32_t ref_resolve(const Operand& op, const std::vector<uint32_t>& r) {
    return op.kind == Operand::Kind::Imm ? op.value : r[op.value];
}

inline Reference run_reference(const std::vector<Instruction>& program, size_t num_regs, size_t max_steps = 100000) {
    Reference out;
    out.regs.assign(num_regs, std::array<uint32_t, WAVE_SIZE>{});

    std::vector<uint32_t> gmem(GMEM_SIZE);
    for (size_t i{}; i < GMEM_SIZE; i++) gmem[i] = i * 10; // same fill as Wave's constructor

    for (size_t lane{}; lane < WAVE_SIZE; lane++) {
        std::vector<uint32_t> r(num_regs, 0);
        r[0] = lane; // r0 = lane id

        size_t pc = 0;
        size_t steps = 0;
        while (pc < program.size()) {
            if (++steps > max_steps) { 
                out.completed = false; 
                break; 
            }
            const Instruction& in = program[pc];

            if (!ref_guard_passes(in, r)) { 
                pc++; 
                continue; 
            }

            switch (in.op) {
                case Opcode::MOV_IMM_U32:
                    r[in.operands[0].value] = ref_resolve(in.operands[1], r);
                    break;
                case Opcode::ADD_U32:
                    r[in.operands[0].value] = ref_resolve(in.operands[1], r) + ref_resolve(in.operands[2], r);
                    break;
                case Opcode::SETP_LT_U32:
                    r[in.operands[0].value] = ref_resolve(in.operands[1], r) < ref_resolve(in.operands[2], r) ? 1 : 0;
                    break;
                case Opcode::LW_U32: {
                    uint32_t addr = ref_resolve(in.operands[1], r) + ref_resolve(in.operands[2], r);
                    r[in.operands[0].value] = gmem[addr % GMEM_SIZE];
                    break;
                }
                case Opcode::BRANCH:
                    pc = in.operands[0].value;
                    continue; // pc is already where it needs to be
                default:
                    break; // SW_U32 is unimplemented in Wave too
            }
            pc++;
        }
        for (size_t i{}; i < num_regs; i++) {
            out.regs[i][lane] = r[i];
        }
    }
    return out;
}

inline void expect_matches_reference_impl(const char* file, int line, const Wave& wave, const std::vector<Instruction>& program) {
    checks_run++;
    const RegFile& got = wave.registers();
    Reference ref = run_reference(program, got.size());

    if (!ref.completed) {
        fail_at(file, line, "reference model hit its step budget (infinite loop?)");
        return;
    }
    bool ok = true;
    for (size_t r{}; r < got.size(); r++) {
        if (got[r] == ref.regs[r]) continue;
        if (ok) { fail_at(file, line, "wave does not match reference"); ok = false; }
        std::cout << "    R" << r << " want:";
        for (uint32_t v : ref.regs[r]) std::cout << " " << v;
        std::cout << "\n           got: ";
        for (uint32_t v : got[r]) std::cout << " " << v;
        std::cout << "\n";
    }
}

#define EXPECT_MATCHES_REFERENCE(wave, program) \
    expect_matches_reference_impl(__FILE__, __LINE__, (wave), (program))
