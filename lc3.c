#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// 1. Define memory
#define MEMORY_MAX (1 << 16)
uint16_t memory[MEMORY_MAX]; /* 65536 locations */

// 2. Define registers
// The LC-3 has 10 registers: 8 general-purpose registers (R0-R7), a Program Counter (PC), 
// and a Condition Flags (COND) register. You’ll store these in an array.

enum
{
    R_R0 = 0,
    R_R1,
    R_R2,
    R_R3,
    R_R4,
    R_R5,
    R_R6,
    R_R7,
    R_PC, /* program counter */
    R_COND,
    R_COUNT
};

uint16_t reg[R_COUNT];

// 3. opcodes - set of instructions for lc-3 VM - all the operations can be done
// using series of these operations

enum
{
    OP_BR = 0, /* branch */
    OP_ADD,    /* add */
    OP_LD,     /* load */
    OP_ST,     /* store */
    OP_JSR,    /* jump register */
    OP_AND,    /* bitwise and */
    OP_LDR,    /* load register */
    OP_STR,    /* store register */
    OP_RTI,    /* unused */
    OP_NOT,    /* bitwise not */
    OP_LDI,    /* load indirect */
    OP_STI,    /* store indirect */
    OP_JMP,    /* jump */
    OP_RES,    /* reserved (unused) */
    OP_LEA,    /* load effective address */
    OP_TRAP    /* execute trap */
};

// 4. Condition FLags

enum
{
    FL_POS = 1 << 0, /* P */
    FL_ZRO = 1 << 1, /* Z */
    FL_NEG = 1 << 2, /* N */
};

// Main loop

int main(int argc, const char* argv[])
{
    // Set the PC to the default starting position
    enum { PC_START = 0x3000 };
    reg[R_PC] = PC_START;

    // Set the condition flags, starting with Z flag
    reg[R_COND] = FL_ZRO;

    // Main loop to execute instructions
    int running = 1;
    while (running)
    {
        // FETCH: Get the instruction at the memory address stored in the PC
        uint16_t instr = memory[reg[R_PC]++];
        uint16_t op = instr >> 12;  // top 4 bits

        // DECODE AND EXECUTE: Use a switch statement to handle each opcode
        switch (op)
        {
            case OP_ADD:
            {
                // Destination register (DR)
                uint16_t r0 = (instr >> 9) & 0x7;
                // First operand (SR1)
                uint16_t r1 = (instr >> 6) & 0x7;
                // Immediate flag
                uint16_t imm_flag = (instr >> 5) & 0x1;

                if (imm_flag) 
                {
                    // Immediate mode
                    uint16_t imm5 = sign_extend(instr & 0x1F, 5);
                    reg[r0] = reg[r1] + imm5;
                } 
                else 
                {
                    // Register mode
                    uint16_t r2 = instr & 0x7;
                    reg[r0] = reg[r1] + reg[r2];
                }

                // Update condition flags
                if (reg[r0] == 0) 
                {
                    reg[R_COND] = FL_ZRO;
                } 
                else if (reg[r0] >> 15) 
                {
                    reg[R_COND] = FL_NEG;
                } 
                else 
                {
                    reg[R_COND] = FL_POS;
                }
            }
            break;

            case OP_AND:
                // Implement the AND instruction here later
            {
            
                 // Destination register (DR)
                uint16_t r0 = (instr >> 9) & 0x7;
                // First operand (SR1)
                uint16_t r1 = (instr >> 6) & 0x7;
                // Immediate flag
                uint16_t imm_flag = (instr >> 5) & 0x1;
            
                if (imm_flag) 
                {
                    // Immediate mode
                    uint16_t imm5 = sign_extend(instr & 0x1F, 5);
                    reg[r0] = reg[r1] & imm5;
                } 
                else 
                {
                    // Register mode
                    uint16_t r2 = instr & 0x7;
                    reg[r0] = reg[r1] & reg[r2];
                }
            
                // Update condition flags
                if (reg[r0] == 0) 
                {
                    reg[R_COND] = FL_ZRO;
                } 
                else if (reg[r0] >> 15) 
                {
                    reg[R_COND] = FL_NEG;
                } 
                else 
                {
                    reg[R_COND] = FL_POS;
                }
            }
            break;
            
            case OP_NOT:
            {
                // Destination register (DR)
                uint16_t r0 = (instr >> 9) & 0x7;
                // Source register (SR)
                uint16_t r1 = (instr >> 6) & 0x7;

                // Perform bitwise NOT on the value in the source register
                reg[r0] = ~reg[r1];

                // Update condition flags
                if (reg[r0] == 0) 
                {
                    reg[R_COND] = FL_ZRO;
                } 
                else if (reg[r0] >> 15) 
                {
                    reg[R_COND] = FL_NEG;
                } 
                else 
                {
                    reg[R_COND] = FL_POS;
                }
            }
            break;

            case OP_BR:
            {
                // The condition flags to check
                uint16_t cond_flag = (instr >> 9) & 0x7;
                // PCoffset9, sign-extended
                uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);

                // Check if the current condition flags meet the condition specified by the instruction
                if (cond_flag & reg[R_COND]) 
                {
                    reg[R_PC] += pc_offset;
                }
            }
            break;


            case OP_JMP:
            {
                // Base register (BR)
                uint16_t r1 = (instr >> 6) & 0x7;

                // Set the PC to the value in the base register
                reg[R_PC] = reg[r1];
            }
            break;


            case OP_JSR:
            {
                // Save the return address in R7
                reg[R_R7] = reg[R_PC];

                uint16_t long_flag = (instr >> 11) & 1;

                if (long_flag) 
                {
                    // JSR: PC-relative offset
                    uint16_t pc_offset = sign_extend(instr & 0x7FF, 11);
                    reg[R_PC] += pc_offset;  // Jump to subroutine
                } 
                else 
                {
                    // JSRR: Base register
                    uint16_t r1 = (instr >> 6) & 0x7;
                    reg[R_PC] = reg[r1];  // Jump to subroutine
                }
            }
            break;


            case OP_LD:
            {
                // Destination register (DR)
                uint16_t r0 = (instr >> 9) & 0x7;
                // PCoffset9, sign-extended
                uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);

                // Load the value from memory into the destination register
                reg[r0] = memory[reg[R_PC] + pc_offset];

                // Update condition flags
                if (reg[r0] == 0) 
                {
                    reg[R_COND] = FL_ZRO;
                } 
                else if (reg[r0] >> 15) 
                {
                    reg[R_COND] = FL_NEG;
                } 
                else 
                {
                    reg[R_COND] = FL_POS;
                }
            }
            break;


            case OP_LDI:
            {
                // Destination register (DR)
                uint16_t r0 = (instr >> 9) & 0x7;
                // PCoffset9, sign-extended
                uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);

                // First, add the offset to the PC to get the memory address of the address
                uint16_t address = memory[reg[R_PC] + pc_offset];
                // Then, load the value from the address stored in memory
                reg[r0] = memory[address];

                // Update condition flags
                if (reg[r0] == 0) 
                {
                    reg[R_COND] = FL_ZRO;
                } 
                else if (reg[r0] >> 15) 
                {
                    reg[R_COND] = FL_NEG;
                } 
                else 
                {
                    reg[R_COND] = FL_POS;
                }
            }
            break;


            case OP_LDR:
            {
                // Destination register (DR)
                uint16_t r0 = (instr >> 9) & 0x7;
                // Base register (BaseR)
                uint16_t r1 = (instr >> 6) & 0x7;
                // offset6, sign-extended
                uint16_t offset = sign_extend(instr & 0x3F, 6);

                // Load the value from memory at BaseR + offset
                reg[r0] = memory[reg[r1] + offset];

                // Update condition flags
                if (reg[r0] == 0) 
                {
                    reg[R_COND] = FL_ZRO;
                } 
                else if (reg[r0] >> 15) 
                {
                    reg[R_COND] = FL_NEG;
                } 
                else 
                {
                    reg[R_COND] = FL_POS;
                }
            }
            break;


            case OP_LEA:
            {
                // Destination register (DR)
                uint16_t r0 = (instr >> 9) & 0x7;
                // PCoffset9, sign-extended
                uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
            
                // Load the effective address (PC + offset) into the destination register
                reg[r0] = reg[R_PC] + pc_offset;
            
                // Update condition flags
                if (reg[r0] == 0) 
                {
                    reg[R_COND] = FL_ZRO;
                } 
                else if (reg[r0] >> 15) 
                {
                    reg[R_COND] = FL_NEG;
                } 
                else 
                {
                    reg[R_COND] = FL_POS;
                }
            }
            break;


            case OP_ST:
                // Implement the ST instruction here later
                break;

            case OP_STI:
                // Implement the STI instruction here later
                break;

            case OP_STR:
                // Implement the STR instruction here later
                break;

            case OP_TRAP:
                // Implement the TRAP instruction here later
                break;

            case OP_RES:
            case OP_RTI:
            default:
                // If we hit an undefined or reserved opcode, exit the program
                running = 0;
                break;
        }
    }

    // Program shutdown code here (if needed)
    return 0;
}

uint16_t sign_extend(uint16_t x, int bit_count)
{
    if ((x >> (bit_count - 1)) & 1) {
        x |= (0xFFFF << bit_count);
    }
    return x;
}

// Step 5: Implement the ADD Instruction
// The ADD instruction adds two values and stores the result in a register.
// The LC-3 architecture supports two modes for the ADD instruction:

// 1. Register Mode: The two values to be added are both stored in registers.
// 2. Immediate Mode: One value is in a register, and the other is an immediate value encoded directly in the instruction.



