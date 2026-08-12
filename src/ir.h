#pragma once
#include <cstdint>
#include <vector>
#include <array>

constexpr size_t WAVE_SIZE = 8; // warp/wavefront
using Value = uint32_t;
using RegFile = std::vector<std::array<Value, WAVE_SIZE>>; // SoA format
inline constexpr uint16_t NO_GUARD = 0xFFFF;

enum class Opcode : uint8_t {
    MOV_IMM_U32, // rd = imm
    ADD_U32, // rd = ra + ab
    BRANCH, // unconditional branch
    SETP_LT_U32, // set pred rd_lane = ra_lane < rb_lane ? 1 : 0 
};

struct Operand {
    enum class Kind : uint8_t {
        Reg,
        Imm,
    };
    Kind kind;
    Value value; // reg -> reg idx, imm -> the bits
};

struct Instruction {
    Opcode op;
    std::array<Operand, 3> operands;
    uint16_t guard = NO_GUARD; // predicate reg idx
    bool guard_negate = false;
};