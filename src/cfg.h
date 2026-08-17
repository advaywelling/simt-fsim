#pragma once
#include <cstdlib>
#include <vector>
#include <iostream>
#include <set>
#include "IR.h"

struct BasicBlock {
    size_t start_pc {};
    size_t end_pc {};
    std::vector<size_t> edges;
};

struct CFG {
    std::vector<BasicBlock> blocks;
    std::vector<size_t> pc_to_block;
    size_t exit_block {}; // virtual block at pc == program.size(), the only one with no successors
};

CFG build_cfg(const std::vector<Instruction>& program);

void print_cfg(const CFG& cfg);

std::vector<std::set<size_t>> compute_postdom(const CFG& cfg);

std::vector<size_t> compute_ipdom(const CFG& cfg, const std::vector<std::set<size_t>>& pdom);