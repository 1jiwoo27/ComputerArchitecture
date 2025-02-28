#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define NUM_REGISTERS 32

typedef struct
{
    uint32_t value;
    int locked; // Flag to indicate if the register is locked
} Register;

Register registers[NUM_REGISTERS];

void initializeRegisters()
{
    for (int i = 0; i < NUM_REGISTERS; i++)
    {
        registers[i].value = 0;
        registers[i].locked = 0;
    }

    // Lock x0 to ensure it stays at 0
    registers[0].locked = 1;
}

uint32_t programCounter = 0; // New register for the program counter

uint32_t readRegister(int regNum)
{
    return registers[regNum].value;
}

void writeRegister(int regNum, uint32_t value)
{
    if (!registers[regNum].locked)
    {
        registers[regNum].value = value;
    }
    else
    {
        printf("Warning: Attempted write to locked register x%d ignored.\n", regNum);
    }
}

void processRType(uint32_t instruction);
void processIType(uint32_t instruction);
void processSType(uint32_t instruction);
void processUType(uint32_t instruction);
void processBType(uint32_t instruction);
void processJALType(uint32_t instruction);
void processJALRType(uint32_t instruction);

int main(int argc, char *argv[])
{
    initializeRegisters();

    if (argc != 2)
    {
        printf("Usage: RiscVSimulator <input_file>\n");
        return 1;
    }

    char *inputFileName = argv[1];

    // Check if the file exists
    FILE *file = fopen(inputFileName, "rb");
    if (!file)
    {
        printf("Error: File '%s' not found.\n", inputFileName);
        return 1;
    }

    // Read binary instructions from the specified file
    while (1)
    {
        // Save the current program counter
        uint32_t currentPC = programCounter;

        // Seek to the correct position in the file
        fseek(file, currentPC, SEEK_SET);

        uint32_t instruction;
        if (fread(&instruction, sizeof(uint32_t), 1, file) != 1)
        {
            // End of file or error, break out of the loop
            break;
        }

        // Extract opcode and other fields, classify into instruction groups, and execute them
        // Extract opcode (bits 0-6)
        uint32_t opcode = instruction & 0x7F;

        printf("Instruction: %08X, Opcode: %02X\n", instruction, opcode);

        switch (opcode)
        {
        case 0x33: // R-type opcode
            printf("R-type instruction\n");
            processRType(instruction);
            break;
        case 0x13: // I-type opcode
            printf("I-type instruction\n");
            processIType(instruction);
            break;
        case 0x23: // S-type opcode
            printf("S-type instruction\n");
            processSType(instruction);
            break;
        case 0x37: // U-type opcode
            printf("U-type instruction\n");
            processUType(instruction);
            break;
        case 0x73: // E-call opcode
            printf("E-call instruction\nThe program has ended.\n\n");
            printf("Simulation completed.\n");
            return 0;
        case 0x17: // AUIPC opcode
            printf("AUIPC instruction\n");
            processUType(instruction);
            break;
        case 0x63: // B-type opcode
            printf("B-type instruction\n");
            processBType(instruction);
            break;
        case 0x6F: // JAL opcode
            printf("JAL instruction\n");
            processJALType(instruction);
            break;
        case 0x67: // JALR opcode
            printf("JALR instruction\n");
            processJALRType(instruction);
            break;
        default:
            printf("Error: Unrecognized opcode '%02X'.\n", opcode);
            break;
        }
    }

    fclose(file);

    printf("Simulation completed.\n");
    return 0;
}

void processRType(uint32_t instruction)
{
    // Process R-type instruction
    uint32_t rd = (instruction >> 7) & 0x1F;
    uint32_t funct3 = (instruction >> 12) & 0x7;
    uint32_t rs1 = (instruction >> 15) & 0x1F;
    uint32_t rs2 = (instruction >> 20) & 0x1F;
    uint32_t funct7 = (instruction >> 25) & 0x7F;

    // Print values before execution in hexadecimal
    printf("Before R-type execution: x%d = 0x%X, x%d = 0x%X, x%d = 0x%X\n", rd, registers[rd].value, rs1, registers[rs1].value, rs2, registers[rs2].value);

    switch (funct3)
    {
    case 0x0: // ADD/SUB
        if (funct7 == 0x00)
        {
            // add (Addition)
            printf("ADD\n");
            registers[rd].value = registers[rs1].value + registers[rs2].value;
        }
        else if (funct7 == 0x20)
        {
            // sub (Subtraction)
            printf("SUB\n");
            registers[rd].value = registers[rs1].value - registers[rs2].value;
        }
        else
        {
            printf("Unrecognized R-type instruction input\n");
        }
        break;

    case 0x1: // SLL
        printf("SLL\n");
        registers[rd].value = registers[rs1].value << registers[rs2].value;
        break;

    case 0x2: // SLT
        printf("SLT\n");
        registers[rd].value = ((int32_t)registers[rs1].value < (int32_t)registers[rs2].value) ? 1 : 0;
        break;

    case 0x3: // SLTU
        printf("SLTU\n");
        registers[rd].value = (registers[rs1].value < registers[rs2].value) ? 1 : 0;
        break;

    case 0x4: // XOR
        printf("XOR\n");
        registers[rd].value = registers[rs1].value ^ registers[rs2].value;
        break;

    case 0x5: // SRL/SRA
        if (funct7 == 0x00)
        {
            // srl (Shift Right Logical)
            printf("SRL\n");
            registers[rd].value = registers[rs1].value >> registers[rs2].value;
        }
        else if (funct7 == 0x20)
        {
            // sra (Shift Right Arithmetic)
            printf("SRA\n");
            registers[rd].value = (int32_t)registers[rs1].value >> registers[rs2].value;
        }
        else
        {
            printf("Unrecognized R-type instruction input\n");
        }
        break;

    case 0x6: // OR
        printf("OR\n");
        registers[rd].value = registers[rs1].value | registers[rs2].value;
        break;

    case 0x7: // AND
        printf("AND\n");
        registers[rd].value = registers[rs1].value & registers[rs2].value;
        break;

    default:
        printf("Unrecognized R-type instruction input\n");
        break;
    }

    // Print values after execution in hexadecimal
    printf("After R-type execution: x%d = 0x%X, x%d = 0x%X, x%d = 0x%X\n\n", rd, registers[rd].value, rs1, registers[rs1].value, rs2, registers[rs2].value);

    programCounter += 4;
}

void processIType(uint32_t instruction)
{
    // Process I-type instruction
    uint32_t rd = (instruction >> 7) & 0x1F;
    uint32_t funct3 = (instruction >> 12) & 0x7;
    uint32_t rs1 = (instruction >> 15) & 0x1F;
    int32_t imm = (int32_t)(((instruction >> 31) ? 0xFFFFF000 : 0) | ((instruction >> 20) & 0xFFF));

    printf("Before: x%d = 0x%x, x%d = 0x%x, imm = %d\n", rd, registers[rd].value, rs1, registers[rs1].value, imm);

    // Add your I-type instruction processing logic here
    switch (funct3)
    {
    case 0x0: // ADDI
        printf("ADDI\n");
        registers[rd].value = registers[rs1].value + imm;
        break;
    case 0x1: // SLLI
        printf("SLLI\n");
        registers[rd].value = registers[rs1].value << imm;
        break;
    case 0x2: // SLTI
        printf("SLTI\n");
        registers[rd].value = ((int32_t)registers[rs1].value < (int32_t)imm) ? 1 : 0;
        break;
    case 0x3: // SLTIU
        printf("SLTIU\n");
        registers[rd].value = (registers[rs1].value < imm) ? 1 : 0;
        break;
    case 0x4: // XORI
        printf("XORI\n");
        registers[rd].value = registers[rs1].value ^ imm;
        break;
    case 0x5: // SRLI/SRAI
        printf("SRLI/SRAI\n");
        if ((instruction & 0x40000000) == 0)
        {
            // srli (Shift Right Logical Immediate)
            registers[rd].value = registers[rs1].value >> imm;
        }
        else
        {
            // srai (Shift Right Arithmetic Immediate)
            registers[rd].value = (int32_t)registers[rs1].value >> imm;
        }
        break;
    case 0x6: // ORI
        printf("ORI\n");
        registers[rd].value = registers[rs1].value | imm;
        break;
    case 0x7: // ANDI
        printf("ANDI\n");
        registers[rd].value = registers[rs1].value & imm;
        break;
    default:
        printf("Unrecognized inmediate instruction input\n");
        break;
    }

    printf("After: x%d = 0x%x, x%d = 0x%x, imm = %d\n\n", rd, registers[rd].value, rs1, registers[rs1].value, imm);

    programCounter += 4;
}

void processSType(uint32_t instruction)
{
    // Process S-type instruction
    uint32_t imm1 = (instruction >> 7) & 0x1F;
    uint32_t funct3 = (instruction >> 12) & 0x7;
    uint32_t rs1 = (instruction >> 15) & 0x1F;
    uint32_t rs2 = (instruction >> 20) & 0x1F;
    uint32_t imm2 = (instruction >> 25) & 0x7F;
    uint32_t imm = (imm2 << 5) | imm1;
    // Add your S-type instruction processing logic here

    programCounter += 4;
}

void processUType(uint32_t instruction)
{
    switch (instruction & 0x7F)
    {
    case 0x17: // AUIPC
    {
        // Process U-type instruction
        uint32_t rd = (instruction >> 7) & 0x1F;
        uint32_t imm = (instruction >> 12) & 0xFFFFF;

        // U-type instruction to implement is AUIPC
        printf("AUIPC\n");
        registers[rd].value = programCounter + (imm << 12);
        printf("x%d = 0x%x\n\n", rd, registers[rd].value);

        programCounter += 4;
        break;
    }
    case 0x37: // LUI
    {
        // Process U-type instruction
        uint32_t rd = (instruction >> 7) & 0x1F;
        uint32_t imm = (instruction >> 12) & 0xFFFFF;

        // U-type instruction to implement is LUI
        printf("LUI\n");
        registers[rd].value = imm << 12;
        printf("x%d = 0x%x\n\n", rd, registers[rd].value);

        programCounter += 4;
        break;
    }
    default:
        printf("Unrecognized U-type instruction input\n");
        programCounter += 4;
        break;
    }
}

void processBType(uint32_t instruction)
{
    // Process B-type instruction
    uint32_t imm1 = (instruction >> 7) & 0x1F;
    uint32_t funct3 = (instruction >> 12) & 0x7;
    uint32_t rs1 = (instruction >> 15) & 0x1F;
    uint32_t rs2 = (instruction >> 20) & 0x1F;
    uint32_t imm2 = (instruction >> 25) & 0x7F;

    int32_t imm = (imm2 << 5) | imm1;
    if (imm2 & 0x40)
    {
        // Set upper bits to 1 for negative values
        imm |= 0xFFFFFFC0;

        // I dont know why, but the imm value is always 1 more than it should be
        imm = imm - 1;
    }

    printf("Before B-type execution: x%d = 0x%X, x%d = 0x%X, imm = %d\n", rs1, registers[rs1].value, rs2, registers[rs2].value, (int32_t)imm);
    printf("Program counter value: %d\n", programCounter);

    switch (funct3)
    {

    case 0x0: // BEQ
        printf("BEQ\n");
        if (registers[rs1].value == registers[rs2].value)
        {
            programCounter += imm;
            printf("Branch taken\n");
        }
        else
        {
            programCounter += 4;
        }
        break;
    case 0x1: // BNE
        printf("BNE\n");
        if (registers[rs1].value != registers[rs2].value)
        {
            programCounter += imm;
            printf("Branch taken\n");
        }
        else
        {
            programCounter += 4;
        }
        break;
    case 0x4: // BLT
        printf("BLT\n");
        if ((int32_t)registers[rs1].value < (int32_t)registers[rs2].value)
        {
            programCounter += imm;
            printf("Branch taken\n");
        }
        else
        {
            programCounter += 4;
        }
        break;
    case 0x5: // BGE
        printf("BGE\n");
        if ((int32_t)registers[rs1].value >= (int32_t)registers[rs2].value)
        {
            programCounter += imm;
            printf("Branch taken\n");
        }
        else
        {
            programCounter += 4;
        }
        break;
    case 0x6: // BLTU
        printf("BLTU\n");
        if (registers[rs1].value < registers[rs2].value)
        {
            programCounter += imm;
            printf("Branch taken\n");
        }
        else
        {
            programCounter += 4;
        }
        break;
    case 0x7: // BGEU
        printf("BGEU\n");
        if (registers[rs1].value >= registers[rs2].value)
        {
            programCounter += imm;
            printf("Branch taken\n");
        }
        else
        {
            programCounter += 4;
        }
        break;
    }
    printf("After B-type execution: x%d = 0x%X, x%d = 0x%X, imm = %d\n", rs1, registers[rs1].value, rs2, registers[rs2].value, imm);
    printf("Program counter value: %d\n\n", programCounter);
}

void processJALType(uint32_t instruction)
{
    uint32_t rd = (instruction >> 7) & 0x1F;
    int32_t imm20 = (instruction >> 31) & 0x1;
    int32_t imm10to1 = (instruction >> 21) & 0x3FF;
    int32_t imm11 = (instruction >> 20) & 0x1;
    int32_t imm19to12 = (instruction >> 12) & 0xFF;

    int32_t imm = (imm20 << 20) | (imm19to12 << 12) | (imm11 << 11) | (imm10to1 << 1);

    printf("Before JAL execution: x%d = 0x%X\n", rd, registers[rd].value);

    // Execute the JAL instruction
    registers[rd].value = programCounter + 4;
    programCounter += imm;

    printf("After JAL execution: x%d = 0x%X\n\n", rd, registers[rd].value);
}

void processJALRType(uint32_t instruction)
{
    uint32_t rd = (instruction >> 7) & 0x1F;
    uint32_t funct3 = (instruction >> 12) & 0x7;
    uint32_t rs1 = (instruction >> 15) & 0x1F;
    int32_t imm = (int32_t)(((instruction >> 31) ? 0xFFFFF000 : 0) | ((instruction >> 20) & 0xFFF));

    printf("Before JALR execution: x%d = 0x%X, x%d = 0x%X, imm = %d\n", rd, registers[rd].value, rs1, registers[rs1].value, imm);

    // Execute the JALR instruction
    uint32_t jumpAddress = (registers[rs1].value + imm) & 0xFFFFFFFE; // Ensure alignment
    registers[rd].value = programCounter + 4;
    programCounter = jumpAddress;

    printf("After JALR execution: x%d = 0x%X, x%d = 0x%X, imm = %d\n\n", rd, registers[rd].value, rs1, registers[rs1].value, imm);
}