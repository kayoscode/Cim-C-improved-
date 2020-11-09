#ifndef INCLUDE_DEFS_H
#define INCLUDE_DEFS_H

#define DEBUG
//#define DEBUG_MEMORY
//#define DEBUG_REGISTERS
#define DEBUG_INSTRUCTIONS

#include <stdint.h>

#include "Logger.h"

#define FP_EPSILON .0000001f

enum Registers {
    R0, R1, R2, R3,
    R4, R5, R6, R7,
    R8, R9, R10, R11,
    R12, R13, R14, R15,
    PC, BP, SP, FR, RA, 
    REGISTER_COUNT
};

const char* registerNames[];
const char* fregisterNames[];

enum REGISTER_FLAGS {
	FL_NEGATIVE = 1,
	FL_ZERO = 2,
	FL_CARRY = 4,
	FL_OVERFLOW = 8,
	FL_ENABLE_INTERRUPT = 16
};

//specified with syntax: operation DREG, SREG, OP(REG/IMM)
enum ARITHMETIC_LOGIC {
	LSL, LSR,
	ADD, SUB, 
	MUL, DIV,
	ULSL, ULSR,
	UADD, USUB,
	AND, 
	OR, XOR,
};

//syntax: MOV/CMP reg1/dest, reg2/source/imm
enum SPECIAL_ARITHMETIC {
	MOV, CMP, LA, MOV_GE, MOV_G, MOV_LE, MOV_L, MOV_E, MOV_C, MOV_NE
};

//opcode immediate/reg
enum BRANCH {
	JMP, JGE, //jmp = always jump, jge = jump if 0 or not negitive
	JG, JLE,  //jg = jump if negitive not set, jle = 0 or negitive is set
	JL, JE,   //jl = if negitive is set, je = 0
	JC, JNE,  //jc = jump if carry, jo = jump if overflow
	CALL
};

//opcode [addr register], imm/source register
enum MAIN_MEM {
	SB, SH, SW,
	LB, LH, LW,
};

//INT
enum INTERRPUT {
	SW_INTERRUPT, HW_INTERRUPT
};

enum OPERATION_CATEGORY { //used by decoder to parse instructions
	ARITHMETIC_LOGIC = 0b000,
	SPECIAL_ARITHMETIC = 0b1,
	FP_ARITHMETIC = 0b010,
	MEM_INSTRUCTIONS = 0b011,
	JUMP_INSTRUCTIONS = 0b100,
	FP_SPECIAL = 0b101,
	INTERRUPT = 0b110,
	STACK = 0b111
};

enum STACK {
	PUSH,
	POP
};

enum FLOATING_POINT { 
	F_ADD,
	F_SUB,
	F_MUL,
	F_DIV,
};

enum FP_SPECIAL {
	F_MOV,
	F_CMP,
	F_S,
	F_L,
	F_RTOF
};

enum FP_REGISTERS { 
	F0, F1,
	F2, F3,
	F4, F5,
	F6, F7,
	F8, F9,
	F10, F11,
	F12, F13, 
	F14, F15,
    FREGISTER_COUNT
};

#endif