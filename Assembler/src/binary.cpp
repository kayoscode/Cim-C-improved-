
#include "binary.h"
#include "Asm.h"
#include "Encode.h"
#include <map>
#include <vector>
#include <string>

#define UP_WORD_BOUNDARY(a) (((a) + 3) & 0xFFFFFFFC)
#define UP_HALF_BOUNDARY(a) (((a) + 1) & 0xFFFFFFFE)

char priority[256] = { 0 };
int pass = 0;

uint32_t byteOffset;
std::map<std::string, Idt> identifiers;
std::vector<std::string> errorList;

static int outputGlobalsegment(GrammarTree* tree, std::ostream& output);
static int outputLabel(GrammarTree* tree, std::ostream& output);
static bool createExpressionStream(Token* start, Token* end, std::vector<ExpressionTree*>& ret);
static ExpressionTree* collapseExpressionList(std::vector<ExpressionTree*> tokens);
static inline void writeByte(uint8_t byte, std::ostream& output);
static bool evalExpr(ExpressionTree* tree);
static bool evaluateExpression(Token* start, Token* end, int& output);

static inline void wordAllign(std::ostream& output) {
    uint32_t newDist = UP_WORD_BOUNDARY(byteOffset) - byteOffset;
    //std::cout << "ALIGNING: " << newDist << "\n";

    for(uint32_t i = 0; i < newDist; i++) {
        writeByte(0, output);
    }
}

static inline void halfWordAlign(std::ostream& output) {
    uint32_t newDist = UP_HALF_BOUNDARY(byteOffset) - byteOffset;
    //std::cout << "ALIGNING: " << newDist << "\n";

    for(uint32_t i = 0; i < newDist; i++) {
        writeByte(0, output);
    }
}

static inline void writeWord(uint32_t word, std::ostream& output) {
    wordAllign(output);

    if(pass > 0) {
        output.write((char*)&word, 4);
    }

    byteOffset += 4;
}

static inline void writeHalf(uint16_t half, std::ostream& output) {
    halfWordAlign(output);

    if(pass > 0) {
        output.write((char*)&half, 2);
    }

    byteOffset += 2;
}

static inline void writeByte(uint8_t byte, std::ostream& output) {
    if(pass > 0) {
        output.write((char*)&byte, 1);
    }

    byteOffset += 1;
}

static inline void writeBuffer(uint8_t* buffer, uint32_t size, std::ostream& output) {
    if(pass > 0) {
        output.write((char*)buffer, size);
    }

    byteOffset += size;
}

static inline void writeInstruction(uint32_t buffer[3], uint32_t size, std::ostream& output) {
    wordAllign(output);

    for(uint32_t i = 0; i < size; i++) {
        writeWord(buffer[i], output);
    }
}

#define ENFORCECON(con, str) if(!enforceCondition(con, str)) return false; else;

static bool enforceCondition(bool condition, std::string failError = "Syntax error") {
    if(!condition) {
        errorList.push_back(failError);
    }

    return condition;
}

static inline GrammarTree* getNextOperand(GrammarTree*& operandList) {
    if(operandList == nullptr) return nullptr;

    if(operandList->subNodes.size() > 1) {
        GrammarTree* ret = operandList->subNodes[0];
        operandList = operandList->subNodes[1];
        return ret;
    }
    else if(operandList->subNodes.size() == 1) {
        GrammarTree* ret = operandList->subNodes[0];
        operandList = nullptr;
        return ret;
    }

    return nullptr;
}

static inline bool getIntRegisterOP(GrammarTree* op, int& out) {
    if(op->nodeStart->type == TokenType::REGISTER) {
        if(op->nodeStart->typeIndex <= (int)Registers::RA) {
            out = op->nodeStart->typeIndex;
            return true;
        }
    }

    return false;
}

static inline bool getFloatRegisterOP(GrammarTree* op, int& out) {
    if(op->nodeStart->type == TokenType::REGISTER) {
        if(op->nodeStart->typeIndex >= (int)Registers::F0) {
            out = op->nodeStart->typeIndex - (int)Registers::F0;
            return true;
        }
    }

    return false;
}

static inline bool getExpressionOP(GrammarTree* op, int& out) {
    if(op->nodeType == EXPRESSION) {
        if(pass == 0) return true;
        bool ret = evaluateExpression(op->nodeStart, op->nodeEnd, out);
        return ret;
    }

    return false;
}

static inline bool outputALInstruction(GrammarTree* instruction, std::ostream& output) {
    //the add instruction always has three operands, the first two are always registers and the last one can be an expression or a register
    if(instruction->subNodes.size() > 1) {
        int opcode = instruction->nodeStart->typeIndex;
        GrammarTree* operands = instruction->subNodes[1];
        bool immediate = false;
        //int opcode = instruction->nodeStart->instOpCode;
        int dest, src, op;
        uint32_t buffer[3];
        int size = 0;

        GrammarTree* destT = getNextOperand(operands);
        GrammarTree* srcT = getNextOperand(operands);
        GrammarTree* opT = getNextOperand(operands);

        ENFORCECON(operands == nullptr && destT != nullptr && srcT != nullptr && opT != nullptr, "improper arguments");
        ENFORCECON(getIntRegisterOP(destT, dest), "destination must be a register");
        ENFORCECON(getIntRegisterOP(srcT, src), "source must be a register");
        ENFORCECON(getIntRegisterOP(opT, op) || (immediate = getExpressionOP(opT, op)), "operand must be a register or immediate/expression");

        encodeAL(buffer, size, (ALOps)opcode, dest, src, op, immediate);
        writeInstruction(buffer, size, output);

        return true;
    }

    return false;
}

/**
 * TODO: FTOR -> do not forget
 * */
static inline bool outputSAInstruction(GrammarTree* instruction, std::ostream& output) {
    ENFORCECON(instruction->subNodes.size() > 1, "arguments expected for instruction");
    int opcode = instruction->nodeStart->typeIndex;
    GrammarTree* operands = instruction->subNodes[1];
    bool immediate = false;

    int dest, op;
    uint32_t buffer[3];
    int size = 0;

    GrammarTree* destT = getNextOperand(operands);
    GrammarTree* opT = getNextOperand(operands);

    ENFORCECON(operands == nullptr && destT != nullptr && opT != nullptr, "improper arguments");
    ENFORCECON(getIntRegisterOP(destT, dest), "destination must be a register");
    ENFORCECON(getIntRegisterOP(opT, op) || (immediate = getExpressionOP(opT, op)), "operand must be a register or immediate/expression");

    if((SAOps)opcode == SAOps::LA) {
        if(immediate) {
            op = op - (byteOffset + 8);
        }
    }

    encodeSA(buffer, size, (SAOps)opcode, dest, op, immediate);
    writeInstruction(buffer, size, output);

    return true;
}

static inline bool outputFPAInstruction(GrammarTree* instruction, std::ostream& output) {
    ENFORCECON(instruction->subNodes.size() > 1, "arguments expected for instruction");

    int opcode = instruction->nodeStart->typeIndex;
    GrammarTree* operands = instruction->subNodes[1];
    int dest, src, op;
    uint32_t buffer[3];
    int size = 0;

    GrammarTree* destT = getNextOperand(operands);
    GrammarTree* srcT = getNextOperand(operands);
    GrammarTree* opT = getNextOperand(operands);

    ENFORCECON(operands == nullptr && destT != nullptr && srcT != nullptr && opT != nullptr, "improper arguments");
    ENFORCECON(getFloatRegisterOP(destT, dest), "destination must be a float register");
    ENFORCECON(getFloatRegisterOP(srcT, src), "source must be a float register");
    ENFORCECON(getFloatRegisterOP(opT, op), "operand must be a float register");

    encodeFPA(buffer, size, (FPAOps)opcode, dest, src, op);
    writeInstruction(buffer, size, output);

    return true;
}

static inline bool outputFPSInstruction(GrammarTree* instruction, std::ostream& output) {
    ENFORCECON(instruction->subNodes.size() > 1, "arguments expected for instruction");
    int opcode = instruction->nodeStart->typeIndex;
    GrammarTree* operands = instruction->subNodes[1];
    int dest, op;
    uint32_t buffer[3];
    int size = 0;
    bool immediate = false;

    GrammarTree* destT = getNextOperand(operands);
    GrammarTree* opT = getNextOperand(operands);

    ENFORCECON(operands == nullptr && destT != nullptr && opT != nullptr, "improper arguments");
    ENFORCECON(getFloatRegisterOP(destT, dest), "destination must be a float register");

    if(opcode == (int)FPSAOps::CMP || opcode == (int)FPSAOps::MOV) {
        //first one must be a float register, and the second one can be either a float register, or an immediate
        ENFORCECON(getFloatRegisterOP(opT, op) || (immediate = getExpressionOP(opT, op)), "operand must be a float register or immediate/expression");
        encodeFPSA(buffer, size, (FPSAOps)opcode, dest, op, immediate);
        writeInstruction(buffer, size, output);
    }
    else if(opcode == (int)FPSAOps::F_L || opcode == (int)FPSAOps::F_S || opcode == (int)FPSAOps::RTOF) {
        ENFORCECON(getIntRegisterOP(opT, op), "operand must be an int register");
        encodeFPSA(buffer, size, (FPSAOps)opcode, dest, op, false);
        writeInstruction(buffer, size, output);
    }

    return true;
}

static inline bool outputMEMInstructions(GrammarTree* instruction, std::ostream& output) {
    ENFORCECON(instruction->subNodes.size() > 1, "arguments expected for instruction");
    int opcode = instruction->nodeStart->typeIndex;
    GrammarTree* operands = instruction->subNodes[1];
    int value, addr, offset = 0, count = 1;
    uint32_t buffer[3];
    int size = 0;

    //the store instructions have three valid syntaxes: 
    //sx reg(value), register(addr)
    //sx reg(value), register(addr)[offset]
    //sx reg(value), register(addr)[offset]:count
    //the load instructions only have two valid syntaxes
    //the same as the store, but not the last one

    GrammarTree* valueT = getNextOperand(operands);
    GrammarTree* addrT = getNextOperand(operands);
    GrammarTree* offsetT = (addrT->subNodes.size() > 1 && addrT->subNodes[1]->subNodes.size() > 1)? addrT->subNodes[1]->subNodes[1] : nullptr;
    GrammarTree* countT = (addrT->subNodes.size() > 2 && addrT->subNodes[2]->subNodes.size() > 1)? addrT->subNodes[2]->subNodes[1] : nullptr;

    addrT = addrT->subNodes.size() > 0? addrT->subNodes[0] : addrT;

    ENFORCECON(operands == nullptr && valueT != nullptr && addrT != nullptr, "improper arguments");
    ENFORCECON(getIntRegisterOP(valueT, value), "value must be a register");
    ENFORCECON(getIntRegisterOP(addrT, addr), "address must be a register");
    ENFORCECON(offsetT == nullptr || getExpressionOP(offsetT, offset), "offset must be an expression");
    ENFORCECON(countT == nullptr || getExpressionOP(countT, count), "count must be an expression");

    if(opcode == (int)MEMOps::LB || opcode == (int)MEMOps::LH || opcode == (int)MEMOps::LW) {
        ENFORCECON(count == 1, "A count may not be defined on a load instruction");
    }

    encodeMEM(buffer, size, (MEMOps)opcode, value, addr, offset, count);
    writeInstruction(buffer, size, output);

    return true;
}

static inline bool outputINTInstruction(GrammarTree* instruction, std::ostream& output) {
    ENFORCECON(instruction->subNodes.size() > 1, "arguments expected for instruction");
    int opcode = instruction->nodeStart->typeIndex;
    GrammarTree* operands = instruction->subNodes[1];
    int value;
    uint32_t buffer[3];
    int size = 0;

    GrammarTree* valueT = getNextOperand(operands);

    ENFORCECON(operands == nullptr && valueT != nullptr, "improper arguments");
    ENFORCECON(getExpressionOP(valueT, value), "expression expected");

    encodeINT(buffer, size, (INTOps)opcode, value);
    writeInstruction(buffer, size, output);

    return true;
}

static inline GrammarTree* getNextRegisterFromList(GrammarTree*& registerList) {
    if(registerList == nullptr) return nullptr;

    if(registerList->subNodes.size() > 1) {
        GrammarTree* ret = registerList->subNodes[0];
        registerList = registerList->subNodes[1];
        return ret;
    }
    else if(registerList->subNodes.size() == 1) {
        GrammarTree* ret = registerList->subNodes[0];
        registerList = nullptr;
        return ret;
    }

    return nullptr;
}

static inline bool outputSTACKInstruction(GrammarTree* instruction, std::ostream& output) {
    ENFORCECON(instruction->subNodes.size() > 1, "arguments expected for instruction");
    int opcode = instruction->nodeStart->typeIndex;
    GrammarTree* operands = instruction->subNodes[1];
    uint8_t registers[4];
    int numRegisters = 0;
    uint32_t buffer[3];
    int size = 0;
    bool floatRegister = false;

    GrammarTree* registersT = getNextOperand(operands);
    if(registersT->nodeType == REGISTER_LIST) {
        int reg;

        registersT = registersT->subNodes[1];
        GrammarTree* regT = getNextRegisterFromList(registersT);

        while(regT != nullptr && (getIntRegisterOP(regT, reg) || (floatRegister = getFloatRegisterOP(regT, reg)))) {
            registers[numRegisters++] = STACK_REG(reg, floatRegister);
            regT = getNextRegisterFromList(registersT);
            floatRegister = false;
        }

        ENFORCECON(numRegisters < 5, "a register set can only contain up to 4 registers");
    }
    else {
        int reg;
        ENFORCECON(getIntRegisterOP(registersT, reg) || (floatRegister = getFloatRegisterOP(registersT, reg)), "register or register list expected");
        registers[0] = STACK_REG(reg, floatRegister);
        numRegisters = 1;
    }

    encodeSTACK(buffer, size, (STACKOps)opcode, numRegisters, registers);
    writeInstruction(buffer, size, output);

    return true;
}

static inline bool outputJMPInstruction(GrammarTree* instruction, std::ostream& output) {
    //jumps are relative, unless they are given a register as a parameter, if an expression is given, it's always relative
    ENFORCECON(instruction->subNodes.size() > 1, "arguments expected for instruction");
    int opcode = instruction->nodeStart->typeIndex;
    GrammarTree* operands = instruction->subNodes[1];
    int value;
    bool immediate = false;
    uint32_t buffer[3];
    int size = 0;

    GrammarTree* valueT = getNextOperand(operands);

    ENFORCECON(operands == nullptr && valueT != nullptr, "improper arguments");
    ENFORCECON(getIntRegisterOP(valueT, value) || (immediate = getExpressionOP(valueT, value)), "operand must be a register or immediate/expression");

    if(immediate) {
        value = value - (byteOffset + 8);
    }

    encodeJMP(buffer, size, (JMPOps)opcode, value, immediate);
    writeInstruction(buffer, size, output);

    return true;
}

static int outputInstruction(GrammarTree* tree, std::ostream& output) {
    if(tree->subNodes.size() > 0) {
        switch(tree->subNodes[0]->nodeStart->instType) {
            case (int)InstructionType::AL:
            return outputALInstruction(tree, output);
            break;
            case (int)InstructionType::SA:
            return outputSAInstruction(tree, output);
            break;
            case (int)InstructionType::FPA:
            return outputFPAInstruction(tree, output);
            break;
            case (int)InstructionType::FPS:
            return outputFPSInstruction(tree, output);
            break;
            case (int)InstructionType::MEM:
            return outputMEMInstructions(tree, output);
            break;
            case (int)InstructionType::INT:
            return outputINTInstruction(tree, output);
            break;
            case (int)InstructionType::STACK:
            return outputSTACKInstruction(tree, output);
            break;
            case (int)InstructionType::JMP:
            return outputJMPInstruction(tree, output);
            break;
            case (int)InstructionType::NO_INST:
            std::cout << "undefined instruction ? this shouldnt ever get here\n";
            return false;
        }
    }

    return true;
}

static int outputCodeList(GrammarTree* tree, std::ostream& output) {
    if(tree->subNodes.size() > 0) {
        int err = 0;

        if(tree->subNodes[0]->nodeType == LABEL) {
            err = outputLabel(tree->subNodes[0], output);
        }
        else if(tree->subNodes[0]->nodeType == INSTRUCTION) {
            err = outputInstruction(tree->subNodes[0], output);
        }

        if(err != 1) {
            return 0;
        }

        if(tree->subNodes.size() > 1) {
            return outputCodeList(tree->subNodes[1], output);
        }

        return true;
    }

    return false;
}

static int outputTextSegment(GrammarTree* tree, std::ostream& output) {
    //if its a txt segment, the first two tokens must be . text, so we start with token 2
    if(tree->subNodes.size() > 2) {
        //it has to be a data definition list
        if(tree->subNodes[2]->nodeType == CODE_LIST) {
            if(!outputCodeList(tree->subNodes[2], output)) {
                return false;
            }
        }
        else {
            return outputGlobalsegment(tree->subNodes[2], output);
        }

        if(tree->subNodes.size() > 3) {
            return outputGlobalsegment(tree->subNodes[3], output);
        }

        return true;
    }

    return tree->subNodes.size() == 2;
}

static int outputLabel(GrammarTree* tree, std::ostream& output) {
    if(pass == 0) {
        Idt identifier;
        identifier.value = byteOffset;
        identifier.label = true;
        identifier.defined = true;

        tree->subNodes[1]->nullTerminate();

        if(identifiers.find(tree->subNodes[1]->nodeStart->start) == identifiers.end()) {
            identifiers[tree->subNodes[1]->nodeEnd->start] = identifier;
            tree->subNodes[1]->unNullTerminate();
        }
        else {
            tree->subNodes[1]->unNullTerminate();
            //add error ("previously defined identifier")
            std::cout << "previously defined label\n";
            return false;
        }
    }

    return true;
}

static bool evaluateExpression(Token* start, Token* end, int& output) {
    std::vector<ExpressionTree*> tokenStream;
    createExpressionStream(start, end, tokenStream);

    ExpressionTree* exprTree = collapseExpressionList(tokenStream);

    //exprTree->printGroupings();
    if(evalExpr(exprTree)){
        output = exprTree->value;
        delete exprTree;
        return true;
    }

    delete exprTree;
    return false;
}

static int outputDataList(GrammarTree* tree, std::ostream& output, Token* type) {
    if(tree->subNodes.size() > 0) { 
        if(iequals(type->start, "const", 5)) {
            if(pass > 0) return true;

            //identifier = value, identifier = value, ...
            tree->subNodes[0]->nullTerminate();
            if(identifiers.find(tree->subNodes[0]->nodeStart->start) == identifiers.end()) {
                int value;
                if(evaluateExpression(tree->subNodes[2]->nodeStart, tree->subNodes[2]->nodeEnd, value)) {
                    Idt idt;
                    idt.defined = true;
                    idt.label = false;
                    idt.value = value;
                    identifiers[tree->subNodes[0]->nodeStart->start] = idt;
                    tree->subNodes[0]->unNullTerminate();
                }
                else {
                    tree->subNodes[0]->unNullTerminate();
                    return false;
                }
            }
            else {
                tree->subNodes[0]->unNullTerminate();
                std::cout << "duplicate variable definition\n";
                return false;
            }

            if(tree->subNodes.size() > 4) {
                return outputDataList(tree->subNodes[4], output, type);
            }
            
            return true;
        }
        else if(iequals(type->start, "asciiz", 6)) {
            writeBuffer((uint8_t*)tree->subNodes[0]->nodeStart->start + 1, (uint32_t)(tree->subNodes[0]->nodeEnd->end - tree->subNodes[0]->nodeStart->start - 2), output);
            writeByte(0, output);
        }
        else if(iequals(type->start, "ascii", 5)) {
            writeBuffer((uint8_t*)tree->subNodes[0]->nodeStart->start + 1, (uint32_t)(tree->subNodes[0]->nodeEnd->end - tree->subNodes[0]->nodeStart->start - 2), output);
        }
        else {
            int value;
            if(evaluateExpression(tree->subNodes[0]->nodeStart, tree->subNodes[0]->nodeEnd, value)) {
                if(iequals(type->start, "word", 4)) {
                    writeWord(value, output);
                }
                else if(iequals(type->start, "half", 4)) {
                    writeHalf((uint16_t)value, output);
                }
                else if(iequals(type->start, "byte", 4)) {
                    writeByte((uint8_t)value, output);
                }
                else if(iequals(type->start, "float", 4)) {
                    writeWord(*(uint32_t*)&value, output);
                }
                else if(iequals(type->start, "space", 4)) {
                    uint8_t* space = new uint8_t[value];
                    for(int i = 0; i < value / 4; i++) {
                        ((uint32_t*)space)[i] = 0;
                    }
                    for(int i = value / 4; i < value; i++) {
                        space[i] = 0;
                    }

                    writeBuffer(space, value, output);
                }
            }
            else {
                return false;
            }
        }

        //it will have the terminal comma and another element
        if(tree->subNodes.size() > 2) {
            return outputDataList(tree->subNodes[2], output, type);
        }

        return true;
    }

    return true;
}

static int outputDataDefinition(GrammarTree* tree, std::ostream& output) {
    //data definitions always start with . something, so start with token[2], but process tokens[1]
    if(tree->subNodes.size() >= 2) {
        if(tree->subNodes.size() > 2) {
            return outputDataList(tree->subNodes[2], output, tree->subNodes[1]->nodeStart);
        }
        else {
            return true;
        }
    }

    return false;
}

static int outputDataDefinitionList(GrammarTree* tree, std::ostream& output) {
    if(tree->subNodes.size() > 0) {
        int err = 0;

        if(tree->subNodes[0]->nodeType == LABEL) {
            err = outputLabel(tree->subNodes[0], output);
        }
        else if(tree->subNodes[0]->nodeType == DATA_DEFINITION) {
            err = outputDataDefinition(tree->subNodes[0], output);
        }

        if(err != 1) {
            return 0;
        }

        if(tree->subNodes.size() > 1) {
            return outputDataDefinitionList(tree->subNodes[1], output);
        }

        return true;
    }

    return false;
}

static int outputDataSegment(GrammarTree* tree, std::ostream& output) {
    //if its a data segment, the first two tokens must be . data, so we start with token 2
    if(tree->subNodes.size() > 2) {
        //it has to be a data definition list
        if(tree->subNodes[2]->nodeType == DATA_DEF_LIST) {
            if(!outputDataDefinitionList(tree->subNodes[2], output)) {
                return false;
            }
        }
        else {
            return outputGlobalsegment(tree->subNodes[2], output);
        }

        if(tree->subNodes.size() > 3) {
            return outputGlobalsegment(tree->subNodes[3], output);
        }

        return true;
    }

    return tree->subNodes.size() == 2;
}

static int outputGlobalsegment(GrammarTree* tree, std::ostream& output) {
    if(tree->subNodes.size() > 0) {
        tree = tree->subNodes[0];
    }

    if(tree->nodeType == TEXT_SEGMENT) {
        wordAllign(output);
        return outputTextSegment(tree, output);
    }
    else if(tree->nodeType == DATA_SEGMENT) {
        wordAllign(output);
        return outputDataSegment(tree, output);
    }
    
    //the global segment has to have one subnode (either a textsegment or a data segment)
    return false;
}

static int outputASMFile(GrammarTree* tree, std::ostream& output) {
    if(tree->subNodes.size() > 0) {
        //there can only be one subnode and it has to be a global segment or eof
        tree = tree->subNodes[0];
    }

    if(tree->nodeType == GLOBAL_SEGMENT) {
        return outputGlobalsegment(tree, output);
    }

    //if its anything but EOF here, there was an error

    return true;
}

int generateBinary(GrammarTree* tree, std::ostream& output) {
    byteOffset = 0;
    pass = 0;
    for(int i = 0; i < 256; i++) priority[i] = -1;

    priority['+'] = 0;
    priority['-'] = 0;
    priority['*'] = 1;
    priority['/'] = 1;

    if(tree->nodeType == FILE_START) {
        tree = tree->subNodes[0];
    }

    if(tree->nodeType == ASM_FILE) {
        int er = outputASMFile(tree, output);

        if(er == 1) {
            std::cout << "SUCCESS\n";
        }
        else {
            std::cout << "FAILED\n";
            return er;
        }
    }

    //now that we've assigned all the identifiers and evaluated all expressions, its time to output to the file
    pass = 1;
    byteOffset = 0;
    int er = outputASMFile(tree, output);

    if(er) {
        std::cout << "SUCCESS\n";
        return true;
    }
    
    std::cout << "FAILED\n";

    return false;
}

static bool evaluateInteger(Token* v, int32_t& value) {
    if(v->type == TokenType::IDENTIFIER) {
        char tmp = *v->end;
        *v->end = 0;

        if(identifiers.find(v->start) != identifiers.end()) {
            value = identifiers[v->start].value;
        }
        else {
            value = 0;
        }

        *v->end = tmp;
        return true;
    }

    int base = 10;
    bool negative = false;
    char* start = v->start;
    char* end = v->end;

    char temp = *end;
    *end = 0;

    if(*start == '-') {
        negative = true;
        start++;
    }

    if(*start == '0') {
        start++;
        if(*start == 'x') {
            base = 16;
            start++;
        }
        else if(*start == 'b') {
            base = 2;
            start++;
        }
        else if(*start == 'o') {
            base = 8;
            start++;
        }
        else {
            start--;
        }
    }

    std::string strValue = start;
    value = std::stoi(strValue, nullptr, base);

    if(negative) {
        value = value * -1;
    }

    *end = temp;
    return true;
}

static bool evalExpr(ExpressionTree* tree) {
    if(tree->v != nullptr) {
        bool ret = evaluateInteger(tree->v, tree->value);

        if(tree->negated) {
            tree->value *= -1;
        }

        return ret;
    }
    else if(tree->right == nullptr) {
        evalExpr(tree->left);
        tree->value = tree->left->value;

        if(tree->negated) {
            tree->value *= -1;
        }

        return true;
    }

    evalExpr(tree->left);
    evalExpr(tree->right);

    switch(tree->op) {
        case '-':
            tree->value = tree->left->value - tree->right->value;
        break;
        case '+':
            tree->value = tree->left->value + tree->right->value;
            break;
        case '*':
            tree->value = tree->left->value * tree->right->value;
            break;
        case '/':
            tree->value = tree->left->value / tree->right->value;
            break;
    }

    if(tree->negated) {
        tree->value *= -1;
    }

    return true;
}

//creates an expression from an individual token in the stream
static ExpressionTree* getExpFromToken(Token*& start, Token* end, bool firstElement) {
    ExpressionTree* ret = new ExpressionTree();

    if(start->type == TokenType::NUMBER || start->type == TokenType::IDENTIFIER) {
        ret->v = start;
        start++;
    }
    else if(*start->start == '(') {
        int cnt = 1;
        Token* temp = start + 1;

        while(temp < end) {
            if(*temp->start == '(') cnt++;
            if(*temp->start == ')') cnt--;

            if(cnt == 0) break;
            temp++;
        }

        std::vector<ExpressionTree*> tokens;
        delete ret;
        createExpressionStream(start + 1, temp - 1, tokens);
        ret = collapseExpressionList(tokens);

        start = temp + 1;
    }
    else if(*start->start == '-' && (start - 1)->type == TokenType::OPERATOR || *start->start == '-' && firstElement) {
        start++;
        delete ret;
        ret = getExpFromToken(start, end, false);
        ret->negated = true;
    }
    else if(start->type == TokenType::OPERATOR) {
        ret->op = *start->start;
        start++;
    }

    return ret;
}

static bool createExpressionStream(Token* start, Token* end, std::vector<ExpressionTree*>& ret) {
    bool first = true;

    while(start <= end) {
        ExpressionTree* next = getExpFromToken(start, end, first);
        first = false;
        ret.push_back(next);
    }

    return true;
}

static ExpressionTree* collapseExpressionList(std::vector<ExpressionTree*> tokens) {
    bool groupLeft = true;

    while(tokens.size() > 1) {
        if(tokens.size() > 3) {
            if(priority[tokens[3]->op] > priority[tokens[1]->op]) {
                groupLeft = false;
            }
        }

        if(groupLeft) {
            ExpressionTree* newTree = new ExpressionTree();
            newTree->left = tokens[0];
            newTree->right = tokens[2];
            newTree->op = tokens[1]->op;
            delete tokens[1];
            tokens.erase(tokens.begin());
            tokens.erase(tokens.begin());
            tokens.erase(tokens.begin());
            tokens.insert(tokens.begin(), newTree);
        }
        else {
            ExpressionTree* newTree = new ExpressionTree();
            newTree->left = tokens[2];
            newTree->right = tokens[4];
            newTree->op = tokens[3]->op;
            delete tokens[3];
            tokens.erase(tokens.begin() + 2);
            tokens.erase(tokens.begin() + 2);
            tokens.erase(tokens.begin() + 2);
            tokens.insert(tokens.begin() + 2, newTree);
            groupLeft = true;
        }
    }

    return tokens[0];
}
