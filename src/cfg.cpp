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
    for (int i{}; i < n; i++) {
        if (program[i].op == Opcode::BRANCH) {
            size_t target = program[i].operands[0].value; 
            if (target < n) is_leader[i] = true; // rule 2
            if (i + 1 < n) is_leader[i + 1] = true; // rule 3
        }
    }
}