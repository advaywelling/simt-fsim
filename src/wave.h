#pragma once
#include "ir.h"

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
