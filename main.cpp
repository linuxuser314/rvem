#include <stdio.h>
#include <stdint.h>

constexpr uint32_t MEM_SIZE = 4096; // 4KB of memory

bool execute_instruction(uint32_t* regs, uint8_t* mem, uint32_t* pc) {
    uint32_t instruction = *((uint32_t*)(mem + *pc)); // Fetch instruction (4 bytes)
    printf("Fetched instruction: 0x%08X\n", instruction);

    //Fix these and change them to shift first, then mask (much easier!)
    /*
    uint8_t opcode = (instruction & 0x0000007F) >> 0 ;//Extract opcode (bits 0-6)
    uint8_t funct7 = (instruction & 0xFE000000) >> 27;//Extract funct7 (bits 25-31 R-type only)
    uint8_t funct3 = (instruction & 0x00007000) >> 12;//Extract funct3 (bits 12-14)
    
    printf("Decoded opcode: 0x%02X ", opcode);*/
    uint8_t opcode = (instruction >> 0 ) & 0x0000007F;//Extract bits 0-6
    uint8_t funct7 = (instruction >> 25) & 0x0000007F; //Extract bits 25-31
    uint8_t funct3 = (instruction >> 12) & 0x00000007; //Extract bits 12-14
    uint8_t rs1 =    (instruction >> 15) & 0x0000001F; //Extract bits 15-19
    uint8_t rs2 =    (instruction >> 20) & 0x0000001F; //Extract bits 20-24
    uint8_t rd  =    (instruction >> 7 ) & 0x0000001F; //Extract bits 7-11
    printf("opcode: 0x%02X\n", opcode);
    printf("funct7: 0x%02X\n", funct7);
    printf("funct3: 0x%02X\n", funct3);
    printf("rs1   : 0x%02X, val: %d\n", rs1   , regs[rs1]);
    printf("rs2   : 0x%02X, val: %d\n", rs2   , regs[rs2]);
    printf("rd    : 0x%02X, val: %d\n", rd    , regs[rd ]);
    switch(opcode){
        case(0x33):{ // R-type
            printf("R-type instruction\n");
                 if(funct3 == 0x00 && funct7 == 0x00) regs[rd] = regs[rs1] + regs[rs2];
            else if(funct3 == 0x00 && funct7 == 0x20) regs[rd] = regs[rs1] - regs[rs2];
            else if(funct3 == 0x01 && funct7 == 0x00) regs[rd] = regs[rs1] << (regs[rs2] & 0x0000001F);
            else if(funct3 == 0x02 && funct7 == 0x00) regs[rd] = ((int32_t)regs[rs1]) < ((int32_t)regs[rs2]);
            else if(funct3 == 0x03 && funct7 == 0x00) regs[rd] = regs[rs1] < regs[rs2];
            else if(funct3 == 0x04 && funct7 == 0x00) regs[rd] = regs[rs1] ^ regs[rs2];
            else if(funct3 == 0x05 && funct7 == 0x00) regs[rd] = regs[rs1] >> (regs[rs2] & 0x0000001F);
            else if(funct3 == 0x05 && funct7 == 0x20) regs[rd] = ((int32_t)regs[rs1]) >> (regs[rs2] & 0x0000001F);
            else if(funct3 == 0x06 && funct7 == 0x00) regs[rd] = regs[rs1] | regs[rs2];
            else if(funct3 == 0x07 && funct7 == 0x00) regs[rd] = regs[rs1] & regs[rs2];
            else printf("Undefined R-Type Instructions!!!\n\n");
            break;
        }
        case(0x13):{ // I-type
            printf("I-type instruction\n");
                int32_t seimm = ((int32_t)instruction) >> 20;
                uint32_t uimm  = seimm;
                 if(funct3 == 0x00                  ) regs[rd] = regs[rs1] + seimm;
            else if(funct3 == 0x01 && funct7 == 0x00) regs[rd] = regs[rs1] << (uimm & 0x0000001F);
            else if(funct3 == 0x02                  ) regs[rd] = ((int32_t)regs[rs1]) < seimm;
            else if(funct3 == 0x03                  ) regs[rd] = regs[rs1] < uimm;
            else if(funct3 == 0x04                  ) regs[rd] = regs[rs1] ^ seimm;
            else if(funct3 == 0x05 && funct7 == 0x00) regs[rd] = regs[rs1] >> (uimm & 0x0000001F);
            else if(funct3 == 0x05 && funct7 == 0x20) regs[rd] = ((int32_t)regs[rs1]) >> (uimm & 0x0000001F);
            else if(funct3 == 0x06                  ) regs[rd] = regs[rs1] | seimm;
            else if(funct3 == 0x07                  ) regs[rd] = regs[rs1] & seimm;
            else printf("Undefined I-Type Instructions!!!\n\n");
            break;
        }
        default:
            printf("Unknown instruction opcode!!!");
            return false; // Invalid instruction
    }
    printf("New rd value: %d\n\n", regs[rd]);
    *pc += 4;
    if(*pc >= MEM_SIZE){
        printf("pc (0x%08X) has exceeded memsize (0x%08X)!", *pc, MEM_SIZE);
        return false;
    }
    else return true;

}

int main(){

    printf("Initializing RV32I simulator by Creed Truman...\n");
    uint32_t regs[32] = {0}; // 32 registers initialized to 0
    uint8_t mem[MEM_SIZE] = {0}; // 4KB of memory initialized to 0
    uint32_t pc = 0; // Program counter initialized to 0
    regs[1] = 16;//For arithmetic testing until I have immediate or memory loading commands
    regs[2] = 2;
    FILE* testing_binary = fopen("rv_test.bin", "rb");
    if (testing_binary == NULL) {
        printf("Error: Could not open rv_test.bin!\n");
        return 1; // Exit the simulator if the file fails to load
    }
    size_t bytes_read = fread(mem, sizeof(uint8_t), MEM_SIZE, testing_binary);
    printf("Successfully loaded %zu bytes into memory.\n", bytes_read);

    bool success = true;
    do{
        success = execute_instruction(regs, mem, &pc);
    }while(success);

    printf("Exiting RV32I simulator...\n");
    return 0;
}