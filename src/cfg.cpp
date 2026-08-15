#include "cfg.h"

CFG build_cfg(const std::vector<Instruction>& program) {
    CFG cfg;
    size_t n = program.size();

    // find block leaders (start of basic block)
    // 3 rules
    // rule 1 -> start of program = leader
    // rule 2 -> branch target = leader (taken path)
    // rule 3 -> branch fall through - leader (not taken path)
    std::vector<bool> is_leader(n, false);
    if (n) is_leader[0] = true; // rule 1
    for (size_t i{}; i < n; i++) {
        if (program[i].op == Opcode::BRANCH) {
            size_t target = program[i].operands[0].value; 
            if (target < n) is_leader[i] = true; // rule 2
            if (i + 1 < n) is_leader[i + 1] = true; // rule 3
        }
    }

    // use leaders to build basic blocks
    for (size_t i{}; i < n; i++) {
        if (is_leader[i]) {
            if (!cfg.blocks.empty()) {
                cfg.blocks.back().end_pc = i - 1;
            }
            BasicBlock new_block;
            new_block.start_pc = i;
            cfg.blocks.push_back(new_block);
        }
        cfg.pc_to_block[i] = cfg.blocks.size() - 1; // which block is instr i in
    }
    if (!cfg.blocks.empty()) {
        cfg.blocks.back().end_pc = n - 1;
    }

    // build edges between blocks (nodes)
    for (size_t i{}; i < cfg.blocks.size(); i++) {
        Instruction curr_instr = program[cfg.blocks[i].end_pc];
        // conditional branch
        if (curr_instr.op == Opcode::BRANCH && curr_instr.guard != NO_GUARD) {
            cfg.blocks[i].edges.push_back(cfg.pc_to_block[cfg.blocks[i].end_pc + 1]); // fall-through block
        }
        // unconditional branch
        else if (curr_instr.op == Opcode::BRANCH && curr_instr.guard == NO_GUARD) {
            cfg.blocks[i].edges.push_back(cfg.pc_to_block[curr_instr.operands[0].value]); // target block
        }
        // not a branch
        else if (cfg.blocks[i].end_pc + 1 < program.size()){
            cfg.blocks[i].edges.push_back(cfg.pc_to_block[cfg.blocks[i].end_pc + 1]); // fall-through block
        }
    }
}