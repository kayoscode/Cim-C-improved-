
#ifndef INCLUDE_ASM_H
#define INCLUDE_ASM_H

#include <string>
#include <vector>
#include <iostream>

#include "defs.h"

enum class TokenType {
    GLOBAL_SEGMENT, SEGMENT, INSTRUCTION, REGISTER,
    IDENTIFIER, NUMBER, STRING, 
    OPERATOR, PUNCTUATOR, ENDOFFILE,
    NO_MATCH
};

enum class InstructionType {
    AL, SA, FPA, FPS, MEM, INT, STACK, JMP, NO_INST
};

enum GrammarNodeName {
    FILE_START, ASM_FILE, GLOBAL_SEGMENT, DATA_SEGMENT, TEXT_SEGMENT,
    LABEL, EXPRESSION, DATA_DEFINITION, DATA_DEF_LIST, DATA_LIST, 
    INSTRUCTION, CODE_LIST, OPERAND_LIST, REGISTER, ADDRESS_OFFSET, COPY_COUNT, OFFSET, 
    REGISTER_LIST, TERMINAL,
};

const char* grammarNodeNames[];

struct Token {
    char* start;
    char* end;
    TokenType type;
    int instType;
    int typeIndex;
};

struct GrammarTree{
    GrammarTree(int nodeType, Token* nodeStart)
        :nodeType(nodeType),
        nodeStart(nodeStart),
        nodeEnd(nodeStart)
    {}

    ~GrammarTree(){
        for(int i = 0; i < subNodes.size(); i++){
            delete subNodes[i];
        }
    }

    inline bool isTerminal(){
        return subNodes.size() == 0;
    }

    inline void addSubNode(GrammarTree* subNode) {
        subNodes.push_back(subNode);
        nodeEnd = subNode->nodeEnd;
    }

    inline void nullTerminate() {
        tmp = *nodeEnd->end;
        *nodeEnd->end = 0;
    }

    inline void unNullTerminate() {
        *nodeEnd->end = tmp;
    }

    void printText();
    void print(int depth = 0);

    int nodeType;
    Token* nodeStart;
    Token* nodeEnd;
    char tmp;

    std::vector<GrammarTree*> subNodes;
};

int assembleFile(const std::string& inputFile, const std::string& outputFile);
void printToken(Token& token);
void printTokenText(Token& token);
bool iequals(const char* a, const char* b, uint32_t count);

#endif