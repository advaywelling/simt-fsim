#pragma once
#include <array>
#include <cstdint>
#include <iostream>
#include <string>
#include "../src/wave.h"

// tests keep their void signature and report into this tally
inline int checks_run = 0;
inline int checks_failed = 0;

inline void fail_at(const char* file, int line, const std::string& what) {
    checks_failed++;
    std::cout << "  FAIL " << file << ":" << line << "  " << what << "\n";
}

inline void check_impl(const char* file, int line, bool ok, const char* expr) {
    checks_run++;
    if (ok) return;
    fail_at(file, line, expr);
}

inline void expect_reg_impl(const char* file, int line, const Wave& wave, size_t r,
                            const std::array<uint32_t, WAVE_SIZE>& want) {
    checks_run++;
    const RegFile& regs = wave.registers();
    if (r >= regs.size()) {
        fail_at(file, line, "R" + std::to_string(r) + " does not exist");
        return;
    }
    if (regs[r] == want) return;

    fail_at(file, line, "R" + std::to_string(r));
    std::cout << "    want:";
    for (uint32_t v : want) std::cout << " " << v;
    std::cout << "\n    got: ";
    for (uint32_t v : regs[r]) std::cout << " " << v;
    std::cout << "\n";
}

// variadic so commas inside the condition (e.g. vector<size_t>{2, 1}) don't split the macro arg
#define CHECK(...) check_impl(__FILE__, __LINE__, (__VA_ARGS__), #__VA_ARGS__)

// EXPECT_REG(wave, 3, 0, 0, 0, 0, 5, 5, 5, 5) -> one expected value per lane
#define EXPECT_REG(wave, r, ...) \
    expect_reg_impl(__FILE__, __LINE__, (wave), (r), std::array<uint32_t, WAVE_SIZE>{__VA_ARGS__})

// returns the process exit code: 0 if everything passed
inline int tests_summary() {
    std::cout << "\n" << (checks_run - checks_failed) << "/" << checks_run << " checks passed\n";
    if (checks_failed) std::cout << checks_failed << " CHECK(S) FAILED\n";
    return checks_failed ? 1 : 0;
}
