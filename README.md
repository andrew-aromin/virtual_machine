# LC3 Virtual Machine

This project is a C and C++ implementation of a Virtual Machine for the LC-3 (Little Computer 3) architecture. The LC-3 is a simplified assembly language often used for educational purposes to teach computer architecture and low-level programming concepts.

This virtual machine can execute LC-3 object files (`.obj`), simulating the hardware components of the LC-3 including its registers, memory, opcodes, and trap routines for basic I/O operations.

## Features

*   **Instruction Set Architecture**: Supports all standard LC-3 opcodes (ADD, AND, BR, JMP, JSR, LD, LDI, LDR, LEA, NOT, ST, STI, STR, TRAP).
*   **Memory**: Simulates $2^{16}$ (65,536) memory locations.
*   **Registers**: Implements standard 8 general-purpose registers (R0-R7), Program Counter (PC), and Condition Flags (COND).
*   **Memory-Mapped I/O**: Basic keyboard input support.
*   **TRAP Routines**: Includes standard OS trap vectors for input/output (GETC, OUT, PUTS, IN, PUTSP, HALT).
*   **Dual Implementation**: Features both a procedural C implementation and an object-oriented C++ implementation.

## Project Structure

*   `src/main.c`: Procedural C implementation of the emulator, containing memory initialization, instruction cycle, and hardware logic.
*   `src/cpp/`: Object-oriented, decoupled C++ implementation.
    *   `src/cpp/LC3.hpp` & `src/cpp/LC3.cpp`: Decoupled `LC3` class encapsulating virtual machine state, memory management, modular opcode dispatch, and trap routines. Supports dependency-injected I/O handlers.
    *   `src/cpp/Terminal.hpp` & `src/cpp/Terminal.cpp`: RAII terminal controller handling Unix terminal raw mode, signals (`SIGINT`), and non-blocking I/O.
    *   `src/cpp/main.cpp`: C++ driver initializing the RAII terminal guard and executing program images on the `LC3` VM.

## Getting Started

### Prerequisites

To compile the virtual machine, you will need a C/C++ compiler such as GCC (`gcc` / `g++`) or Clang (`clang` / `clang++`). This emulator uses Unix-specific headers for terminal I/O buffering (`termios.h`, etc.) and is designed to run on Unix-like operating systems (Linux, macOS).

### Compilation

You can compile either implementation from the root directory:

**C Implementation:**
```bash
gcc -O3 src/main.c -o lc3
```

**C++ Implementation:**
```bash
g++ -O3 -std=c++17 src/cpp/*.cpp -o lc3_cpp
```

### Usage

To run the virtual machine, provide a compiled LC-3 object file (`.obj` image) as an argument:

**C Version:**
```bash
./lc3 [image-file1.obj] ...
```

**C++ Version:**
```bash
./lc3_cpp [image-file1.obj]
```

The VM loads the provided image file into simulated memory starting at the default PC origin (`0x3000`) and begins execution.

To exit the VM manually at any time, press `Ctrl+C`.

## How it Works

The emulator reads binary image files into its internal memory array, converting big-endian LC-3 words to host byte order. It then enters an execution loop: fetching the instruction at the Program Counter (PC), decoding the 4-bit opcode, and performing the corresponding operations. Condition flags (POS, ZRO, NEG) are updated on register writes to enable conditional branching. The C++ version encapsulates all hardware logic and VM state cleanly inside the `LC3` class.
