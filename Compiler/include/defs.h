#ifndef INCLUDE_DEFS_H
#define INCLUDE_DEFS_H

#include <string>
#include <map>

#define DEBUG
#include "Logger.h"

#define SIZE(a) (sizeof(a)/sizeof(a[0]))

const char* operators[];
const char* punctuators[];
const char* types[];
const char* modifiers[];
const char* constructs[];

const char* TokenNames[];

enum class TokenTypes {
    OPERATOR, PUNCTUATOR,
    MODIFIER, TYPE, CONSTRUCT,
    NO_MATCH, NUMBER, STRING, IDENTIFIER,
    ENDOFFILE,
    TOKEN_TYPES_COUNT
};

enum class Operators {
    PLUS, MINUS, DIV, MUL_DERF,
    LSR, LSL, AND_ADDR, OR, MOD, 
    NOT, INCREMENT, DECREMENT, EQ, 
    PLUS_EQ, MINUS_EQ, DIV_EQ, MUL_EQ,
    LSR_EQ, LSL_EQ, AND_EQ, 
    MOD_EQ, LAND, LOR, LEQ, LNEQ,
    LGT, LLT, LT_EQ, GT_EQ, OPERATORS_COUNT
};

enum class Punctuators {
    EOL, OBRACK, CBRACK,
    COMMA, COLON, OPARN, 
    CPARN, OBRACE, CBRACE, 
    PUNCTUATORS_COUNT
};

enum class Modifiers {
    CONST, STATIC, 
    EXTERN, SIGNED,
    UNSIGNED, PUBLIC, PRIVATE,
    MODIFIERS_COUNT
};

enum class Types {
    DOUBLE, FLOAT, LONG, INT, SHORT, AUTO, CHAR, REGISTER, VOID, BOOL, TYPES_COUNT
};

enum class Constructs {
    TYPEDEF, BREAK, CONTNUE, CASE, SWITCH, DEFAULT, DO, WHILE, ELSE, ENUM, UNION, FOR, IF, STRUCT, CLASS, RETURN, CONSTRUCTS_COUNT
};

#define DOUBLE_SIZE 4
#define FLOAT_SIZE 4
#define LONG_SIZE 4
#define INT_SIZE 4
#define SHORT_SIZE 2
#define CHAR_SIZE 1
#define REGISTER_SIZE 4
#define VOID_SIZE 1
#define BOOL_SIZE 1

#endif