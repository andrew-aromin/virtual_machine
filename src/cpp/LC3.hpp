#ifndef LC3_HPP
#define LC3_HPP

#include <array>
#include <cstdint>
#include <functional>
#include <string>

namespace lc3 {

// Strongly typed register indices
enum class Register : std::uint8_t {
    R0 = 0, R1, R2, R3, R4, R5, R6, R7,
    PC,
    COND,
    COUNT
};

// Condition flags
enum class ConditionFlag : std::uint16_t {
    POS = 1 << 0,
    ZRO = 1 << 1,
    NEG = 1 << 2
};

// Opcodes
enum class Opcode : std::uint8_t {
    BR = 0, ADD, LD, ST, JSR, AND, LDR, STR,
    RTI, NOT, LDI, STI, JMP, RES, LEA, TRAP
};

// Memory-mapped registers
enum MemoryMappedReg : std::uint16_t {
    MR_KBSR = 0xFE00, // Keyboard status
    MR_KBDR = 0xFE02  // Keyboard data
};

// Trap vectors
enum class TrapVector : std::uint8_t {
    GETC = 0x20,
    OUT = 0x21,
    PUTS = 0x22,
    IN = 0x23,
    PUTSP = 0x24,
    HALT = 0x25
};

// I/O Interface callbacks for decoupling
struct IOHandler {
    std::function<bool()> check_key;
    std::function<char()> read_char;
    std::function<void(char)> write_char;
};

class LC3 {
public:
    static constexpr std::size_t MEMORY_SIZE = 65536;
    static constexpr std::uint16_t DEFAULT_PC_START = 0x3000;

    explicit LC3(IOHandler io_handler = {});

    LC3(const LC3&) = delete;
    LC3& operator=(const LC3&) = delete;
    LC3(LC3&&) noexcept = default;
    LC3& operator=(LC3&&) noexcept = default;

    // Load binary object file into memory
    bool load_program(const std::string& filename);

    // Run the execution loop
    void run();

    // Reset VM state (registers and memory)
    void reset();

    // Register & memory state accessors
    std::uint16_t get_register(Register reg) const;
    void set_register(Register reg, std::uint16_t value);
    std::uint16_t read_memory(std::uint16_t address);
    void write_memory(std::uint16_t address, std::uint16_t value);

    bool is_running() const { return running_; }
    void stop() { running_ = false; }

private:
    std::array<std::uint16_t, MEMORY_SIZE> memory_{};
    std::array<std::uint16_t, static_cast<std::size_t>(Register::COUNT)> reg_{};
    bool running_{false};
    IOHandler io_handler_;

    // Core Instruction Execution Helpers
    void execute_instruction(std::uint16_t instr);
    void execute_add(std::uint16_t instr);
    void execute_and(std::uint16_t instr);
    void execute_not(std::uint16_t instr);
    void execute_br(std::uint16_t instr);
    void execute_jmp(std::uint16_t instr);
    void execute_jsr(std::uint16_t instr);
    void execute_ld(std::uint16_t instr);
    void execute_ldi(std::uint16_t instr);
    void execute_ldr(std::uint16_t instr);
    void execute_lea(std::uint16_t instr);
    void execute_st(std::uint16_t instr);
    void execute_sti(std::uint16_t instr);
    void execute_str(std::uint16_t instr);
    void execute_trap(std::uint16_t instr);

    // Internal utilities
    static std::uint16_t sign_extend(std::uint16_t x, int bit_count);
    static std::uint16_t swap16(std::uint16_t x);
    void update_flags(std::uint16_t reg_idx);
    std::uint16_t mem_read(std::uint16_t address);
    void mem_write(std::uint16_t address, std::uint16_t val);
};

} // namespace lc3

#endif // LC3_HPP