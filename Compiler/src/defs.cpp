
#include "defs.h"

const char* TokenNames[] {
    "operator", "punctuator",
    "modifier", "type", "construct", 
    "nomatch", "number", "string", 
    "identifier", "eof"
};

const char* operators[] {
    "+", "-", "/", "*", ">>", "<<", "&", "|", "%", 
    "!", "++", "--", 
    "=", "+=", "-=", "/=", "*=", ">>=", "<<=", "&=", "%=", 
    "&&", "||", "==", "!=", ">", "<", "<=", ">="
};

const char* punctuators[] {
    ";", "[", "]",
    ",", ":",
    "(" ,")",
    "{", "}",
};

const char* modifiers[] { 
    "const", "static", "extern", "signed", "unsigned", "public", "private"
};

const char* types[] {
    "float", "long", "int", "short", "auto", "char", "register", "void", "bool"
};

const char* constructs[] {
    "typedef", "break", "continue", "case", "switch", "default", "do", "while", "else", "enum", "union", "for", "if", "struct", "class", "return"
};