#include <stdio.h>
#include <stdint.h>

constexpr uint32_t MEM_SIZE = 4096; // 4KB of memory

bool execute_instruction(uint32_t* regs, uint8_t* mem, uint32_t* pc) {
    uint32_t instruction = *((uint32_t*)(mem + *pc)); // Fetch instruction (4 bytes)
    printf("Fetched instruction: 0x%08X\n", instruction);
    *pc += 4;
    if(*pc >= 40) return false;
    else return true;

}

int main()
{
    printf("Initializing RV32I simulator by Creed Truman...\n");
    uint32_t regs[32] = {0}; // 32 registers initialized to 0
    uint8_t mem[MEM_SIZE] = {0}; // 4KB of memory initialized to 0
    uint32_t pc = 0; // Program counter initialized to 0

    //Putting some test data in memory...
    for(int i = 0; i < 10; i++){
        mem[i * 4] = i;
    }
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