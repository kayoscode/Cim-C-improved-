
#ifndef INCLUDE_BINARY_H
#define INCLUDE_BINARY_H

#include "defs.h"
#include "fstream"
#include "Asm.h"

struct GrammarTree;
struct Token;

struct Idt {
    uint32_t value;
    bool label;
    bool defined = false;
    std::vector<uint32_t> usages;
};

struct ExpressionTree {
    ExpressionTree()
        :v(nullptr), left(nullptr), right(nullptr), value(0), negated(false)
    {}

    virtual ~ExpressionTree() {
        delete left;
        delete right;
    }

    void printGroupings() {
        if(negated) {
            std::cout << "-";
        }

        if(v == nullptr) {
            std::cout << "(";

            if(left != nullptr)
                left->printGroupings();


            if(right != nullptr) {
                std::cout << " " << op << " ";
                right->printGroupings();
            }

            std::cout << ")";
        }
        else {
            printTokenText(*v);
        }
    }

    Token* v;
    char op = 0;
    ExpressionTree* left;
    ExpressionTree* right;
    int32_t value;
    bool negated;
};

int generateBinary(GrammarTree* tree, std::ostream& output);

#endif