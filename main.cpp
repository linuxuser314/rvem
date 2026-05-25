/*
*TODO:
*Add overflow-checking logic to importBinary
*Clean up executeClockCycle and make it more hardware-accurate
*Eliminate magic numbers in opcodes and such
*Make it auto-import a binary with a filename that can be passed in via arguments, not that is hardcoded to rv_test.bin
*
*
*Add a header
*Add a readme
*Add a src, include, gitignore, and some sort of makefile
*/

#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

constexpr uint32_t MEM_SIZE = 4096; // 4KB of memory
constexpr uint32_t PC_START = 0;
constexpr uint32_t BIN_IMPORT_ADDR = PC_START;

constexpr uint8_t OPCODE_R_ALU = 0x33;
constexpr uint8_t OPCODE_I_ALU = 0x13;
constexpr uint8_t OPCODE_I_LOAD   = 0x03;
constexpr uint8_t OPCODE_S_STORE  = 0x23;
constexpr uint8_t OPCODE_B_BRANCH = 0x63;
constexpr uint8_t OPCODE_U_LUI    = 0x37;
constexpr uint8_t OPCODE_U_AUIPC  = 0x17;
constexpr uint8_t OPCODE_I_JALR   = 0x67;
constexpr uint8_t OPCODE_J_JAL    = 0x6F;

enum class InstructionType {
    R_TYPE,
    I_TYPE,
    S_TYPE,
    B_TYPE,
    U_TYPE,
    J_TYPE
};

struct instruction{
    uint8_t opcode;
    uint8_t funct7;
    uint8_t funct3;
    uint8_t rs1;
    uint8_t rs2;
    uint8_t rd;
    int32_t imm;
};

int32_t signExtendImmediate(uint32_t value, uint8_t MSB){
    //The idea here is that if the most significant bit is 1 then the immediate is negative.
    //Then we have to make everything below the MSB into 1s.
    //To do that, we shift 1 over MSB times and subtract 1 to create 00011111....
    //Then we negate that and OR it to mask the first bits as 1s.
    if(value & (1 << MSB)){
        return value | ~((1 << (MSB + 1))-1);
    }
    return value;
}

struct Machine {
    uint32_t regs[32] = {0};
    uint8_t mem[MEM_SIZE] = {0};
    uint32_t pc = {0};
    bool debug_mode = false;
    uint32_t fetch(){
        if(pc > MEM_SIZE - 4) dumpProcessorState("PC exceeded memory size");
        if(pc % 4 != 0) dumpProcessorState("PC is not 4-byte aligned");
        
        uint32_t instruction = *((uint32_t*)(mem + pc)); // Fetch instruction (4 bytes)

        if(debug_mode) printf("[FETCH]: 0x%08X\n", instruction);

        return instruction;
    }
    void executeClockCycle(){
        uint32_t instruction = fetch();
        
        int32_t pc_increment = 4;

        uint8_t opcode = (instruction >> 0 ) & 0x0000007F;//Extract bits 0-6
        uint8_t funct7 = (instruction >> 25) & 0x0000007F; //Extract bits 25-31
        uint8_t funct3 = (instruction >> 12) & 0x00000007; //Extract bits 12-14
        uint8_t rs1 =    (instruction >> 15) & 0x0000001F; //Extract bits 15-19
        uint8_t rs2 =    (instruction >> 20) & 0x0000001F; //Extract bits 20-24
        uint8_t rd  =    (instruction >> 7 ) & 0x0000001F; //Extract bits 7-11
        if(debug_mode){
            printf("opcode: 0x%02X\n", opcode);
            printf("funct7: 0x%02X\n", funct7);
            printf("funct3: 0x%02X\n", funct3);
            printf("rs1   : 0x%02X, val: %d\n", rs1   , regs[rs1]);
            printf("rs2   : 0x%02X, val: %d\n", rs2   , regs[rs2]);
            printf("rd    : 0x%02X, val: %d\n", rd    , regs[rd ]);
        }

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
                if(address >= MEM_SIZE || (address + loadSize) > MEM_SIZE){
                    printf("ERROR! address exceeds memory size. ");
                    exit(EXIT_FAILURE);
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
                if((address + loadSize) > MEM_SIZE || address >= MEM_SIZE){
                    printf("ERROR! address exceeds memory size. ");
                    exit(EXIT_FAILURE);
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
                regs[rd] = (instruction & 0xFFFFF000) + pc;
                break;
            }
            case(0x67):{ // I-type jalr instruction
                regs[rd] = pc + 4;
                pc = (regs[rs1] + (((int32_t)instruction) >> 20)) & ~0x01;
                pc_increment = 0;
                break;
            }
            case(0x6F):{ //J-type jal instruction
                regs[rd] = pc + 4;
                
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
                dumpProcessorState("Undefined opcode encountered");
            }
        }
        
        regs[0] = 0;
        pc += pc_increment;

        if(debug_mode) printf("New rd value: %d, New PC: %08X\n\n", regs[rd], pc);
        
    }
    instruction decode(int32_t encoded){
        instruction decoded = {0};
        decoded.opcode = (encoded >> 0 ) & 0x0000007F;//Extract bits 0-6
        decoded.funct7 = (encoded >> 25) & 0x0000007F; //Extract bits 25-31
        decoded.funct3 = (encoded >> 12) & 0x00000007; //Extract bits 12-14
        decoded.rs1    = (encoded >> 15) & 0x0000001F; //Extract bits 15-19
        decoded.rs2    = (encoded >> 20) & 0x0000001F; //Extract bits 20-24
        decoded.rd     = (encoded >> 7 ) & 0x0000001F; //Extract bits 7-11

        decoded.imm = 0;
        switch(decoded.opcode){
            case(OPCODE_R_ALU): break;
            case(OPCODE_I_ALU):
            case(OPCODE_I_LOAD):
            case(OPCODE_I_JALR):
                decoded.imm = ((int32_t)encoded) >> 20;
                break;
            case(OPCODE_S_STORE):
                break;
            case(OPCODE_B_BRANCH):
                break;
            case(OPCODE_J_JAL):
                break;
            case(OPCODE_U_LUI):
            case(OPCODE_U_AUIPC):
                break;
            default:
                dumpProcessorState("Undefined opcode encountered during immediate decoding");
                break;
        }
        return decoded;
    };
    void dumpProcessorState(const std::string& reason){
        printf("\n\nDumping processor state...\n\nRegisters:\n");
        printf("reg0 :%08X, reg1 :%08X, reg2 :%08X, reg3 :%08X\n", regs[0 ], regs[1 ], regs[2 ], regs[3 ]);
        printf("reg4 :%08X, reg5 :%08X, reg6 :%08X, reg7 :%08X\n", regs[4 ], regs[5 ], regs[6 ], regs[7 ]);
        printf("reg8 :%08X, reg9 :%08X, reg10:%08X, reg11:%08X\n", regs[8 ], regs[9 ], regs[10], regs[11]);
        printf("reg12:%08X, reg13:%08X, reg14:%08X, reg15:%08X\n", regs[12], regs[13], regs[14], regs[15]);
        printf("reg16:%08X, reg17:%08X, reg18:%08X, reg19:%08X\n", regs[16], regs[17], regs[18], regs[19]);
        printf("reg20:%08X, reg21:%08X, reg22:%08X, reg23:%08X\n", regs[20], regs[21], regs[22], regs[23]);
        printf("reg24:%08X, reg25:%08X, reg26:%08X, reg27:%08X\n", regs[24], regs[25], regs[26], regs[27]);
        printf("reg28:%08X, reg29:%08X, reg30:%08X, reg31:%08X\n", regs[28], regs[29], regs[30], regs[31]);
        
        printf("\nProgram Counter: %08X\n", pc);

        printf("\nReason for dump: %s\n\n", reason.c_str());
        printf("End of processor state dump.\n\n");
        exit(EXIT_FAILURE);
    }
};

static void importBinary(const std::string& filename, uint8_t* mem){
    //Loads a file into memory!
    FILE* testing_binary = fopen(filename.c_str(), "rb");
    if (testing_binary == NULL) {
        printf("Error: Could not open %s\n", filename.c_str());
        exit(EXIT_FAILURE); // Exit the simulator if the file fails to load
    }
    size_t bytes_read = fread(mem + BIN_IMPORT_ADDR, sizeof(uint8_t), MEM_SIZE - BIN_IMPORT_ADDR, testing_binary);
    printf("Successfully loaded %zu bytes into memory.\nStarting the emulator...\n\n", bytes_read);
    //I should probaly make this function check to make sure it doesn't overflow memory...    
}

int main(int argc, char* argv[]){

    printf("Initializing RV32I emulator by Creed Truman...\n");
    if(PC_START %2 != 0){
        printf("ERROR! PC_START is not 2-byte aligned.");
        exit(EXIT_FAILURE);
    }

    Machine emulator;

    if(argc > 1){
        if(std::string(argv[1]) == "-debug" || std::string(argv[1]) == "-d"){
            emulator.debug_mode = true;
        }
    }

    importBinary("rv_test.bin", emulator.mem);

    while(true){
        emulator.executeClockCycle();
    }
}