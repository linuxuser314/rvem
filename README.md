# rvem

*RISC-V Emulator*

This is my first RISC-V coding project. It supports the base RV32I unprivileged ISA.

This was an educational project to teach me about the details of all the instructions in the base RISC-V architecture, to learn more about the instruction encodings, and to discover any missing hidden traps. I wrote it in C++ so I could gain some experience with RISC-V before implementing it in SystemVerilog.

I have now moved on to the SystemVerilog implementation (`linuxuser314/nano-rv32i`) and have sucesfully ran it, passed RISC-V tests, and synthesized it to an FPGA (Sipeed Tang Nano 20K). As a result, this repository is no longer maintained. If you have any questions though, feel free to contact me.

## Basic Structure
The code follows a simple structure. All code is in `main.cpp`.
It uses the `Machine.executeClockCycle()` method to run `Machine.fetch()`, `Machine.decode()`, and `Machine.execute()`.

## How to use
1. To use, compile `main.cpp` with the compiler of your choice.
2. Place a RV32I unprivileged binary (**not** an ELF file) in your working directory. You can use escape sequences to place the machine code bytes manually or use the RISC-V toolchain. If you have it installed:

``` bash
# Compile assembly to object
riscv32-unknown-elf-as test.s -o test.o
# Link to raw binary (not ELF)
riscv32-unknown-elf-ld test.o -o test.elf
riscv32-unknown-elf-objcopy -O binary test.elf rv_test.bin
```
3. Run the executable. You may pass the optional -debug flag.

**Note:** this program has only been tested on x86-64 systems. Endianness assumptions and byte addressing may cause errors on legacy architectures.