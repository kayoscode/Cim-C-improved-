#ifndef INCLUDE_ENCODE_H
#define INCLUDE_ENCODE_H

#include <stdint.h>

#define STACK_REG(r, f) ((f << 5) | r)
#define GETFP_REG(r) ((r >> 5) & 1)

//NOTE: THESE NEED TO BE IN THE EXACT SAME ORDER AS THE STRINGS DEFINED IN defs.cpp because it uses their index in the list as the opcode number
enum class ALOps {
	LSL, LSR,
	ADD, SUB, 
	MUL, DIV,
	ULSL, ULSR,
	UADD, USUB,
	AND, 
	OR, XOR
};

enum class SAOps {
	MOV, CMP, 
    LA, MOV_GE, 
    MOV_G, MOV_LE, 
    MOV_L, MOV_E, 
    MOV_C, MOV_NE
}; //TODO FTOR

enum class FPAOps {
    F_ADD, F_SUB,
    F_MUL, F_DIV
};

enum class FPSAOps {
    MOV, CMP,
    F_S, F_L,
    RTOF
};

enum class MEMOps {
    SB, SH,
    SW, LB,
    LH, LW
};

enum class INTOps {
    INT, HWINT
};

enum class STACKOps {
    PUSH, POP
};

enum class JMPOps {
    JMP, JGE, 
	JG, JLE, 
	JL, JE,
	JC, JNE,
	CALL
};

enum class Registers {
    R0, R1, R2, R3,
    R4, R5, R6, R7,
    R8, R9, R10, R11,
    R12, R13, R14, R15,
    PC, BP, SP, FR, RA, 
    F0, F1, F2, F3, 
    F4, F5, F6, F7, 
    F8, F9, F10, F11, 
    F12, F13, F14, F15, 
};

void encodeAL(uint32_t* buffer, int& index, ALOps op, uint8_t dest, uint8_t src, uint32_t val, bool immediate);
void encodeSA(uint32_t* buffer, int& index, SAOps op, uint8_t dest, uint32_t arg, bool immediate);
void encodeFPA(uint32_t* buffer, int& index, FPAOps op, uint8_t dest, uint8_t src1, uint8_t src2);
void encodeFPSA(uint32_t* buffer, int& index, FPSAOps op, uint8_t dest, uint32_t arg, bool immediate);
void encodeMEM(uint32_t* buffer, int& index, MEMOps op, uint8_t reg, uint8_t addrReg, uint32_t offset, uint32_t count);
void encodeJMP(uint32_t* buffer, int& index, JMPOps op, uint32_t arg, bool immediate);
void encodeINT(uint32_t* buffer, int& index, INTOps op, uint32_t interruptId);
void encodeSTACK(uint32_t* buffer, int& index, STACKOps ops, uint32_t registerCount, uint8_t registers[4]);

#endif