#include <iostream>
#include <vector>
#include <array>
#include <cstdint>
#include <cassert>

constexpr size_t WAVE_SIZE = 8; // warp x wavefront = wave
using Value = uint32_t;
using RegFile = std::vector<std::array<Value, WAVE_SIZE>>; // SoA format
inline constexpr uint16_t NO_GUARD = 0xFFFF;

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

struct Instruction {
    Opcode op;
    std::array<Operand, 3> operands;
    uint16_t guard = NO_GUARD; // predicate reg idx
    bool guard_negate = false;
};

class Wave {
    public: 
        explicit Wave(size_t num_regs);
        void dump_regs() const;
        void run(const std::vector<Instruction>& program);
    private:
        size_t pc {};
        RegFile regs;
        Value resolve(const Operand& op, size_t lane) const;
};

Wave::Wave(size_t num_regs) : regs(num_regs) {
    if (num_regs > 0) {
        for (size_t i{}; i < WAVE_SIZE; i++) {
            regs[0][i] = i;
        }
    } else {
        std::cout << "Why would you want 0 regs";
        std::abort();
    }
}

void Wave::dump_regs() const {
    for(size_t i{}; i < regs.size(); i++) {
        std::cout << "R" << i << ":";
        for(size_t j{}; j < WAVE_SIZE; j++) {
            std::cout << " " << regs[i][j];
        }
        std::cout << "\n";
    }
    std::cout << " ----------------- " << "\n";
}

Value Wave::resolve(const Operand& operand, size_t lane) const {
    if (operand.kind == Operand::Kind::Imm) {
        return operand.value;
    } else if (operand.value < regs.size()){
        return regs[operand.value][lane];
    }
    std::cout << "Register out of bounds";
    std::abort();
}

void Wave::run(const std::vector<Instruction>& program) {
    pc = 0;
    while (pc < program.size()) {
        const Instruction& instr = program[pc];
        switch (instr.op) {
            case Opcode::MOV_IMM_U32 : {
                if (instr.operands[0].value < regs.size()) {
                    for(size_t i{}; i < WAVE_SIZE; i++) {    
                        regs[instr.operands[0].value][i] = resolve(instr.operands[1], i);
                    }
                } else {
                    std::cout << "R" << instr.operands[0].value << " out of bounds\n";
                }
                break;
            }
            case Opcode::ADD_U32 : {
                if (instr.operands[0].value < regs.size()) {
                    for(size_t i{}; i < WAVE_SIZE; i++) {
                        Value val1 = resolve(instr.operands[1], i);
                        Value val2 = resolve(instr.operands[2], i);
                        regs[instr.operands[0].value][i] = val1 + val2;
                    }
                } else {
                    std::cout << "R" << instr.operands[0].value << " out of bounds\n";
                }
                break;
            }
        }
        pc++;
    }
}

int main() {
    constexpr size_t num_regs = 4;
    Wave wave(num_regs);
    std::array<Operand, 3> operands1 = {{{Operand::Kind::Reg, 3}, {Operand::Kind::Imm, 5}, {Operand::Kind::Reg, 2}}};
    std::array<Operand, 3> operands2 = {{{Operand::Kind::Reg, 2}, {Operand::Kind::Imm, 1}, {Operand::Kind::Reg, 2}}};
    std::array<Operand, 3> operands3 = {{{Operand::Kind::Reg, 1}, {Operand::Kind::Imm, 2}, {Operand::Kind::Reg, 3}}};
    std::array<Operand, 3> operands4 = {{{Operand::Kind::Reg, 0}, {Operand::Kind::Reg, 0}, {Operand::Kind::Reg, 1}}};
    Instruction instr1 = {Opcode::MOV_IMM_U32, operands1};
    Instruction instr2 = {Opcode::MOV_IMM_U32, operands2};
    Instruction instr3 = {Opcode::ADD_U32, operands3};
    Instruction instr4 = {Opcode::ADD_U32, operands4};
    std::vector<Instruction> program = {instr1, instr2, instr3, instr4};
    wave.run(program);
    wave.dump_regs();
}
