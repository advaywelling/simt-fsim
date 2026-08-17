#pragma once
#include "IR.h"

struct ReconvEntry {
    std::array<bool, WAVE_SIZE> mask; // lanes to run 
    size_t resume_pc = SIZE_MAX; // where deferred path starts, unused for type JOIN
    size_t reconv_pc = SIZE_MAX; // where everyone reconverges
    enum class Entry_Type : uint8_t {
        PATH,
        JOIN,
    };
    Entry_Type entry;
};

class Wave {
    public: 
        explicit Wave(size_t num_regs);
        void dump_regs() const;
        void run(const std::vector<Instruction>& program);
        void stats() const;
        const RegFile& registers() const { return regs; } // for tests to assert on

    private:
        size_t pc {};
        RegFile regs;
        std::vector<uint32_t> gmem; // yes i know gmem is NOT per wave but ill change this sometime
        uint32_t resolve(const Operand& op, size_t lane) const;
        std::array<bool, WAVE_SIZE> active_mask; // who's active rn
        std::vector<ReconvEntry> simt_stack; // all deferred lanes
        void execute(const Instruction& instr); // execute 1 instr
        std::array<bool, WAVE_SIZE> exec_mask(const Instruction& instr) const; // active AND guard passes
        void count_issue(const std::array<bool, WAVE_SIZE>& mask); // one instruction issue
        void count_mem(size_t segments, size_t lanes); // one memory instruction
        size_t curr_reconv_pc = SIZE_MAX;
        size_t total_active_lanes {};
        size_t ideal_active_lanes {};
        size_t total_mem_transfers {};
        size_t ideal_mem_transfers {};
};
