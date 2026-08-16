#include "cfg.h"

CFG build_cfg(const std::vector<Instruction>& program) {
    CFG cfg;
    size_t n = program.size();
    cfg.pc_to_block.resize(n);

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
            if (target < n) is_leader[target] = true; // rule 2
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
        size_t fallthrough = cfg.blocks[i].end_pc + 1;
        bool within_bounds = fallthrough < program.size();
        // conditional branch
        if (curr_instr.op == Opcode::BRANCH && curr_instr.guard != NO_GUARD) {
            cfg.blocks[i].edges.push_back(cfg.pc_to_block[curr_instr.operands[0].value]);
            if (within_bounds) {
                cfg.blocks[i].edges.push_back(cfg.pc_to_block[fallthrough]); // fall-through block
            }
        }
        // unconditional branch
        else if (curr_instr.op == Opcode::BRANCH && curr_instr.guard == NO_GUARD) {
            cfg.blocks[i].edges.push_back(cfg.pc_to_block[curr_instr.operands[0].value]); // target block
        }
        // not a branch
        else {
            if (within_bounds) {
                cfg.blocks[i].edges.push_back(cfg.pc_to_block[cfg.blocks[i].end_pc + 1]); // fall-through block
            }
        }
    }   
    return cfg;
}

void print_cfg(const CFG& cfg) {
    for (size_t i = 0; i < cfg.blocks.size(); i++) {
        const BasicBlock& b = cfg.blocks[i];
        std::cout << "BB" << i << " [" << b.start_pc << "-" << b.end_pc << "] -> {";
        for (size_t s = 0; s < b.edges.size(); s++) {
            std::cout << "BB" << b.edges[s];
            if (s + 1 < b.edges.size()) std::cout << ", ";
        }
        std::cout << "}\n";
    }
}

std::vector<std::vector<size_t>> compute_postdom(const CFG& cfg) {
    size_t num_blocks = cfg.blocks.size();
    std::vector<std::set<size_t>> postdom(num_blocks);

    // find exit block
    size_t exit = 0;
    for(size_t i{}; i < num_blocks; i++) {
        if (cfg.blocks[i].edges.empty()) {
            exit = i;
            break;
        }
    }

    // build pdom sets optimistically 
    // if last block then pdom set is just itself, otherwise start with all blocks
    for(size_t i{}; i < num_blocks; i++) {
        if (i == exit) {
            postdom[i].insert(exit);
        } else {
            for(size_t j{}; j < num_blocks; j++) {
                postdom[i].insert(j);
            }
        }
    }

    // iterate until nothing changes
    bool changed = true;
    while (changed) {
        changed = false;
        for(size_t i {}; i < num_blocks; i++) {
            if (i == exit) continue;

            const auto& curr_block_edges = cfg.blocks[i].edges; // all edges of curr block
            std::set<size_t> new_pdom;

            if (!curr_block_edges.empty()) {
                new_pdom = postdom[curr_block_edges[0]];
                for(size_t i = 1; i < curr_block_edges.size(); i++) {
                    std::set<size_t> temp;
                    // for each successor, only insert if ALSO in successing successors (say that quickly 5 times)
                    for(size_t x : new_pdom) {
                        if(postdom[curr_block_edges[i]].count(x)) {
                            temp.insert(x);
                        }
                    }
                    new_pdom = temp; // new pdom set shrinks to common pdoms
                }
            }
            new_pdom.insert(i); // insert itself

            if(new_pdom != postdom[i]) {
                postdom[i] = new_pdom;
                changed = true;
            }
        }
    }
}