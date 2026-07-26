#ifndef LC3_HPP
#define LC3_HPP

#include <array>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>

// Function declaration required by mem_read
bool check_key();

class LC3 {
private:
    std::array<u_int16_t, 65536> memory;

    enum Register { R0=0, R1, R2, R3, R4, R5, R6, R7, PC, COND, COUNT };

    std::array<u_int16_t, Register::COUNT> reg;

    enum ConditionFlag { POS = 1 << 0, ZRO = 1 << 1, NEG = 1 << 2 };

    enum Opcodes {
        OP_BR = 0, OP_ADD, OP_LD, OP_ST, OP_JSR, OP_AND, OP_LDR, OP_STR,
        OP_RTI, OP_NOT, OP_LDI, OP_STI, OP_JMP, OP_RES, OP_LEA, OP_TRAP
    };

    enum MemoryMappedReg {
        MR_KBSR = 0xFE00,
        MR_KBDR = 0xFE02
    };

    enum TrapVector {
        TRAP_GETC = 0x20,
        TRAP_OUT = 0x21,
        TRAP_PUTS = 0x22,
        TRAP_IN = 0x23,
        TRAP_PUTSP = 0x24,
        TRAP_HALT = 0x25
    };

    u_int16_t sign_extend(u_int16_t x, int bit_count) {
        if ((x >> (bit_count - 1)) & 1) {
            x |= (0xFFFF << bit_count);
        }
        return x;
    }

    void update_flags(u_int16_t r) {
        if (reg[r] == 0) {
            reg[COND] = ZRO;
        } else if (reg[r] >> 15) {
            reg[COND] = NEG;
        } else {
            reg[COND] = POS;
        }
    }

    uint16_t swap16(uint16_t x) {
        return (x << 8) | (x >> 8);
    }

    uint16_t mem_read(uint16_t address) {
        if (address == MR_KBSR) {
            if (check_key()) {
                memory[MR_KBSR] = (1 << 15);
                memory[MR_KBDR] = getchar();
            } else {
                memory[MR_KBSR] = 0;
            }
        }
        return memory[address];
    }

    void mem_write(uint16_t address, uint16_t val) {
        memory[address] = val;
    }

public:
    LC3() {
        memory.fill(0);
        reg.fill(0);
        reg[PC] = 0x3000;
    }

    void run() {
        bool running = true;
        while (running) {
            u_int16_t instr = memory[reg[PC]];
            reg[PC]++;
            u_int16_t op = instr >> 12;
            switch(op) {
                case OP_ADD: {
                    uint16_t r0 = (instr >> 9) & 0x7;
                    uint16_t r1 = (instr >> 6) & 0x7;
                    uint16_t imm_flag = (instr >> 5) & 0x1;
                    
                    if (imm_flag) {
                        uint16_t imm5 = sign_extend(instr & 0x1F, 5);
                        reg[r0] = reg[r1] + imm5;
                    } else {
                        uint16_t r2 = instr & 0x7;
                        reg[r0] = reg[r1] + reg[r2];
                    }
                    
                    update_flags(r0);
                    break;
                }
                case OP_LDI: {
                    uint16_t r0 = (instr >> 9) & 0x7;
                    uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
                    uint16_t data_address = mem_read(reg[PC] + pc_offset);
                    reg[r0] = mem_read(data_address);
                    update_flags(r0);
                    break;
                }
                case OP_BR: {
                    uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
                    uint16_t cond_flag = (instr >> 9) & 0x7;
                    if (cond_flag & reg[COND]) {
                        reg[PC] += pc_offset;
                    }
                    break;
                }
                case OP_AND: {
                    uint16_t r0 = (instr >> 9) & 0x7;
                    uint16_t r1 = (instr >> 6) & 0x7;
                    uint16_t imm_flag = (instr >> 5) & 0x1;
                    
                    if (imm_flag) {
                        uint16_t imm5 = sign_extend(instr & 0x1F, 5);
                        reg[r0] = reg[r1] & imm5;
                    } else {
                        uint16_t r2 = instr & 0x7;
                        reg[r0] = reg[r1] & reg[r2];
                    }
                    
                    update_flags(r0);
                    break;
                }
                case OP_NOT: {
                    uint16_t r0 = (instr >> 9) & 0x7;
                    uint16_t r1 = (instr >> 6) & 0x7;
                    reg[r0] = ~reg[r1];
                    update_flags(r0);
                    break;
                }
                case OP_LD: {
                    uint16_t r0 = (instr >> 9) & 0x7;
                    uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
                    reg[r0] = mem_read(reg[PC] + pc_offset);
                    update_flags(r0);
                    break;
                }
                case OP_ST: {
                    uint16_t r0 = (instr >> 9) & 0x7;
                    uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
                    mem_write(reg[PC] + pc_offset, reg[r0]);
                    break;
                }
                case OP_LDR: {
                    uint16_t r0 = (instr >> 9) & 0x7;
                    uint16_t base_reg = (instr >> 6) & 0x7;
                    uint16_t offset = sign_extend(instr & 0x3F, 6);
                    reg[r0] = mem_read(reg[base_reg] + offset);
                    update_flags(r0);
                    break;
                }
                case OP_STR: {
                    uint16_t r0 = (instr >> 9) & 0x7;
                    uint16_t base_reg = (instr >> 6) & 0x7;
                    uint16_t offset = sign_extend(instr & 0x3F, 6);
                    mem_write(reg[base_reg] + offset, reg[r0]);
                    break;
                }
                case OP_LEA: {
                    uint16_t r0 = (instr >> 9) & 0x7;
                    uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
                    reg[r0] = reg[PC] + pc_offset;
                    update_flags(r0);
                    break;
                }
                case OP_STI: {
                    uint16_t r0 = (instr >> 9) & 0x7;
                    uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
                    uint16_t address_pointer = mem_read(reg[PC] + pc_offset);
                    mem_write(address_pointer, reg[r0]);
                    break;
                }
                case OP_JMP: {
                    uint16_t base_reg = (instr >> 6) & 0x7;
                    reg[PC] = reg[base_reg];
                    break;
                }
                case OP_JSR: {
                    reg[R7] = reg[PC];
                    uint16_t long_flag = (instr >> 11) & 1;
                    if (long_flag) {
                        uint16_t pc_offset = sign_extend(instr & 0x7FF, 11);
                        reg[PC] += pc_offset; 
                    } else {
                        uint16_t base_reg = (instr >> 6) & 0x7;
                        reg[PC] = reg[base_reg];
                    }
                    break;
                }
                case OP_TRAP: {
                    reg[R7] = reg[PC];
                    uint16_t trap_vector = instr & 0xFF;
                    switch (trap_vector) {
                        case TRAP_PUTS: {
                            uint16_t address = reg[R0];
                            uint16_t c = mem_read(address);
                            while (c != 0x0000) {
                                std::cout << (char)c;
                                c = mem_read(++address);
                            }
                            std::cout << std::flush;
                            break;
                        }
                        case TRAP_HALT: {
                            std::cout << "HALT" << std::endl;
                            running = false;
                            break;
                        }
                        case TRAP_GETC: {
                            reg[R0] = (uint16_t)getchar();
                            update_flags(R0);
                            break;
                        }
                        case TRAP_OUT: {
                            std::cout << (char)reg[R0] << std::flush;
                            break;
                        }
                        case TRAP_IN: {
                            std::cout << "Enter a character: ";
                            char c = getchar();
                            std::cout << c << std::flush;
                            reg[R0] = (uint16_t)c;
                            update_flags(R0);
                            break;
                        }
                        case TRAP_PUTSP: {
                            uint16_t address = reg[R0];
                            uint16_t c = mem_read(address);
                            while (c != 0x0000) {
                                char char1 = c & 0xFF;
                                std::cout << char1;
                                char char2 = c >> 8;
                                if (char2) std::cout << char2;
                                c = mem_read(++address);
                            }
                            std::cout << std::flush;
                            break;
                        }
                    }
                    break;
                }
                default: {
                    running = false;
                    break;
                }
            }
        }
    }

    bool load_program(const std::string& filename) {
        std::ifstream file(filename, std::ios::binary);
        if (!file) return false;

        uint16_t origin;
        if (!file.read(reinterpret_cast<char*>(&origin), sizeof(origin))) {
            return false;
        }
        origin = swap16(origin);

        uint16_t p = origin;
        uint16_t instr;
        while (file.read(reinterpret_cast<char*>(&instr), sizeof(instr))) {
            memory[p++] = swap16(instr);
        }
        return true;
    }
};

#endif