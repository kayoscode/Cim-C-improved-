#ifndef INCLUDE_COMPILE_H
#define INCLUDE_COMPILE_H

#include <string>
#include <fstream>
#include <map>
#include <unordered_map>
#include "defs.h"

struct Token {
    char* start;
    char* end;
    int type;
    int subType;
    int line;
};

enum GrammarNodeName {
    FILE_START, LANGUAGE_FILE, GLOBAL, GLOBAL_LIST, DECLARATION, INTIALIZATION, POINTER_TYPE,
    ASSIGNMENT, EXPRESSION, MODIFIER, MODIFIER_LIST, TYPE, FUNCTION_DECLARATION, FUNCTION_DEC_OPERAND, FUNCTION_OPERANDS,
    FUNCTION, FUNCTION_DEC_ARG_LIST, STATEMENT_LIST, CODE_BLOCK,
    STATEMENT, SWITCH_STATEMENT, CASE, CASE_LIST, ARRAY_TYPE, FUNCTION_PTR_TYPE,
    IF_STATEMENT, ELSE_IF_STATEMENT, ELSE_STATEMENT,
    TYPEDEF, WHILE_LOOP, DO_WHILE_LOOP, FOR_LOOP, FUNCTION_CALL_ARGS,
    SUB_STATEMENT, FUNCTION_CALL, LOOP, RETURN_STATEMENT, TERMINAL, LVALUE, RVALUE, PREFIX_OPERATOR,
    STRUCTURE_DEFINTION, STRUCTURE, STRUCT, CLASS
};

const char* grammarNodeNames[];

struct Type {
    Type() :size(1) {}
    Type(uint32_t size) :size(size) {}
    uint32_t size;
    //include things like a memory layout
    //include metadata like member names, method sizes, etc
};

struct GrammarTree{
    GrammarTree(int nodeType, Token* nodeStart)
        :nodeType(nodeType),
        nodeStart(nodeStart),
        nodeEnd(nodeStart),
        tmp(0)
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
        subNode->parent = this;
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

    inline void addType(const char* typeName, Type type) {
        localTypes.emplace(typeName, type);
    }

    void getType(const char* name, Type& type) {
    }

    void getIdentifier(const char* identifier, int& idt) {
    }

    void printText();
    void print(int depth = 0);

    char nodeType;
    Token* nodeStart;
    Token* nodeEnd;
    char tmp;

    std::map<std::string, Type> localTypes;
    std::map<std::string, int> localIdentifiers;
    std::map<std::string, GrammarTree*> identifierParent;
    std::map<std::string, GrammarTree*> typeParent;

    GrammarTree* parent = nullptr;
    std::vector<GrammarTree*> subNodes;
};

int compileFile(const std::string& inputFile, const std::string& outputFile);
bool sequals(const char* a, const char* b, uint32_t count);

#endif