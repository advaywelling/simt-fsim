#pragma once
#include <cstdint>
#include <vector>
#include <array>

constexpr size_t WAVE_SIZE = 32; // warp/wavefront
using RegFile = std::vector<std::array<uint32_t, WAVE_SIZE>>; // SoA format
inline constexpr uint16_t NO_GUARD = 0xFFFF;
constexpr size_t GMEM_SIZE = 16384;
constexpr size_t GMEM_SEGMENT_SIZE = 32; // line/segment size = 128 words / 1024 bytes

enum class Opcode : uint8_t {
    MOV_IMM_U32, // rd = imm
    ADD_U32, // rd = ra + ab
    BRANCH, // unconditional branch
    SETP_LT_U32, // set pred rd_lane = ra_lane < rb_lane ? 1 : 0 
    LW_U32, // rd = mem[ra + imm]
    SW_U32, // mem[rd + imm] = ra
};

struct Operand {
    enum class Kind : uint8_t {
        Reg,
        Imm,
    };
    Kind kind;
    uint32_t value; // reg -> reg idx, imm -> the bits
};

struct Instruction {
    Opcode op;
    std::array<Operand, 3> operands;
    uint16_t guard = NO_GUARD; // predicate reg idx
    bool guard_negate = false;
};