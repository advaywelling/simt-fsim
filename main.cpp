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

    std::cout << " ---------- " << "\n";
}

int main() {
    reg_file regs(num_regs);
    for (size_t i{}; i < regs.size(); i++) {
        for (size_t j{}; j < WAVE_SIZE; j++) {
            regs[i][j] = i + j;
        }
    }
    dump_regs(regs);
    Operand dest = {Operand::Kind::Reg, 0};
    Operand src1 = {Operand::Kind::Reg, 1};
    Operand src2 = {Operand::Kind::Reg, 2};
    std::array<Operand, 3> operands = {dest, src1, src2};
    Instruction instr = {Opcode::ADD_U32, operands};
    switch (instr.op) {
        case Opcode::MOV_IMM_U32 : {
            for(size_t i{}; i < WAVE_SIZE; i++) {
                regs[instr.operands[0].value][i] = instr.operands[1].value;
            }
            break;
        }
        case Opcode::ADD_U32 : {
            for(size_t i{}; i < WAVE_SIZE; i++) {
                Value val1 = (instr.operands[1].kind == Operand::Kind::Reg) ? regs[instr.operands[1].value][i] : instr.operands[1].value;
                Value val2 = (instr.operands[2].kind == Operand::Kind::Reg) ? regs[instr.operands[2].value][i] : instr.operands[2].value;
                regs[instr.operands[0].value][i] = val1 + val2;
                dump_regs(regs);
            }
            break;
        }
        default:
            break;
    }

    //dump_regs(regs);
}
