#include <iostream>
#include <cstdlib>
#include <set>
#include "wave.h"
#include "cfg.h"

Wave::Wave(size_t num_regs) : regs(num_regs) {
    if (num_regs > 0) {
        for (size_t i{}; i < WAVE_SIZE; i++) {
            regs[0][i] = i;
        }
    } else {
        std::cout << "Why would you want 0 regs";
        std::abort();
    }
    gmem.resize(GMEM_SIZE);
    for(size_t i{}; i < GMEM_SIZE; i++) {
        gmem[i] = i * 10;
    }
}

void Wave::dump_regs() const {
    for(size_t i{}; i < regs.size(); i++) {
        std::cout << "R" << i << ":";
        for(size_t j{}; j < WAVE_SIZE; j++) {
            std::cout << " " << regs[i][j];
        }
        std::cout << "\n";
    }
    std::cout << " ----------------- " << "\n";
}

void Wave::stats() const {
    int simd_util = (double)total_active_lanes / ideal_active_lanes * 100;
    std::cout << "Average SIMD utilization = " << simd_util << "%\n";

    if (total_mem_transfers != 0) {
        int coalescing = (double)ideal_mem_transfers / total_mem_transfers * 100;
        std::cout << "Average coalescing = " << coalescing << "%\n";
    } else {
        std::cout << "No coalescing to report\n";
    }
}

uint32_t Wave::resolve(const Operand& operand, size_t lane) const {
    if (operand.kind == Operand::Kind::Imm) {
        return operand.value;
    } else if (operand.value < regs.size()){
        return regs[operand.value][lane];
    }
    std::cout << "Register out of bounds";
    std::abort();
}

void Wave::execute(const Instruction& instr) {
    switch (instr.op) {
        case Opcode::MOV_IMM_U32: {
            for (size_t i {}; i < WAVE_SIZE; i++) {
                if (!active_mask[i]) continue; // dont execute masked lanes
                if (instr.guard != NO_GUARD) {
                    bool pred_val = regs[instr.guard][i];
                    if (pred_val == instr.guard_negate) continue; // dont execute if guard val matches
                }
                regs[instr.operands[0].value][i] = resolve(instr.operands[1], i);
            }
            break;
        }
        case Opcode::ADD_U32: {
            for (size_t i {}; i < WAVE_SIZE; i++) {
                if (!active_mask[i]) continue;
                if (instr.guard != NO_GUARD) {
                    bool pred_val = regs[instr.guard][i];
                    if (pred_val == instr.guard_negate) continue;
                }
                uint32_t a = resolve(instr.operands[1], i);
                uint32_t b = resolve(instr.operands[2], i);
                regs[instr.operands[0].value][i] = a + b;
            }
            break;
        }
        case Opcode::SETP_LT_U32: {
            for (size_t i {}; i < WAVE_SIZE; i++) {
                if (!active_mask[i]) continue;
                if (instr.guard != NO_GUARD) {
                    bool pred_val = regs[instr.guard][i];
                    if (pred_val == instr.guard_negate) continue;
                }
                uint32_t a = resolve(instr.operands[1], i);
                uint32_t b = resolve(instr.operands[2], i);
                regs[instr.operands[0].value][i] = (a < b) ? 1 : 0;
            }
            break;
        }
        case Opcode::LW_U32: {
            std::set<size_t> segments;
            for (size_t i{}; i < WAVE_SIZE; i++) {
                if (!active_mask[i]) continue;
                if (instr.guard != NO_GUARD) {
                    bool pred_val = regs[instr.guard][i];
                    if (pred_val == instr.guard_negate) continue;
                }
                uint32_t a = resolve(instr.operands[1], i);
                uint32_t b = resolve(instr.operands[2], i);
                uint32_t addr = a + b;
                segments.insert(addr / GMEM_SEGMENT_SIZE); // for checking coalescing
                regs[instr.operands[0].value][i] = gmem[addr];
            }
            total_mem_transfers += segments.size();
            ideal_mem_transfers += 1;
            break;
        }
        default: break;
        // BRA handled in run, not here
    }
    // count active lanes
    int active_count = 0;
    for (size_t i{}; i < WAVE_SIZE; i++) {
        active_count += active_mask[i] ? 1 : 0;
    }
    total_active_lanes += active_count;
    ideal_active_lanes += WAVE_SIZE;
}

bool check_mask(const std::array<bool, WAVE_SIZE> path) {
    size_t count{};
    for(size_t i{}; i < WAVE_SIZE; i++) {
        count += path[i] ? 1 : 0;
    }
    return static_cast<bool>(count);
}

void Wave::run(const std::vector<Instruction>& program) {
    pc = 0;
    for(size_t i{}; i < WAVE_SIZE;i++) {
        active_mask[i] = true; // set all masks active to start with
    }
    simt_stack.clear();

    CFG cfg = build_cfg(program);
    auto postdom = compute_postdom(cfg);
    auto ipdom = compute_ipdom(cfg, postdom);

    while (pc < program.size()) {
        if (pc == curr_reconv_pc) {
            ReconvEntry top = simt_stack.back();
            simt_stack.pop_back();
            if (top.entry == ReconvEntry::Entry_Type::PATH) {
                active_mask = top.mask;
                pc = top.resume_pc;
                curr_reconv_pc = top.reconv_pc;
            }
            else { // entry type is JOIN
                active_mask = top.mask;
                curr_reconv_pc = simt_stack.empty() ? SIZE_MAX : simt_stack.back().reconv_pc;
            }
            continue;
        }
        const Instruction& instr = program[pc];
        if (instr.op == Opcode::BRANCH) {
            if (!check_mask(active_mask)) {
                pc++;
                continue;
            }
            uint32_t target = instr.operands[0].value;
            //uint32_t reconv_pc = instr.operands[1].value;

            auto curr_block = cfg.pc_to_block[pc];
            auto ipdom_block = ipdom[curr_block];
            auto reconv_pc = cfg.blocks[ipdom_block].start_pc; 

            std::array<bool, WAVE_SIZE> taken_mask {};
            std::array<bool, WAVE_SIZE> fall_mask {};
            for(size_t i {}; i < WAVE_SIZE; i++) {
                if (!active_mask[i]) continue;
                bool pred_status = instr.guard == NO_GUARD ? true : regs[instr.guard][i] != 0;
                if (pred_status) {
                    taken_mask[i] = true;
                } else {
                    fall_mask[i] = true;
                }
            }
            ReconvEntry join, true_path, false_path;
            // reconverge here
            join.mask = active_mask;
            join.reconv_pc = reconv_pc;
            join.entry = ReconvEntry::Entry_Type::JOIN;
            // taken path
            true_path.mask = taken_mask;
            true_path.resume_pc = target;
            true_path.reconv_pc = reconv_pc;
            true_path.entry = ReconvEntry::Entry_Type::PATH;
            // deferred path
            false_path.mask = fall_mask;
            false_path.resume_pc = pc + 1;
            false_path.reconv_pc = reconv_pc;
            false_path.entry = ReconvEntry::Entry_Type::PATH;
            // push all to stack
            simt_stack.push_back(join);
            if (check_mask(false_path.mask)) simt_stack.push_back(false_path);
            if (check_mask(true_path.mask)) simt_stack.push_back(true_path);

            ReconvEntry curr_entry = simt_stack.back();
            simt_stack.pop_back();
            curr_reconv_pc = curr_entry.reconv_pc;
            pc = curr_entry.resume_pc;
            active_mask = curr_entry.mask;
            continue;
        }
        execute(instr);
        pc++;
    }
}
