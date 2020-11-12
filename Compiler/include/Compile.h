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
struct GrammarTree;

struct Type {
    Type() :size(1) {}
    Type(uint32_t size) :size(size) {}
    uint32_t size;
    GrammarTree* type = nullptr;
};

struct Identifier {
    Type type;
    GrammarTree* idt = nullptr;
};

struct GrammarTree{
    GrammarTree(int nodeType, Token* nodeStart, GrammarTree* parent)
        :nodeType(nodeType),
        nodeStart(nodeStart),
        nodeEnd(nodeStart),
        parent(parent),
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

    inline void addType(Token* name, Type type) {
        char tmp = *name->end;
        *name->end = 0;
        localTypes.emplace(name->start, type);
        *name->end = tmp;
    }

    void addIdentifier(Token* name, Identifier idt) {
        if(parent != nullptr && (nodeType != STATEMENT_LIST && nodeType != GLOBAL_LIST)) {
            parent->addIdentifier(name, idt);
        }
        else {
            char tmp = *name->end;
            *name->end = 0;
            localIdentifiers.emplace(name->start, idt);
            *name->end = tmp;
        }
    }

    /**
    void addOperandIdentifier(Token* name, Identifier idt) {
        if(parent != nullptr && (nodeType != FUNCTION)) {
            parent->addOperandIdentifier(name, idt);
        }
        else {
            char tmp = *name->end;
            *name->end = 0;
            localIdentifiers.emplace(name->start, idt);
            *name->end = tmp;
        }
    }
    */

    //for adding functions to the global scope of the function
    void addScopeIdentifier(Token* name, Identifier type) {
        if(parent != nullptr && parent->nodeType != STATEMENT_LIST && parent->nodeType != GLOBAL_LIST) {
            parent->addScopeIdentifier(name, type);
        }
        else {
            char tmp = *name->end;
            *name->end = 0;
            localIdentifiers.emplace(name->start, type);
            *name->end = tmp;
        }
    }

    bool getIdentifier(Token* name, Identifier& ret) {
        return true;
    }

    GrammarTree* getType(Token* name, Type& type) {
        char tmp = *name->end;
        *name->end = 0;

        if(localTypes.find(name->start) != localTypes.end()) {
            *name->end = tmp;
            return this;
        }
        else if(GrammarTree* ret = parent->getType(name, type)) {
            *name->end = tmp;
            return ret;
        }

        *name->end = tmp;
        return nullptr;
    }

    GrammarTree* getIdentifier(Token* name, int& idt, bool defaultCheck = true) {
        char tmp = *name->end;
        *name->end = 0;
        *name->end = tmp;
        return nullptr;
    }

    void printText();
    void print(int depth = 0);

    char nodeType;
    Token* nodeStart;
    Token* nodeEnd;
    char tmp;

    std::map<std::string, Type> localTypes;
    std::map<std::string, Identifier> localIdentifiers;

    //stores the parent in which the type was defined for log(n) access next time its searched for from a child node
    //std::map<std::string, GrammarTree*> identifierParent;
    //std::map<std::string, GrammarTree*> typeParent;

    GrammarTree* parent = nullptr;
    std::vector<GrammarTree*> subNodes;
};

int compileFile(const std::string& inputFile, const std::string& outputFile);
bool sequals(const char* a, const char* b, uint32_t count);
void printToken(Token& token);

#endif