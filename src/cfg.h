#pragma once
#include <cstdlib>
#include <vector>
#include <iostream>
#include "IR.h"

struct BasicBlock {
    size_t start_pc {};
    size_t end_pc {};
    std::vector<size_t> edges;
};

struct CFG {
    std::vector<BasicBlock> blocks;
    std::vector<size_t> pc_to_block;
};

CFG build_cfg(const std::vector<Instruction>& program);
void print_cfg(const CFG& cfg);