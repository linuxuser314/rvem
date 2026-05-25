#include <stdio.h>
#include <stdint.h>

constexpr uint32_t MEM_SIZE = 4096; // 4KB of memory
constexpr uint32_t MEMORY_MAPPED_OUTPUT = 4095;
bool execute_instruction(uint32_t* regs, uint8_t* mem, uint32_t* pc) {
    uint32_t instruction = *((uint32_t*)(mem + *pc)); // Fetch instruction (4 bytes)
    printf("Fetched instruction: 0x%08X\n", instruction);
    
    int32_t pc_increment = 4;

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
        case(0x33):{ // R-type ALU math
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
        case(0x13):{ // I-type ALU math
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
        case(0x03):{ // I-type loads
            uint32_t address = regs[rs1] + ((int32_t)instruction >> 20);
            uint8_t loadSize = 1 << (funct3 & 0x03);;
            if(address + loadSize - 1 >= MEM_SIZE){
                printf("ERROR! address exceeds memory size. ");
                return 0;
            }
            switch(funct3){
                case(0x00):{
                    regs[rd] = (int8_t)mem[address];
                    break;
                }
                case(0x01):{
                    regs[rd] = (int16_t)((mem[address]) | ((uint16_t)mem[address + 1] << 8));
                    break;
                }
                case(0x02):{
                    regs[rd] = (mem[address]) | ((uint16_t)mem[address + 1] << 8) | ((uint32_t)mem[address + 2] << 16) | ((uint32_t)mem[address + 3] << 24);
                    break;
                }
                case(0x04):{
                    regs[rd] = mem[address];
                    break;
                }
                case(0x05):{
                    regs[rd] = (mem[address]) | ((uint16_t)mem[address + 1] << 8);
                    break;
                }
                default:{
                    printf("ERROR! Undefined load instruction (funct3 = %02X)", funct3);
                    break;
                }
           
            }
         break;
        }
        case(0x23):{ // S-type stores
            printf("Store instruction!");
            int32_t imm = rd | funct7 << 5;
            if(imm & 0x00000800){
                imm |= 0xFFFFF000;//Manual sign extension (better than typecasting!)
            }
            uint32_t address = regs[rs1] + imm;
            uint8_t loadSize = 1 << (funct3 & 0x03);
            if(address + loadSize - 1 >= MEM_SIZE){
                printf("ERROR! address exceeds memory size. ");
                return 0;
            }

            switch(funct3){
                case(0x00):{
                    mem[address] = regs[rs2] & 0xFF;
                    break;
                }
                case(0x01):{
                    mem[address    ] = (regs[rs2] >> 0) & 0xFF;
                    mem[address + 1] = (regs[rs2] >> 8) & 0xFF;
                    break;
                }
                case(0x02):{
                    mem[address    ] = (regs[rs2] >> 0 ) & 0xFF;
                    mem[address + 1] = (regs[rs2] >> 8 ) & 0xFF;
                    mem[address + 2] = (regs[rs2] >> 16) & 0xFF;
                    mem[address + 3] = (regs[rs2] >> 24) & 0xFF;
                    break;
                }
                default:{
                    printf("ERROR! Undefined store instruction (funct3 = %02X)", funct3);
                    break;
                }
            }
            break;
        }
        case(0x63):{ // B-type branches
            bool isBranching = false;
            isBranching |= funct3 == 0x0 && regs[rs1] == regs[rs2];
            isBranching |= funct3 == 0x1 && regs[rs1] != regs[rs2];
            isBranching |= funct3 == 0x4 && (int32_t)regs[rs1] <  (int32_t)regs[rs2];
            isBranching |= funct3 == 0x5 && (int32_t)regs[rs1] >= (int32_t)regs[rs2];
            isBranching |= funct3 == 0x6 && regs[rs1] <  regs[rs2];
            isBranching |= funct3 == 0x7 && regs[rs1] >= regs[rs2];
            if(isBranching){
                int32_t imm = 0;
                imm |= 0;//Hardwired 0 bit
                imm |= ((instruction >> 8 ) & 0x0F) << 1; //Bits 1-4
                imm |= ((instruction >> 25) & 0x3F) << 5 ;//Bits 5:10
                imm |= ((instruction >> 7 ) & 0x01) << 11;//Bit 11
                imm |= ((instruction >> 31 ) & 0x01) << 12;//Bit 12
                if(imm & 0x00001000){
                    imm |= 0xFFFFF000;
                }
                pc_increment = imm;
            }
            break;

        }
        case(0x37):{ // U-type LUI instruction
            regs[rd] = instruction & 0xFFFFF000;
            break;
        }
        case(0x17):{ // U-type AUIPC instruction
            regs[rd] = (instruction & 0xFFFFF000) + *pc;
            break;
        }
        case(0x67):{ // I-type jalr instruction
            regs[rd] = *pc + 4;
            *pc = regs[rs1] + (((int32_t)instruction) >> 20) & ~0x01;
            pc_increment = 0;
            break;
        }
        case(0x6F):{ //J-type jal instruction
            regs[rd] = *pc + 4;
            
            int32_t imm = 0;
            imm |= 0;//Hardwired 0 bit

            imm |= ((instruction >> 21) & 0x03FF) << 1 ;//Extract bits 1-10
            imm |= ((instruction >> 20) & 0x0001) << 11;//Extrat bit 11.
            imm |= ((instruction >> 12) & 0x00FF) << 12;//Extract bits 12-19
            imm |= ((instruction >> 31) & 0x0001) << 20;//Extract bit 20

            
            //Sign extension
            if(imm & 0x00100000){
                imm |= 0xFFF00000;
            }
            pc_increment = imm;
            break;
        }
        default:{
            printf("Unknown instruction opcode!!!");
            return false; // Invalid instruction
        }
    }
    regs[0] = 0;
    printf("New rd value: %d\n\n", regs[rd]);
    *pc += pc_increment;

    if(mem[MEMORY_MAPPED_OUTPUT] != 0){
        printf("\n\n Memory Mapped Output Triggered: ");
        putchar(mem[MEMORY_MAPPED_OUTPUT]);
        return false;
    }
    //Technically buggy on multi-byte fetches, but OK for now.
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