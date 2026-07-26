#include <iostream>
#include <vector>
#include <array>
#include <cstdint>

constexpr size_t WAVE_SIZE = 8;
constexpr int num_regs = 4;
using Value = uint32_t;
using reg_file = std::vector<std::array<Value, WAVE_SIZE>>;

enum class Opcode : uint8_t {
    MOV_IMM_U32, // rd = imm
    ADD_U32, // rd = ra + ab
};

struct Operand {
    enum class Kind : uint8_t {
        Reg,
        Imm,
    };
    Kind kind;
    Value value; // reg -> reg idx, imm -> the bits
};

inline constexpr uint16_t NO_GUARD = 0xFFFF;

struct Instruction {
    Opcode op;
    std::array<Operand, 3> operands;
    uint16_t guard = NO_GUARD; // predicate reg idx
    bool guard_negate = false;
};

void dump_regs(const reg_file& regs) {
    for(size_t i{}; i < regs.size(); i++) {
        std::cout << "r" << i << ":";
        for(size_t j{}; j < WAVE_SIZE; j++) {
            std::cout << " " << regs[i][j];
        }
        std::cout << "\n";
    }
}

int main() {
    reg_file regs(num_regs);
    for(size_t i{}; i < WAVE_SIZE; i++) {
        regs[0][i] = i;
    }
    dump_regs(regs);
}
