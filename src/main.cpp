#include "wave.h"
#include "cfg.h"
#include "../tests/tests_all.h"

int main(int argc, char* argv[]) {
    std::string test = (argc < 2) ? "all" : argv[1];
    bool ran = false;

    if (test == "all" || test == "alu") {
        std::cout << "Running ALU test\n";
        test_alu();
        std::cout << "\n";
        ran = true;
    }
    if (test == "all" || test == "branch") {
        std::cout << "Running Branch test\n";
        test_branch_basic();
        std::cout << "\n";
        test_branch_nested();
        std::cout << "\n";
        ran = true;
    }
    if (test == "all" || test == "lw") {
        std::cout << "Running LW test\n";
        test_lw();
        std::cout << "\n";
        ran = true;
    }
    if (test == "all" || test == "cfg") {
        std::cout << "Running CFG test\n";
        test_cfg();
        std::cout << "\n";
        ran = true;
    }

    if (!ran) {
        std::cout << "Unknown test '" << test << "'. Options: alu, branch, lw, cfg\n";
        return 2;
    }
    return tests_summary();
}
