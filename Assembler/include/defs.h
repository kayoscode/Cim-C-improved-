#ifndef INCLUDE_DEFS_H
#define INCLUDE_DEFS_H

#include <string>
#include <map>

#include "Logger.h"

#define SIZE(a) (sizeof(a)/sizeof(a[0]))

#define OPERATOR_SIZE 4
#define PUNC_SIZE 8
#define GLOBAL_SEG_SIZE 3
#define SEG_SIZE 9
#define REG_SIZE 41
#define AL_INST_SIZE 13
#define SA_INST_SIZE 10
#define FPA_INST_SIZE 4
#define FPS_INST_SIZE 5
#define MEM_INST_SIZE 6
#define INT_INST_SIZE 2
#define STACK_INST_SIZE 2
#define JMP_INST_SIZE 9

const char* operators[];
const char* punctuators[];
const char* globalSegment[];
const char* segment[];
const char* registers[];
const char* ALInstructions[];
const char* SAInstructions[];
const char* FPAInstructions[];
const char* FPSInstructions[];
const char* MEMInstructions[];
const char* INTInstructions[];
const char* STACKInstructions[];
const char* JMPInstructions[];

const char* TokenNames[];

#endif