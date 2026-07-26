# LC3 Virtual Machine

This project is a C implementation of a Virtual Machine for the LC-3 (Little Computer 3) architecture. The LC-3 is a simplified assembly language often used for educational purposes to teach computer architecture and low-level programming concepts.

This virtual machine can execute LC-3 object files (`.obj`), simulating the hardware components of the LC-3 including its registers, memory, opcodes, and trap routines for basic I/O operations.

## Features

*   **Instruction Set Architecture**: Supports all standard LC-3 opcodes (ADD, AND, BR, JMP, JSR, LD, LDI, LDR, LEA, NOT, ST, STI, STR, TRAP).
*   **Memory**: Simulates $2^{16}$ (65,536) memory locations.
*   **Registers**: Implements standard 8 general-purpose registers (R0-R7), Program Counter (PC), and Condition Flags (COND).
*   **Memory-Mapped I/O**: Basic keyboard input support.
*   **TRAP Routines**: Includes standard OS trap vectors for input/output (GETC, OUT, PUTS, IN, PUTSP, HALT).

## Project Structure

*   `src/main.c`: Contains the entire emulator source code, including memory initialization, the instruction cycle (fetch, decode, execute), and simulated hardware logic.

## Getting Started

### Prerequisites

To compile the virtual machine, you will need a C compiler such as GCC or Clang. This emulator uses Unix-specific headers for terminal I/O buffering (`termios.h`, etc.) and is designed to run on Unix-like operating systems (Linux, macOS).

### Compilation

You can compile the project using standard C compilation commands from the root directory:

```bash
gcc -O3 src/main.c -o lc3
```

### Usage

To run the virtual machine, provide one or more compiled LC-3 object files (`.obj` images) as arguments:

```bash
./lc3 [image-file1.obj] ...
```

The VM will load the provided image(s) into its simulated memory starting at the default PC origin (`0x3000`) and begin execution.

To exit the VM manually at any time, use `Ctrl+C`.

## How it Works

The emulator reads the binary image file into its internal memory array, handling endianness differences. It then enters an infinite loop, fetching the instruction at the current Program Counter (PC), decoding the 4-bit opcode, and executing the corresponding simulated hardware behavior using a `switch` statement. It properly updates the condition flags based on operations to support branching.
