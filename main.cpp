#include <iostream>
#include <vector>
#include <array>
#include <cstdint>

constexpr size_t WAVE_SIZE = 8;
constexpr int num_regs = 4;
using Value = uint32_t;

void dump_regs(const std::vector<std::array<Value, WAVE_SIZE>>& regs) {
    for(int i{}; i < num_regs; i++) {
        std::cout << "r" << i << ":";
        for(size_t j{}; j < WAVE_SIZE; j++) {
            std::cout << " " << regs[i][j];
        }
        std::cout << "\n";
    }
}

int main() {
    std::vector<std::array<Value, WAVE_SIZE>> regs(num_regs);
    for(size_t i{}; i < WAVE_SIZE; i++) {
        regs[0][i] = i;
    }
    dump_regs(regs);
 }
