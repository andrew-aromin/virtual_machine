#include "LC3.hpp"
#include <fstream>
#include <iostream>
#include <utility>

namespace lc3 {

LC3::LC3(IOHandler io_handler)
    : io_handler_(std::move(io_handler)) {
    reset();
}

void LC3::reset() {
    memory_.fill(0);
    reg_.fill(0);
    auto pc_idx = static_cast<std::size_t>(Register::PC);
    auto cond_idx = static_cast<std::size_t>(Register::COND);
    reg_[pc_idx] = DEFAULT_PC_START;
    reg_[cond_idx] = static_cast<std::uint16_t>(ConditionFlag::ZRO);
    running_ = false;
}

std::uint16_t LC3::get_register(Register reg) const {
    return reg_[static_cast<std::size_t>(reg)];
}

void LC3::set_register(Register reg, std::uint16_t value) {
    reg_[static_cast<std::size_t>(reg)] = value;
}

std::uint16_t LC3::read_memory(std::uint16_t address) {
    return mem_read(address);
}

void LC3::write_memory(std::uint16_t address, std::uint16_t value) {
    mem_write(address, value);
}

bool LC3::load_program(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) return false;

    std::uint16_t origin;
    if (!file.read(reinterpret_cast<char*>(&origin), sizeof(origin))) {
        return false;
    }
    origin = swap16(origin);

    std::uint16_t p = origin;
    std::uint16_t instr;
    while (file.read(reinterpret_cast<char*>(&instr), sizeof(instr))) {
        if (p >= MEMORY_SIZE) {
            return false;
        }
        memory_[p++] = swap16(instr);
    }
    return true;
}

void LC3::run() {
    running_ = true;
    auto pc_idx = static_cast<std::size_t>(Register::PC);
    while (running_) {
        std::uint16_t pc_val = reg_[pc_idx];
        reg_[pc_idx]++;
        std::uint16_t instr = mem_read(pc_val);
        execute_instruction(instr);
    }
}

void LC3::execute_instruction(std::uint16_t instr) {
    auto op = static_cast<Opcode>(instr >> 12);
    switch (op) {
        case Opcode::ADD:  execute_add(instr);  break;
        case Opcode::AND:  execute_and(instr);  break;
        case Opcode::NOT:  execute_not(instr);  break;
        case Opcode::BR:   execute_br(instr);   break;
        case Opcode::JMP:  execute_jmp(instr);  break;
        case Opcode::JSR:  execute_jsr(instr);  break;
        case Opcode::LD:   execute_ld(instr);   break;
        case Opcode::LDI:  execute_ldi(instr);  break;
        case Opcode::LDR:  execute_ldr(instr);  break;
        case Opcode::LEA:  execute_lea(instr);  break;
        case Opcode::ST:   execute_st(instr);   break;
        case Opcode::STI:  execute_sti(instr);  break;
        case Opcode::STR:  execute_str(instr);  break;
        case Opcode::TRAP: execute_trap(instr); break;
        case Opcode::RES:
        case Opcode::RTI:
        default:
            running_ = false;
            break;
    }
}

void LC3::execute_add(std::uint16_t instr) {
    std::uint16_t r0 = (instr >> 9) & 0x7;
    std::uint16_t r1 = (instr >> 6) & 0x7;
    std::uint16_t imm_flag = (instr >> 5) & 0x1;

    if (imm_flag) {
        std::uint16_t imm5 = sign_extend(instr & 0x1F, 5);
        reg_[r0] = reg_[r1] + imm5;
    } else {
        std::uint16_t r2 = instr & 0x7;
        reg_[r0] = reg_[r1] + reg_[r2];
    }
    update_flags(r0);
}

void LC3::execute_and(std::uint16_t instr) {
    std::uint16_t r0 = (instr >> 9) & 0x7;
    std::uint16_t r1 = (instr >> 6) & 0x7;
    std::uint16_t imm_flag = (instr >> 5) & 0x1;

    if (imm_flag) {
        std::uint16_t imm5 = sign_extend(instr & 0x1F, 5);
        reg_[r0] = reg_[r1] & imm5;
    } else {
        std::uint16_t r2 = instr & 0x7;
        reg_[r0] = reg_[r1] & reg_[r2];
    }
    update_flags(r0);
}

void LC3::execute_not(std::uint16_t instr) {
    std::uint16_t r0 = (instr >> 9) & 0x7;
    std::uint16_t r1 = (instr >> 6) & 0x7;
    reg_[r0] = ~reg_[r1];
    update_flags(r0);
}

void LC3::execute_br(std::uint16_t instr) {
    std::uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
    std::uint16_t cond_flag = (instr >> 9) & 0x7;
    auto pc_idx = static_cast<std::size_t>(Register::PC);
    auto cond_idx = static_cast<std::size_t>(Register::COND);
    if (cond_flag & reg_[cond_idx]) {
        reg_[pc_idx] += pc_offset;
    }
}

void LC3::execute_jmp(std::uint16_t instr) {
    std::uint16_t base_reg = (instr >> 6) & 0x7;
    auto pc_idx = static_cast<std::size_t>(Register::PC);
    reg_[pc_idx] = reg_[base_reg];
}

void LC3::execute_jsr(std::uint16_t instr) {
    auto r7_idx = static_cast<std::size_t>(Register::R7);
    auto pc_idx = static_cast<std::size_t>(Register::PC);
    reg_[r7_idx] = reg_[pc_idx];

    std::uint16_t long_flag = (instr >> 11) & 1;
    if (long_flag) {
        std::uint16_t pc_offset = sign_extend(instr & 0x7FF, 11);
        reg_[pc_idx] += pc_offset;
    } else {
        std::uint16_t base_reg = (instr >> 6) & 0x7;
        reg_[pc_idx] = reg_[base_reg];
    }
}

void LC3::execute_ld(std::uint16_t instr) {
    std::uint16_t r0 = (instr >> 9) & 0x7;
    std::uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
    auto pc_idx = static_cast<std::size_t>(Register::PC);
    reg_[r0] = mem_read(reg_[pc_idx] + pc_offset);
    update_flags(r0);
}

void LC3::execute_ldi(std::uint16_t instr) {
    std::uint16_t r0 = (instr >> 9) & 0x7;
    std::uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
    auto pc_idx = static_cast<std::size_t>(Register::PC);
    std::uint16_t data_address = mem_read(reg_[pc_idx] + pc_offset);
    reg_[r0] = mem_read(data_address);
    update_flags(r0);
}

void LC3::execute_ldr(std::uint16_t instr) {
    std::uint16_t r0 = (instr >> 9) & 0x7;
    std::uint16_t base_reg = (instr >> 6) & 0x7;
    std::uint16_t offset = sign_extend(instr & 0x3F, 6);
    reg_[r0] = mem_read(reg_[base_reg] + offset);
    update_flags(r0);
}

void LC3::execute_lea(std::uint16_t instr) {
    std::uint16_t r0 = (instr >> 9) & 0x7;
    std::uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
    auto pc_idx = static_cast<std::size_t>(Register::PC);
    reg_[r0] = reg_[pc_idx] + pc_offset;
    update_flags(r0);
}

void LC3::execute_st(std::uint16_t instr) {
    std::uint16_t r0 = (instr >> 9) & 0x7;
    std::uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
    auto pc_idx = static_cast<std::size_t>(Register::PC);
    mem_write(reg_[pc_idx] + pc_offset, reg_[r0]);
}

void LC3::execute_sti(std::uint16_t instr) {
    std::uint16_t r0 = (instr >> 9) & 0x7;
    std::uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
    auto pc_idx = static_cast<std::size_t>(Register::PC);
    std::uint16_t address_pointer = mem_read(reg_[pc_idx] + pc_offset);
    mem_write(address_pointer, reg_[r0]);
}

void LC3::execute_str(std::uint16_t instr) {
    std::uint16_t r0 = (instr >> 9) & 0x7;
    std::uint16_t base_reg = (instr >> 6) & 0x7;
    std::uint16_t offset = sign_extend(instr & 0x3F, 6);
    mem_write(reg_[base_reg] + offset, reg_[r0]);
}

void LC3::execute_trap(std::uint16_t instr) {
    auto r7_idx = static_cast<std::size_t>(Register::R7);
    auto pc_idx = static_cast<std::size_t>(Register::PC);
    reg_[r7_idx] = reg_[pc_idx];

    auto trap_vector = static_cast<TrapVector>(instr & 0xFF);
    switch (trap_vector) {
        case TrapVector::GETC: {
            if (io_handler_.read_char) {
                reg_[0] = static_cast<std::uint16_t>(io_handler_.read_char());
                update_flags(0);
            }
            break;
        }
        case TrapVector::OUT: {
            if (io_handler_.write_char) {
                io_handler_.write_char(static_cast<char>(reg_[0]));
            }
            break;
        }
        case TrapVector::PUTS: {
            std::uint16_t address = reg_[0];
            std::uint16_t c = mem_read(address);
            while (c != 0x0000) {
                if (io_handler_.write_char) {
                    io_handler_.write_char(static_cast<char>(c));
                }
                c = mem_read(++address);
            }
            break;
        }
        case TrapVector::IN: {
            if (io_handler_.write_char) {
                std::string prompt = "Enter character: ";
                for (char ch : prompt) io_handler_.write_char(ch);
            }
            if (io_handler_.read_char) {
                char c = io_handler_.read_char();
                if (io_handler_.write_char) {
                    io_handler_.write_char(c);
                }
                reg_[0] = static_cast<std::uint16_t>(c);
                update_flags(0);
            }
            break;
        }
        case TrapVector::PUTSP: {
            std::uint16_t address = reg_[0];
            std::uint16_t c = mem_read(address);
            while (c != 0x0000) {
                char char1 = static_cast<char>(c & 0xFF);
                if (io_handler_.write_char) io_handler_.write_char(char1);
                char char2 = static_cast<char>(c >> 8);
                if (char2 && io_handler_.write_char) {
                    io_handler_.write_char(char2);
                }
                c = mem_read(++address);
            }
            break;
        }
        case TrapVector::HALT: {
            if (io_handler_.write_char) {
                std::string msg = "HALT\n";
                for (char ch : msg) io_handler_.write_char(ch);
            }
            running_ = false;
            break;
        }
        default:
            running_ = false;
            break;
    }
}

std::uint16_t LC3::sign_extend(std::uint16_t x, int bit_count) {
    if ((x >> (bit_count - 1)) & 1) {
        x |= (0xFFFF << bit_count);
    }
    return x;
}

std::uint16_t LC3::swap16(std::uint16_t x) {
    return static_cast<std::uint16_t>((x << 8) | (x >> 8));
}

void LC3::update_flags(std::uint16_t reg_idx) {
    auto cond_idx = static_cast<std::size_t>(Register::COND);
    if (reg_[reg_idx] == 0) {
        reg_[cond_idx] = static_cast<std::uint16_t>(ConditionFlag::ZRO);
    } else if (reg_[reg_idx] >> 15) {
        reg_[cond_idx] = static_cast<std::uint16_t>(ConditionFlag::NEG);
    } else {
        reg_[cond_idx] = static_cast<std::uint16_t>(ConditionFlag::POS);
    }
}

std::uint16_t LC3::mem_read(std::uint16_t address) {
    if (address == MR_KBSR) {
        if (io_handler_.check_key && io_handler_.check_key()) {
            memory_[MR_KBSR] = (1 << 15);
            if (io_handler_.read_char) {
                memory_[MR_KBDR] = static_cast<std::uint16_t>(io_handler_.read_char());
            }
        } else {
            memory_[MR_KBSR] = 0;
        }
    }
    return memory_[address];
}

void LC3::mem_write(std::uint16_t address, std::uint16_t val) {
    memory_[address] = val;
}

} // namespace lc3
