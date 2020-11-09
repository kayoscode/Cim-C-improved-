#include "defs.h"

const char* TokenNames[] {
    "GLOBAL_SEGMENT", "SEGMENT", "INSTRUCTION", "REGISTER",
    "IDENTIFIER", "NUMBER", "STRING", 
    "OPERATOR", "PUNCTUATOR", "EOF",
    "NO_MATCH"
};

const char* operators[] {
    "+", "-", "*", "/"
};

const char* punctuators[] {
    "[", "]",
    ",", ".",
    ":",
    "(" ,")", "="
};

const char* globalSegment[] {
    "DATA", "TEXT", "BSS"
};

const char* segment[] {
    "LABEL",
    "WORD", "HALF", "BYTE", "FLOAT", 
    "SPACE", "CONST", "ASCII", "ASCIIZ"
};

const char* registers[] {
    "R0", "R1", "R2", "R3",
    "R4", "R5", "R6", "R7",
    "R8", "R9", "R10", "R11",
    "R12", "R13", "R14", "R15",
    "PC", "BP", "SP", "FR", "RA",
    "F0", "F1", "F2", "F3",
    "F4", "F5", "F6", "F7",
    "F8", "F9", "F10", "F11",
    "F12", "F13", "F14", "F15",
    "F12", "F13", "F14", "F15"
};

const char* ALInstructions[] {
	"LSL", "LSR",
	"ADD", "SUB", 
	"MUL", "DIV",
	"ULSL", "ULSR",
	"UADD", "USUB",
	"AND", 
	"OR", "XOR"
};

const char* SAInstructions[] {
	"MOV", "CMP", 
    "LA", "MOVGE", 
    "MOVG", "MOVLE", 
    "MOVL", "MOVE", 
    "MOVC", "MOVNE"
};

const char* FPAInstructions[] {
    "FADD", "FSUB",
    "FMUL", "FDIV"
};

const char* FPSInstructions[] {
    "FMOV", "FCMP",
    "FS", "FL",
    "RTOF"
};

const char* MEMInstructions[] {
    "SB", "SH",
    "SW", "LB",
    "LH", "LW"
};

const char* INTInstructions[] {
    "INT", "HWINT"
};

const char* STACKInstructions[] {
    "PUSH", "POP"
};

const char* JMPInstructions[] {
    "JMP", "JGE", 
	"JG", "JLE", 
	"JL", "JE",
	"JC", "JNE",
	"CALL"
};