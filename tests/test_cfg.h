#pragma once
#include "ISA.h"
#include "check.h"

inline void test_cfg() {
    std::cout << "CFG Test\n";
    std::vector<Instruction> program = {
        MOV(1, 4),         // 0
        SETP_LT(2, 0, 1),  // 1
        BRANCH(5, 6, 2),   // 2: cond branch, target=5, guard=R2
        MOV(3, 100),       // 3: fall-through path
        BRANCH(6, 6),      // 4: unconditional to 6
        MOV(4, 200),       // 5: taken path
        MOV(5, 99),        // 6: reconverge
    };
    CFG cfg = build_cfg(program);
    print_cfg(cfg);

    // leaders are 0 (entry), 3 and 5 (branch fall-through / target), 6 (branch target)
    CHECK(cfg.blocks.size() == 4);
    if (cfg.blocks.size() != 4) return; // the checks below would index off the end

    CHECK(cfg.blocks[0].start_pc == 0 && cfg.blocks[0].end_pc == 2);
    CHECK(cfg.blocks[1].start_pc == 3 && cfg.blocks[1].end_pc == 4);
    CHECK(cfg.blocks[2].start_pc == 5 && cfg.blocks[2].end_pc == 5);
    CHECK(cfg.blocks[3].start_pc == 6 && cfg.blocks[3].end_pc == 6);

    CHECK(cfg.blocks[0].edges == std::vector<size_t>{2, 1}); // cond branch: taken then fall-through
    CHECK(cfg.blocks[1].edges == std::vector<size_t>{3});    // uncond branch to BB3
    CHECK(cfg.blocks[2].edges == std::vector<size_t>{3});    // falls through into BB3
    CHECK(cfg.blocks[3].edges.empty());                      // exit

    std::vector<std::set<size_t>> pdom = compute_postdom(cfg);
    CHECK(pdom[0] == std::set<size_t>{0, 3});
    CHECK(pdom[1] == std::set<size_t>{1, 3});
    CHECK(pdom[2] == std::set<size_t>{2, 3});
    CHECK(pdom[3] == std::set<size_t>{3});

    // both arms of the branch reconverge at BB3
    std::vector<size_t> ipdom = compute_ipdom(cfg, pdom);
    CHECK(ipdom[0] == 3);
    CHECK(ipdom[1] == 3);
    CHECK(ipdom[2] == 3);
    CHECK(ipdom[3] == SIZE_MAX); // exit block has no post-dominator but itself
}
