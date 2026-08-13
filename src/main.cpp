#include "wave.h"
#include "../tests/tests_all.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Running all tests\n";
        test_alu();
        test_branch_basic();
        return 0;
    }
    std::string test = argv[1];
    if (test == "alu") {
        std::cout << "Running ALU test\n";
        test_alu();
    }
    if (test == "branch") {
        std::cout << "Running Branch test \n";
        test_branch_basic();
    }
}
