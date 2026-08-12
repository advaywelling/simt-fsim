#pragma once
#include "ir.h"

struct ReconvEntry {
    std::array<bool, WAVE_SIZE> mask; // which lans the deferred path runs on
    size_t resume_pc; // where deferred path starts
    size_t reconv_pc; // where everyone reconverges
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
        std::array<bool, WAVE_SIZE> active_mask; // who's active rn
        std::vector<ReconvEntry> simt_stack; // all deferred lanes
        void execute(const Instruction& instr); // execute 1 instr
        
};
