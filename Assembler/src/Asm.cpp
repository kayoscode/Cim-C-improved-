
#include "Asm.h"
#include "binary.h"

#include <fstream>

#define ISIDT_START(a) ((a >= 'a' && a <= 'z') || (a >= 'A' && a <= 'Z') || a == '_')
#define ISNUM_START(a) (a >= '0' && a <= '9')
#define ISIDT_CHAR(a) (ISIDT_START(a) || (a >= '0' && a <= '9'))
#define IS_STRING(a) (a != '"')
#define ISWHITE(a) (a == ' ' || a == '\t' || a == 0 || a == '\r' || a == '\n')
#define SKIP_WHITE(file, index) while(ISWHITE(file[index])) { index ++; }
#define ISEOF(index, size) (index >= size)
#define IS_HEX_CHAR(a) ((a >= '0' && a <= '9') || (a >= 'a' && a <= 'f') || (a >= 'A' && a <= 'F'))
#define IS_OCTAL_CHAR(a) ((a >= '0' && a <= '7'))

const char* grammarNodeNames[] {
    "FILE_START", "ASM_FILE", "GLOBAL_SEGMENT", "DATA_SEGMENT", "TEXT_SEGMENT",
    "LABEL", "EXPRESSION", "DATA_DEFINITION", "DATA_DEFINITION_LIST", "DATA_LIST", 
    "INSTRUCTION", "CODE_LIST", "OPERAND_LIST", "REGISTER", "ADDRESS_OFFSET", 
    "COPY_COUNT", "OFFSET", "REGISTER_LIST", "TERMINAL",
};

bool generateTokenStream(char* file, size_t size, Token* tokenStream);
static bool isGlobalSegment(Token* tokens, uint32_t& index, GrammarTree* tree);
static bool isExpression(Token* tokens, uint32_t& index, GrammarTree* tree);

void printToken(Token& token) {
    char tmp = *(token.end);
    *(token.end) = 0;

    std::cout << TokenNames[(int)token.type] << ": " << token.start << "\n";
    *(token.end) = tmp;
}

void printTokenText(Token& token) {
    char tmp = *(token.end);
    *(token.end) = 0;

    std::cout << token.start;

    *(token.end) = tmp;
}

bool iequals(const char* a, const char* b, uint32_t count) {
    for (uint32_t i = 0; i < count; i++) {
        if (tolower(a[i]) != tolower(b[i])) {
            return false;
        }
    }

    return true;
}

static int generateParseTree(Token* tokenStream, GrammarTree* tree);
static TokenType getNextToken(char* file, size_t size, size_t& index, const char*& start, const char*& end);

void GrammarTree::print(int depth) {
    for(int i = 0; i < depth; i++){
        printf(" ");
    }

    if(!isTerminal()){
        printf("%s:", grammarNodeNames[nodeType]);
        printText();
        printf("\n");

        for(int i = 0; i < subNodes.size(); i++){
            subNodes[i]->print(depth + 1);
        }
    }
    else{
        printf("%s:", grammarNodeNames[nodeType]);
        printText();
        printf("\n");
    }
}

void GrammarTree::printText() {
    for(char* i = nodeStart->start; i < nodeEnd->end; i++) {
        if(*i == '\n') {
            std::cout << "{nl}";
            continue;
        }

        if(*i == '\r') {
            continue;
        }
        
        printf("%c", *i);
    }
}

int assembleFile(const std::string& inputFile, const std::string& outputFile) {
    FILE* file = fopen(inputFile.c_str(), "rb");
    fseek(file, 0L, SEEK_END);
    size_t fileSize = ftell(file);
    fseek(file, 0L, SEEK_SET);

    char* asmFile = new char[fileSize + 1];
    Token* tokenStream = new Token[fileSize + 1];
    fread(asmFile, sizeof(char), fileSize, file);
    fclose(file);
    asmFile[fileSize] = ' ';

    std::ofstream outputStream(outputFile, std::ios::binary);
    GrammarTree* grammarTree = new GrammarTree(FILE_START, &tokenStream[0]);

    generateTokenStream(asmFile, fileSize, tokenStream);
    generateParseTree(tokenStream, grammarTree);
    generateBinary(grammarTree, outputStream);

    //grammarTree->print();

    outputStream.close();
    delete[] asmFile;
    delete[] tokenStream;
    return 0;
}

static void loadNumericalToken(char* file, size_t size, size_t& index) {
    if(file[index] == '0') {
        index++;
        if(file[index] == 'b') {
            //load base 2
            index++;
            while(file[index] == '0' || file[index] == '1') {
                index++;
            }

            return;
        }
        else if(file[index] == 'x'){
            //load base 16
            index++;
            while(IS_HEX_CHAR(file[index])) {
                index++;
            }

            return;
        }
        else if(file[index] == 'o') {
            //load base 8
            index++;
            while(IS_OCTAL_CHAR(file[index])) {
                index++;
            }

            return;
        }
    }

    bool usedDecimal = false;

    //load base 10
    while(ISNUM_START(file[index]) || file[index] == '.' && !usedDecimal) {
        if(file[index] == '.' && !usedDecimal) {
            usedDecimal = true;
        }

        index++;
    }
}

static void loadIdentifier(char* file, size_t size, size_t& index) {
    while(ISIDT_CHAR(file[index])) {
        index++;
    }
}

static void loadString(char* file, size_t size, size_t& index) {
    while(IS_STRING(file[index])) {
        index++;
    }

    index++;
}

static bool loadKeywordFromList(char* file, size_t size, size_t index, const char** list, size_t len, int& maxLength, int& foundIndex) {
    bool ret = false;

    for(int i = 0; i < len; i++) {
        size_t length = strlen(list[i]);

        if(length > maxLength) {
            if(iequals(file + index, list[i], (uint32_t)length)) {
                maxLength = (int)length;
                foundIndex = i;
                ret = true;
            }
        }
    }

    return ret;
}

static TokenType loadKeywordToken(char* file, size_t size, size_t& index, int maxLength, int& type, int& typeIndex) {
    TokenType ret = TokenType::NO_MATCH;

    if(loadKeywordFromList(file, size, index, globalSegment, GLOBAL_SEG_SIZE, maxLength, typeIndex)) {
        ret = TokenType::GLOBAL_SEGMENT;
    }

    if(loadKeywordFromList(file, size, index, segment, SEG_SIZE, maxLength, typeIndex)) {
        ret = TokenType::SEGMENT;
    }

    if(loadKeywordFromList(file, size, index, registers, REG_SIZE, maxLength, typeIndex)) {
        ret = TokenType::REGISTER;
    }

    if(loadKeywordFromList(file, size, index, ALInstructions, AL_INST_SIZE, maxLength, typeIndex)) {
        ret = TokenType::INSTRUCTION;
        type = (int)InstructionType::AL;
    }
    else if(loadKeywordFromList(file, size, index, SAInstructions, SA_INST_SIZE, maxLength, typeIndex)) {
        ret = TokenType::INSTRUCTION;
        type = (int)InstructionType::SA;
    }
    else if(loadKeywordFromList(file, size, index, FPAInstructions, FPA_INST_SIZE, maxLength, typeIndex)) {
        ret = TokenType::INSTRUCTION;
        type = (int)InstructionType::FPA;
    }
    else if(loadKeywordFromList(file, size, index, FPSInstructions, FPS_INST_SIZE, maxLength, typeIndex)) {
        ret = TokenType::INSTRUCTION;
        type = (int)InstructionType::FPS;
    }
    else if(loadKeywordFromList(file, size, index, MEMInstructions, MEM_INST_SIZE, maxLength, typeIndex)) {
        ret = TokenType::INSTRUCTION;
        type = (int)InstructionType::MEM;
    }
    else if(loadKeywordFromList(file, size, index, INTInstructions, INT_INST_SIZE, maxLength, typeIndex)) {
        ret = TokenType::INSTRUCTION;
        type = (int)InstructionType::INT;
    }
    else if(loadKeywordFromList(file, size, index, STACKInstructions, STACK_INST_SIZE, maxLength, typeIndex)) {
        ret = TokenType::INSTRUCTION;
        type = (int)InstructionType::STACK;
    }
    else if(loadKeywordFromList(file, size, index, JMPInstructions, JMP_INST_SIZE, maxLength, typeIndex)) {
        ret = TokenType::INSTRUCTION;
        type = (int)InstructionType::JMP;
    }

    index += maxLength;
    return ret;
}

static TokenType loadOperatorPuntuator(char* file, size_t size, size_t& index, int maxLength) {
    TokenType ret = TokenType::NO_MATCH;
    int type;

    if(loadKeywordFromList(file, size, index, operators, OPERATOR_SIZE, maxLength, type)) {
        ret = TokenType::OPERATOR;
    }
    if(loadKeywordFromList(file, size, index, punctuators, PUNC_SIZE, maxLength, type)) {
        ret = TokenType::PUNCTUATOR;
    }

    index += maxLength;

    return ret;
} 

static void getNextToken(char* file, size_t size, size_t& index, Token& token) {
    SKIP_WHITE(file, index);
    if(ISEOF(index, size)) {
        token.start = file + index;
        token.end = file + index;
        token.type = TokenType::ENDOFFILE;
        return;
    }

    if(ISNUM_START(file[index])) {
        bool canProceed = false;
        if(file[index] == '-') {
            if(file[index + 1] >= '0' && file[index + 1] <= '9') {
                canProceed = true;
            }
        }
        else {
            canProceed = true;
        }

        if(canProceed) {
            token.start = file + index;
            //index += 1;
            loadNumericalToken(file, size, index);
            token.end = file + index;
            token.type = TokenType::NUMBER;
            return;
        }
    }

    if(ISIDT_START(file[index])) {
        //find the longest match to see if it's a keyword or an identifier
        size_t temp = index;
        size_t temp2 = index;
        size_t idtLength = 0;

        loadIdentifier(file, size, index);

        TokenType r = loadKeywordToken(file, (int)size, temp, (int)(index - temp - 1), token.instType, token.typeIndex);

        if(r != TokenType::NO_MATCH) {
            index = temp;
            token.start = file + temp2;
            token.end = file + index;
            token.type = r;
            return;
        }

        token.start = file + temp2;
        token.end = file + index;
        token.type = TokenType::IDENTIFIER;
        return;
    }
    else if(file[index] == '"') {
        token.start = file + index;
        index += 1;
        loadString(file, size, index);
        token.end = file + index;
        token.type = TokenType::STRING;
        return;
    }

    size_t temp = index;
    TokenType ret = loadOperatorPuntuator(file, size, index, 0);

    if(ret != TokenType::NO_MATCH) {
        token.start = file + temp;
        token.end = file + index;
        token.type = ret;
        return;
    }

    token.type = TokenType::NO_MATCH;
}

bool generateTokenStream(char* file, size_t size, Token* tokenStream) {
    size_t index = 0;
    size_t tokenIndex = 0;

    do {
        getNextToken(file, size, index, tokenStream[tokenIndex++]);

        if(tokenStream[tokenIndex - 1].type == TokenType::NO_MATCH) {
            return false;
        }
    } while(tokenStream[tokenIndex - 1].type != TokenType::ENDOFFILE);

    for(size_t i = tokenIndex; i < size + 1; i++) {
        tokenStream[i].start = file + size - 1;
        tokenStream[i].end = file + size;
        tokenStream[i].type = TokenType::ENDOFFILE;
    }
    
    return true;
}

static void addTerminalToTree(Token* tokens, uint32_t& index, GrammarTree* tree) {
    GrammarTree* newNode = new GrammarTree(TERMINAL, &tokens[index]);
    tree->addSubNode(newNode);
    index++;
}

static bool isTerminal(Token* tokens, uint32_t& index, const char* terminal, GrammarTree* tree, int len = 1) {
    if(iequals(tokens[index].start, terminal, len)) {
        GrammarTree* newNode = new GrammarTree(TERMINAL, &tokens[index]);
        tree->addSubNode(newNode);
        index++;
        return true;
    }

    return false;
}

static bool isEOF(Token* tokens, uint32_t& index, GrammarTree* tree) {
    if(tokens[index].type == TokenType::ENDOFFILE) {
        GrammarTree* newNode = new GrammarTree(TERMINAL, &tokens[index]);
        tree->addSubNode(newNode);
        index++;
        return true;
    }

    return false;
}

static bool isLabel(Token* tokens, uint32_t& index, GrammarTree* tree) {
    uint32_t temp = index;
    GrammarTree* newNode = new GrammarTree(LABEL, &tokens[index]);

    if(tokens[index].type == TokenType::SEGMENT) {
        if(isTerminal(tokens, index, "label", newNode, 5)) {
            if(tokens[index].type == TokenType::IDENTIFIER) {
                addTerminalToTree(tokens, index, newNode);

                if(isTerminal(tokens, index, ":", newNode)) {
                    tree->addSubNode(newNode);
                    return true;
                }
            }
        }
    }

    index = temp;
    delete newNode;
    return false;
}

static void skipComma(Token* tokens, uint32_t& index, GrammarTree* tree) {
    GrammarTree throwAway(0, &tokens[index]);
    isTerminal(tokens, index, ",", &throwAway);
}

static bool isOffset(Token* tokens, uint32_t& index, GrammarTree* tree) {
    uint32_t temp = index;
    GrammarTree* newNode = new GrammarTree(OFFSET, &tokens[index]);

    if(isTerminal(tokens, index, "[", newNode)) {
        if(isExpression(tokens, index, newNode)) {
            if(isTerminal(tokens, index, "]", newNode)) {
                tree->addSubNode(newNode);
                return true;
            }
        }
    }

    index = temp;
    delete newNode;
    return false;
}

static bool isAddressCount(Token* tokens, uint32_t& index, GrammarTree* tree) {
    uint32_t temp = index;
    GrammarTree* newNode = new GrammarTree(COPY_COUNT, &tokens[index]);

    if(isTerminal(tokens, index, ":", newNode)) {
        if(isExpression(tokens, index, newNode)) {
            tree->addSubNode(newNode);
            return true;
        }
    }

    index = temp;
    delete newNode;
    return false;
}

static bool isAddressOffset(Token* tokens, uint32_t& index, GrammarTree* tree) {
    uint32_t temp = index;
    GrammarTree* newNode = new GrammarTree(ADDRESS_OFFSET, &tokens[index]);

    if(tokens[index].type == TokenType::REGISTER){
        addTerminalToTree(tokens, index, newNode);

        if(isOffset(tokens, index, newNode)) {
            isAddressCount(tokens, index, newNode);
            tree->addSubNode(newNode);
            return true;
        }

        if(isAddressCount(tokens, index, newNode)) {
            tree->addSubNode(newNode);
            return true;
        }
    }

    index = temp;
    delete newNode;
    return false;
}

static bool isRegisterList(Token* tokens, uint32_t& index, GrammarTree* tree) {
    uint32_t temp = index;
    GrammarTree* newNode = new GrammarTree(REGISTER_LIST, &tokens[index]);

    if(tokens[index].type == TokenType::REGISTER) {
        addTerminalToTree(tokens, index, newNode);
        skipComma(tokens, index, newNode);
        isRegisterList(tokens, index, newNode);
        tree->addSubNode(newNode);
        return true;
    }

    index = temp;
    delete newNode;
    return false;
}

static bool isRegisterSet(Token* tokens, uint32_t& index, GrammarTree* tree) {
    uint32_t temp = index;
    GrammarTree* newNode = new GrammarTree(REGISTER_LIST, &tokens[index]);

    if(isTerminal(tokens, index, "[", newNode)) {
        if(isRegisterList(tokens, index, newNode)) {
            if(isTerminal(tokens, index, "]", newNode)) {
                tree->addSubNode(newNode);
                return true;
            }
        }
    }

    index = temp;
    delete newNode;
    return false;
}

static bool isOperandList(Token* tokens, uint32_t& index, GrammarTree* tree) {
    uint32_t temp = index;
    GrammarTree* newNode = new GrammarTree(OPERAND_LIST, &tokens[index]);

    bool validOperandList = false;

    if(isRegisterSet(tokens, index, newNode)) {
        validOperandList = true;
    }
    else if(isAddressOffset(tokens, index, newNode)) {
        validOperandList = true;
    }
    else if(tokens[index].type == TokenType::REGISTER){
        addTerminalToTree(tokens, index, newNode);
        validOperandList = true;
    }
    else if(isExpression(tokens, index, newNode)) {
        validOperandList = true;
    }

    if(validOperandList) {
        skipComma(tokens, index, newNode);
        isOperandList(tokens, index, newNode);
        tree->addSubNode(newNode);
        return true;
    }
    
    index = temp;
    delete newNode;
    return true;
}

static bool isInstruction(Token* tokens, uint32_t& index, GrammarTree* tree) {
    uint32_t temp = index;
    GrammarTree* newNode = new GrammarTree(INSTRUCTION, &tokens[index]);

    if(tokens[index].type == TokenType::INSTRUCTION) {
        addTerminalToTree(tokens, index, newNode);

        if(isOperandList(tokens, index, newNode)) {
            tree->addSubNode(newNode);
            return true;
        }
    }

    index = temp;
    delete newNode;
    return false;
}

static bool isCodeList(Token* tokens, uint32_t& index, GrammarTree* tree) {
    uint32_t temp = index;
    GrammarTree* newNode = new GrammarTree(CODE_LIST, &tokens[index]);

    if(isLabel(tokens, index, newNode) || isInstruction(tokens, index, newNode)) {
        isCodeList(tokens, index, newNode);
        tree->addSubNode(newNode);
        return true;
    }

    index = temp;
    delete newNode;
    return false;
}

static bool isTextSegment(Token* tokens, uint32_t& index, GrammarTree* tree) {
    uint32_t temp = index;
    GrammarTree* newNode = new GrammarTree(TEXT_SEGMENT, &tokens[index]);

    if(isTerminal(tokens, index, ".", newNode)) {
        if(isTerminal(tokens, index, "text", newNode, 4)) {
            isCodeList(tokens, index, newNode);
            isGlobalSegment(tokens, index, newNode);
            tree->addSubNode(newNode);
            return true;
        }
    }

    index = temp;
    delete newNode;
    return false;
}

static bool isExpression(Token* tokens, uint32_t& index, GrammarTree* tree) {
    uint32_t temp = index;
    GrammarTree* newNode = new GrammarTree(EXPRESSION, &tokens[index]);
    bool valid = false;

    if(isTerminal(tokens, index, "-", newNode)) {
        if(isExpression(tokens, index, newNode)) {
            valid = true;
        }
    }
    else if(isTerminal(tokens, index, "(", newNode)) {
        if(isExpression(tokens, index, newNode)) {
            if(isTerminal(tokens, index, ")", newNode)) {
                valid = true;
            }
        }
    }
    else if(tokens[index].type == TokenType::NUMBER || tokens[index].type == TokenType::STRING || tokens[index].type == TokenType::IDENTIFIER) {
        addTerminalToTree(tokens, index, newNode);
        valid = true;
    }

    if(valid) {
        if(tokens[index].type == TokenType::OPERATOR) {
            addTerminalToTree(tokens, index, newNode);

            if(isExpression(tokens, index, newNode)) {
                tree->addSubNode(newNode);
                return true;
            }
            else {
                index = temp;
                delete newNode;
                return false;
            }
        }

        tree->addSubNode(newNode);
        return true;
    }

    index = temp;
    delete newNode;
    return false;
}

static bool isConstDataList(Token* tokens, uint32_t& index, GrammarTree* tree) {
    uint32_t temp = index;
    GrammarTree* newNode = new GrammarTree(DATA_LIST, &tokens[index]);

    if(tokens[index].type == TokenType::IDENTIFIER) { 
        addTerminalToTree(tokens, index, newNode);
        if(isTerminal(tokens, index, "=", newNode)) {
            if(isExpression(tokens, index, newNode)) { 
                if(isTerminal(tokens, index, ",", newNode)) {
                    isConstDataList(tokens, index, newNode);
                }

                tree->addSubNode(newNode);
                return true;
            }
        }
    }

    index = temp;
    delete newNode;
    return false;
}

static bool isDataList(Token* tokens, uint32_t& index, GrammarTree* tree) {
    uint32_t temp = index;
    GrammarTree* newNode = new GrammarTree(DATA_LIST, &tokens[index]);

    if(isExpression(tokens, index, newNode)) { 
        if(isTerminal(tokens, index, ",", newNode)) {
            isDataList(tokens, index, newNode);
        }

        tree->addSubNode(newNode);
        return true;
    }

    index = temp;
    delete newNode;
    return false;
}

static bool isDataDefinition(Token* tokens, uint32_t& index, GrammarTree* tree) {
    uint32_t temp = index;
    GrammarTree* newNode = new GrammarTree(DATA_DEFINITION, &tokens[index]);

    if(isTerminal(tokens, index, ".", newNode)) {
        if(tokens[index].type == TokenType::SEGMENT) {
            if(isTerminal(tokens, index, "const", newNode, 5)) {
                isConstDataList(tokens, index, newNode);
                tree->addSubNode(newNode);
                return true;
            }
            else {
                addTerminalToTree(tokens, index, newNode);
                isDataList(tokens, index, newNode);
                tree->addSubNode(newNode);
                return true;
            }
        }
    }

    index = temp;
    delete newNode;
    return false;
}

static bool isDataDefinitionList(Token* tokens, uint32_t& index, GrammarTree* tree) {
    uint32_t temp = index;
    GrammarTree* newNode = new GrammarTree(DATA_DEF_LIST, &tokens[index]);

    if(isLabel(tokens, index, newNode) || isDataDefinition(tokens, index, newNode)) {
        isDataDefinitionList(tokens, index, newNode);
        tree->addSubNode(newNode);
        return true;
    }

    index = temp;
    delete newNode;
    return false;
}

static bool isDataSegment(Token* tokens, uint32_t& index, GrammarTree* tree) {
    uint32_t temp = index;
    GrammarTree* newNode = new GrammarTree(DATA_SEGMENT, &tokens[index]);

    if(isTerminal(tokens, index, ".", newNode)) {
        if(isTerminal(tokens, index, "data", newNode, 4)) {
            isDataDefinitionList(tokens, index, newNode);
            isGlobalSegment(tokens, index, newNode);
            tree->addSubNode(newNode);
            return true;
        }
    }

    index = temp;
    delete newNode;
    return false;
}

static bool isGlobalSegment(Token* tokens, uint32_t& index, GrammarTree* tree) {
    uint32_t temp = index;
    GrammarTree* newNode = new GrammarTree(GLOBAL_SEGMENT, &tokens[index]);

    if(isDataSegment(tokens, index, newNode) || isTextSegment(tokens, index, newNode)) {
        tree->addSubNode(newNode);
        return true;
    }

    index = temp;
    delete newNode;
    return false;
}

static bool isAsmFile(Token* tokens, uint32_t& index, GrammarTree* tree) {
    uint32_t temp = index;
    GrammarTree* newNode = new GrammarTree(ASM_FILE, &tokens[index]);

    if(isGlobalSegment(tokens, index, newNode)) {
        tree->addSubNode(newNode);
        return true;
    }
    else if(isEOF(tokens, index, newNode)) {
        tree->addSubNode(newNode);
        return true;
    }

    delete newNode;
    index = temp;
    return false;
}

static int generateParseTree(Token* tokens, GrammarTree* tree) {
    uint32_t tokenIndex = 0;

    if(isAsmFile(tokens, tokenIndex, tree)) {
        std::cout << "true\n";
        return true;
    }

    std::cout << "false\n";
    return false;
}
