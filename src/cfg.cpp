#include "cfg.h"

CFG build_cfg(const std::vector<Instruction>& program) {
    CFG cfg;
    size_t n = program.size();
    cfg.pc_to_block.resize(n + 1);

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

    // end of program block
    BasicBlock end_block;
    end_block.start_pc = end_block.end_pc = n;
    cfg.blocks.push_back(end_block);
    cfg.pc_to_block[n] = cfg.blocks.size() - 1;

    // build edges between blocks (nodes) 
    // stop before exit block
    for (size_t i{}; i < cfg.blocks.size() - 1; i++) { 
        Instruction curr_instr = program[cfg.blocks[i].end_pc];
        size_t fallthrough = cfg.blocks[i].end_pc + 1;
        bool within_bounds_fallthrough = fallthrough < program.size();
        size_t jump_pc = curr_instr.operands[0].value;
        bool within_bounds_jump = jump_pc < program.size();
        // conditional branch
        if (curr_instr.op == Opcode::BRANCH && curr_instr.guard != NO_GUARD) {
            if (within_bounds_jump) {
                cfg.blocks[i].edges.push_back(cfg.pc_to_block[jump_pc]);
            }
            else {
                cfg.blocks[i].edges.push_back(cfg.pc_to_block[program.size()]);
            }
            if (within_bounds_fallthrough) {
                cfg.blocks[i].edges.push_back(cfg.pc_to_block[fallthrough]); // fall-through block
            }
            else {
                cfg.blocks[i].edges.push_back(cfg.pc_to_block[program.size()]);
            }
        }
        // unconditional branch
        else if (curr_instr.op == Opcode::BRANCH && curr_instr.guard == NO_GUARD) {
            if (within_bounds_jump) {
                cfg.blocks[i].edges.push_back(cfg.pc_to_block[jump_pc]); // target block
            }
            else {
                cfg.blocks[i].edges.push_back(cfg.pc_to_block[program.size()]);
            }
        }
        // not a branch
        else {
            if (within_bounds_fallthrough) {
                cfg.blocks[i].edges.push_back(cfg.pc_to_block[fallthrough]); // fall-through block
            }
            else {
                cfg.blocks[i].edges.push_back(cfg.pc_to_block[program.size()]);
            }
        }
    }   
    return cfg;
}

std::vector<std::set<size_t>> compute_postdom(const CFG& cfg) {
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
                for(size_t j = 1; j < curr_block_edges.size(); j++) {
                    std::set<size_t> temp;
                    // for each successor, only insert if ALSO in successing successors (say that quickly 5 times)
                    for(size_t x : new_pdom) {
                        if(postdom[curr_block_edges[j]].count(x)) {
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
    return postdom;
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
    std::vector<std::set<size_t>> pdom = compute_postdom(cfg);
    for (size_t i{}; i < pdom.size(); i++) {
        std::cout << "postdom(BB" << i << ") = {";
        for (size_t x : pdom[i]) {
            std::cout << " " << x << " ";
        }
        std::cout << "}\n";
    }
    std::vector<size_t> ipdom = compute_ipdom(cfg, pdom);
    for (size_t i{}; i < ipdom.size(); i++) {
        std::cout << "ipdom(BB" << i << ") = ";
        if (ipdom[i] == SIZE_MAX) std::cout << "none\n";
        else std::cout << "BB" << ipdom[i] << "\n";
    }
}

std::vector<size_t> compute_ipdom(const CFG& cfg, const std::vector<std::set<size_t>>& pdom) {
    size_t num_blocks = cfg.blocks.size();
    std::vector<size_t> ipdom(num_blocks, SIZE_MAX);

    for (size_t i{}; i < num_blocks; i++) { 
        size_t smallest_pc = SIZE_MAX;
        size_t earliest_block = SIZE_MAX;
        for(size_t j : pdom[i]) { // each block j that post dominates block i
            if (j == i) continue;
            if (cfg.blocks[i].start_pc < cfg.blocks[j].start_pc && cfg.blocks[j].start_pc < smallest_pc) { // downstream + smallest pc yet
                smallest_pc = cfg.blocks[j].start_pc;
                earliest_block = j;
            }
        }
        ipdom[i] = earliest_block;
    }
    return ipdom;
}