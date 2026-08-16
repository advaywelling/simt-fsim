#include "ISA.h"

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
    print_cfg(cfg); // also runs pdom and ipdom functions
}