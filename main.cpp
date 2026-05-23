#include <stdio.h>
#include <stdint.h>

constexpr uint32_t MEM_SIZE = 4096; // 4KB of memory

bool execute_instruction(uint32_t* regs, uint8_t* mem, uint32_t* pc) {
    uint32_t instruction = *((uint32_t*)(mem + *pc)); // Fetch instruction (4 bytes)
    printf("Fetched instruction: 0x%08X\n", instruction);

    //Fix these and change them to shift first, then mask (much easier!)
    uint8_t opcode = (instruction & 0x0000007F) >> 0 ;//Extract opcode (bits 0-6)
    uint8_t funct7 = (instruction & 0xFE000000) >> 27;//Extract funct7 (bits 25-31 R-type only)
    uint8_t funct3 = (instruction & 0x00007000) >> 12;//Extract funct3 (bits 12-14)
    printf("Decoded opcode: 0x%02X ", opcode);
    switch(opcode){
        case 0x33: // R-type
            printf("R-type instruction\n");
            break;
        default:
            printf("Unknown instruction opcode!!!");
            return false; // Invalid instruction
    }

    *pc += 4;
    if(*pc >= MEM_SIZE){
        printf("pc (0x%08X) has exceeded memsize (0x%08X)!", *pc, MEM_SIZE);
        return false;
    }
    else return true;

}

int main()
{
    printf("Initializing RV32I simulator by Creed Truman...\n");
    uint32_t regs[32] = {0}; // 32 registers initialized to 0
    uint8_t mem[MEM_SIZE] = {0}; // 4KB of memory initialized to 0
    uint32_t pc = 0; // Program counter initialized to 0

    while(true){
        bool success = execute_instruction(regs, mem, &pc);
        if(success){
        }
        else{
            printf("Error!\n");
            break;
        }
    }
    printf("Exiting RV32I simulator...\n");
    return 0;
}