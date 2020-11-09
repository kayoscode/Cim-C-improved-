#include "Compile.h"

/**
 * TODO: 
 * 2. breaks & continues
 * 3. constructors
 * */

int line = 0;

#define ISIDT_START(a) ((a >= 'a' && a <= 'z') || (a >= 'A' && a <= 'Z') || a == '_')
#define ISNUM_START(a) (a >= '0' && a <= '9')
#define ISIDT_CHAR(a) (ISIDT_START(a) || (a >= '0' && a <= '9'))
#define IS_STRING(a) (a != '"')
#define ISWHITE(a) (a == ' ' || a == '\t' || a == 0 || a == '\r' || a == '\n')
#define SKIP_WHITE(file, index) while(ISWHITE(file[index])) { line += file[index] == '\n'; index ++; }
#define ISEOF(index, size) (index >= size)
#define IS_HEX_CHAR(a) ((a >= '0' && a <= '9') || (a >= 'a' && a <= 'f') || (a >= 'A' && a <= 'F'))
#define IS_OCTAL_CHAR(a) ((a >= '0' && a <= '7'))

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

const char* grammarNodeNames[] {
    "FILE_START", "LANGUAGE_FILE", "GLOBAL", "GLOBAL_LIST", "DECLARATION", "INTIALIZATION", "POINTER_TYPE",
    "ASSIGNMENT", "EXPRESSION", "MODIFIER", "MODIFIER_LIST", "TYPE", "FUNCTION_DECLARATION", "FUNCTION_DEC_OPERAND", "FUNCTION_OPERANDS",
    "FUNCTION", "FUNCTION_DEC_ARG_LIST", "STATEMENT_LIST", "CODE_BLOCK",
    "STATEMENT", "SWITCH_STATEMENT", "CASE", "CASE_LIST", "ARRAY_TYPE", "FUNCTION_PTR_TYPE",
    "IF_STATEMENT", "ELSE_IF_STATEMENT", "ELSE_STATEMENT",
    "TYPEDEF", "WHILE_LOOP", "DO_WHILE_LOOP", "FOR_LOOP", "FUNCTION_CALL_ARGS",
    "SUB_STATEMENT", "FUNCTION_CALL", "LOOP", "RETURN_STATEMENT", "TERMINAL", "LVALUE", "RVALUE", "PREFIX_OPERATOR",
    "STRUCTURE_DEFINITION", "STRUCTURE", "STRUCT", "CLASS"
};

std::unordered_map<const char*, int> defaultTypes {
    {"float", {FLOAT_SIZE}}, {"long", {LONG_SIZE}}, {"int", {INT_SIZE}},
    {"short", {SHORT_SIZE}}, {"char", {CHAR_SIZE}},
    {"register", {REGISTER_SIZE}}, {"void", {VOID_SIZE}}, {"bool", {BOOL_SIZE}}
};

static bool loadKeywordFromList(char* file, size_t size, size_t index, const char** list, size_t len, int& maxLength, int& foundIndex);
static bool isStatementList(Token* tokens, uint32_t& index, GrammarTree* tree);
static bool isStatement(Token* tokens, uint32_t& index, GrammarTree* tree);
static bool isCodeBlock(Token* tokens, uint32_t& index, GrammarTree* tree);
static bool isCase(Token* tokens, uint32_t& index, GrammarTree* tree);
static bool isModifierList(Token* tokens, uint32_t& index, GrammarTree* tree);
static bool isFunctionDecOperands(Token* tokens, uint32_t& index, GrammarTree* tree);
static bool isLValue(Token* tokens, uint32_t& index, GrammarTree* tree);
static bool isExpression(Token* tokens, uint32_t& index, GrammarTree* tree);
static bool isFunctionOperands(Token* tokens, uint32_t& index, GrammarTree* tree);
static bool isStructure(Token* tokens, uint32_t& index, GrammarTree* tree);

void printToken(Token& token) {
    char tmp = *(token.end);
    *(token.end) = 0;

    std::cout << TokenNames[(int)token.type] << ": " << token.start << ": " << token.line << "\n";
    *(token.end) = tmp;
}

void printTokenText(Token& token) {
    char tmp = *(token.end);
    *(token.end) = 0;

    std::cout << token.start;

    *(token.end) = tmp;
}

bool sequals(const char* a, const char* b, uint32_t count) {
    for (uint32_t i = 0; i < count; i++) {
        if (tolower(a[i]) != tolower(b[i])) {
            return false;
        }
    }

    return true;
}

static TokenTypes loadKeywordToken(char* file, size_t size, size_t& index, int maxLength, int& subType) {
    TokenTypes ret = TokenTypes::NO_MATCH;

    if(loadKeywordFromList(file, size, index, types, (int)Types::TYPES_COUNT, maxLength, subType)) {
        ret = TokenTypes::TYPE;
    }
    else if(loadKeywordFromList(file, size, index, modifiers, (int)Modifiers::MODIFIERS_COUNT, maxLength, subType)) {
        ret = TokenTypes::MODIFIER;
    }
    else if(loadKeywordFromList(file, size, index, constructs, (int)Constructs::CONSTRUCTS_COUNT, maxLength, subType)) {
        ret = TokenTypes::CONSTRUCT;
    }

    index += maxLength;
    return ret;
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

static void loadString(char* file, size_t size, size_t& index) {
    while(IS_STRING(file[index])) {
        index++;
    }

    index++;
}

static void loadIdentifier(char* file, size_t size, size_t& index) {
    while(ISIDT_CHAR(file[index])) {
        index++;
    }
}

static bool loadKeywordFromList(char* file, size_t size, size_t index, const char** list, size_t len, int& maxLength, int& foundIndex) {
    bool ret = false;

    for(int i = 0; i < len; i++) {
        size_t length = strlen(list[i]);

        if(length > maxLength) {
            if(sequals(file + index, list[i], (uint32_t)length)) {
                maxLength = (int)length;
                foundIndex = i;
                ret = true;
            }
        }
    }

    return ret;
}

static TokenTypes loadOperatorPuntuator(char* file, size_t size, size_t& index, int maxLength, int& type) {
    TokenTypes ret = TokenTypes::NO_MATCH;

    if(loadKeywordFromList(file, size, index, operators, (int)Operators::OPERATORS_COUNT, maxLength, type)) {
        ret = TokenTypes::OPERATOR;
    }
    if(loadKeywordFromList(file, size, index, punctuators, (int)Punctuators::PUNCTUATORS_COUNT, maxLength, type)) {
        ret = TokenTypes::PUNCTUATOR;
    }

    index += maxLength;

    return ret;
} 

static void getNextToken(char* file, size_t size, size_t& index, Token& token) {
    if(index < size) {
        SKIP_WHITE(file, index);
    }

    if(ISEOF(index, size)) {
        token.start = file + index;
        token.end = file + index;
        token.type = (int)TokenTypes::ENDOFFILE;
        return;
    }

    SKIP_WHITE(file, index);

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
            token.type = (int)TokenTypes::NUMBER;
            return;
        }
    }

    if(ISIDT_START(file[index])) {
        //find the longest match to see if it's a keyword or an identifier
        size_t temp = index;
        size_t temp2 = index;
        size_t idtLength = 0;

        loadIdentifier(file, size, index);

        TokenTypes r = loadKeywordToken(file, (int)size, temp, (int)(index - temp - 1), token.subType);

        if(r != TokenTypes::NO_MATCH) {
            index = temp;
            token.start = file + temp2;
            token.end = file + index;
            token.type = (int)r;
            return;
        }

        token.start = file + temp2;
        token.end = file + index;
        token.type = (int)TokenTypes::IDENTIFIER;
        return;
    }
    else if(file[index] == '"') {
        token.start = file + index;
        index += 1;
        loadString(file, size, index);
        token.end = file + index;
        token.type = (int)TokenTypes::STRING;
        return;
    }

    size_t temp = index;
    int subType;
    TokenTypes ret = loadOperatorPuntuator(file, size, index, 0, subType);

    if(ret != TokenTypes::NO_MATCH) {
        token.start = file + temp;
        token.end = file + index;
        token.type = (int)ret;
        token.subType = subType;
        return;
    }

    token.type = (int)TokenTypes::NO_MATCH;
}

bool generateTokenStream(char* file, size_t size, Token* tokenStream) {
    size_t index = 0;
    size_t tokenIndex = 0;

    do {
        getNextToken(file, size, index, tokenStream[tokenIndex++]);
        tokenStream[tokenIndex - 1].line = line;

        if(tokenStream[tokenIndex - 1].type == (int)TokenTypes::NO_MATCH) {
            return false;
        }
    } while(tokenStream[tokenIndex - 1].type != (int)TokenTypes::ENDOFFILE);

    for(size_t i = tokenIndex; i < size + 1; i++) {
        tokenStream[i].start = file + size - 1;
        tokenStream[i].end = file + size;
        tokenStream[i].type = (int)TokenTypes::ENDOFFILE;
    }
    
    return true;
}

static bool isEOF(Token* tokens, uint32_t& index, GrammarTree* tree) {
    if(tokens[index].type == (int)TokenTypes::ENDOFFILE) {
        GrammarTree* newNode = new GrammarTree(TERMINAL, &tokens[index]);
        tree->addSubNode(newNode);
        index++;
        return true;
    }

    return false;
}

static void addTerminalToTree(Token* tokens, uint32_t& index, GrammarTree* tree) {
    GrammarTree* newNode = new GrammarTree(TERMINAL, &tokens[index]);
    tree->addSubNode(newNode);
    index++;
}

static bool isTerminal(Token* tokens, uint32_t& index, const char* terminal, GrammarTree* tree, int len = 1) {
    if(sequals(tokens[index].start, terminal, len)) {
        GrammarTree* newNode = new GrammarTree(TERMINAL, &tokens[index]);
        tree->addSubNode(newNode);
        index++;
        return true;
    }

    return false;
}

static bool isPointerType(Token* tokens, uint32_t& index, GrammarTree* tree) {
    uint32_t temp = index;
    GrammarTree* newNode = new GrammarTree(POINTER_TYPE, &tokens[index]);

    if(tokens[index].type == (int)TokenTypes::OPERATOR && tokens[index].subType == (int)Operators::MUL_DERF) {
        addTerminalToTree(tokens, index, newNode);
        isPointerType(tokens, index, newNode);
        tree->addSubNode(newNode);
        return true;
    }

    delete newNode;
    index = temp;
    return false;
}

static bool isArrayType(Token* tokens, uint32_t& index, GrammarTree* tree) {
    uint32_t temp = index;
    GrammarTree* newNode = new GrammarTree(ARRAY_TYPE, &tokens[index]);

    if(tokens[index].type == (int)TokenTypes::PUNCTUATOR && tokens[index].subType == (int)Punctuators::OBRACK) {
        addTerminalToTree(tokens, index, newNode);
        isExpression(tokens, index, newNode);
        if(tokens[index].type == (int)TokenTypes::PUNCTUATOR && tokens[index].subType == (int)Punctuators::CBRACK) {
            addTerminalToTree(tokens, index, newNode);
            isArrayType(tokens, index, newNode);
            tree->addSubNode(newNode);
            return true;
        }
    }

    delete newNode;
    index = temp;
    return false;
}

static bool isFunctionPointerType(Token* tokens, uint32_t& index, GrammarTree* tree) {
    uint32_t temp = index;
    GrammarTree* newNode = new GrammarTree(FUNCTION_PTR_TYPE, &tokens[index]);

    if(isFunctionDecOperands(tokens, index, newNode)) {
        isFunctionPointerType(tokens, index, newNode);
        tree->addSubNode(newNode);
        return true;
    }

    delete newNode;
    index = temp;
    return false;
}

static bool isType(Token* tokens, uint32_t& index, GrammarTree* tree) {
    uint32_t temp = index;
    GrammarTree* newNode = new GrammarTree(TYPE, &tokens[index]);

    //it actually doesn't need to have anything here, int is assumed
    if(isModifierList(tokens, index, newNode)) {
        //check to see if it's a valid type
        if(tokens[index].type == (int)TokenTypes::TYPE || tokens[index].type == (int)TokenTypes::IDENTIFIER) {
            addTerminalToTree(tokens, index, newNode);
            isPointerType(tokens, index, newNode);

            //adding a function argument list transforms it into a function pointer with the previous types as the return type: ie
            isFunctionPointerType(tokens, index, newNode);
            isArrayType(tokens, index, newNode);

            tree->addSubNode(newNode);
            return true;
        }
    }

    delete newNode;
    index = temp;
    return false;
}

static bool isModifier(Token* tokens, uint32_t& index, GrammarTree* tree) {
    uint32_t temp = index;
    GrammarTree* newNode = new GrammarTree(MODIFIER, &tokens[index]);

    if(tokens[index].type == (int)TokenTypes::MODIFIER) {
        addTerminalToTree(tokens, index, newNode);
        tree->addSubNode(newNode);
        return true;
    }

    delete newNode;
    index = temp;
    return false;
}

static bool isModifierList(Token* tokens, uint32_t& index, GrammarTree* tree) {
    uint32_t temp = index;
    GrammarTree* newNode = new GrammarTree(MODIFIER_LIST, &tokens[index]);

    if(isModifier(tokens, index, newNode)) {
        isModifierList(tokens, index, newNode);
        tree->addSubNode(newNode);
    }

    return true;
}

static bool isFunctionCallArg(Token* tokens, uint32_t& index, GrammarTree* tree) {
    uint32_t temp = index;
    GrammarTree* newNode = new GrammarTree(FUNCTION_CALL_ARGS, &tokens[index]);

    if(isExpression(tokens, index, newNode)) {
        tree->addSubNode(newNode);
        return true;
    }

    delete newNode;
    index = temp;
    return false;
}

static bool isFunctionCallArgList(Token* tokens, uint32_t& index, GrammarTree* tree) {
    uint32_t temp = index;
    GrammarTree* newNode = new GrammarTree(FUNCTION_CALL_ARGS, &tokens[index]);

    if(isFunctionCallArg(tokens, index, newNode)) {
        if(tokens[index].type == (int)TokenTypes::PUNCTUATOR && tokens[index].subType == (int)Punctuators::COMMA) {
            addTerminalToTree(tokens, index, tree);
            if(isFunctionCallArgList(tokens, index, newNode)) {
                tree->addSubNode(newNode);
                return true;
            }
        }
        else {
            tree->addSubNode(newNode);
            return true;
        }
    }

    delete newNode;
    index = temp;
    return false;
}

static bool isFunctionCallArgs(Token* tokens, uint32_t& index, GrammarTree* tree) {
    uint32_t temp = index;
    GrammarTree* newNode = new GrammarTree(FUNCTION_CALL_ARGS, &tokens[index]);

    if(tokens[index].type == (int)TokenTypes::PUNCTUATOR && tokens[index].subType == (int)Punctuators::OPARN) {
        addTerminalToTree(tokens, index, newNode);
        isFunctionCallArgList(tokens, index, newNode);
        if(tokens[index].type == (int)TokenTypes::PUNCTUATOR && tokens[index].subType == (int)Punctuators::CPARN) {
            addTerminalToTree(tokens, index, newNode);
            tree->addSubNode(newNode);
            return true;
        }
    }

    delete newNode;
    index = temp;
    return false;
}

static bool isFunctionCall(Token* tokens, uint32_t& index, GrammarTree* tree) {
    uint32_t temp = index;
    GrammarTree* newNode = new GrammarTree(FUNCTION_CALL, &tokens[index]);

    if(isLValue(tokens, index, newNode) && isFunctionCallArgs(tokens, index, newNode)) {
        tree->addSubNode(newNode);
        return true;
    }

    delete newNode;
    index = temp;
    return false;
}

static bool isPrefixOperator(Token* tokens, uint32_t& index, GrammarTree* tree) {
    uint32_t temp = index;
    GrammarTree* newNode = new GrammarTree(PREFIX_OPERATOR, &tokens[index]);

    if(tokens[index].type == (int)TokenTypes::OPERATOR) {
        if(tokens[index].subType == (int)Operators::DECREMENT || tokens[index].subType == (int)Operators::INCREMENT || 
            tokens[index].subType == (int)Operators::MINUS || tokens[index].subType == (int)Operators::AND_ADDR || 
            tokens[index].subType == (int)Operators::MUL_DERF || tokens[index].subType == (int)Operators::NOT) 
        {
            addTerminalToTree(tokens, index, newNode);
            tree->addSubNode(newNode);
            return true;
        }
    }

    delete newNode;
    index = temp;
    return false;
}

static bool isPostfixOperator(Token* tokens, uint32_t& index, GrammarTree* tree) {
    uint32_t temp = index;
    GrammarTree* newNode = new GrammarTree(PREFIX_OPERATOR, &tokens[index]);

    if(tokens[index].type == (int)TokenTypes::OPERATOR) {
        if(tokens[index].subType == (int)Operators::DECREMENT || tokens[index].subType == (int)Operators::INCREMENT) {
            addTerminalToTree(tokens, index, newNode);
            tree->addSubNode(newNode);
            return true;
        }
    }

    delete newNode;
    index = temp;
    return false;
}

static bool isExpression(Token* tokens, uint32_t& index, GrammarTree* tree) {
    uint32_t temp = index;
    GrammarTree* newNode = new GrammarTree(EXPRESSION, &tokens[index]);
    bool valid = false;

    if(isPrefixOperator(tokens, index, newNode)) {
        isExpression(tokens, index, newNode);
        valid = true;
    }
    else if(tokens[index].type == (int)TokenTypes::PUNCTUATOR && tokens[index].subType == (int)Punctuators::OPARN) {
        addTerminalToTree(tokens, index, newNode);
        if(isExpression(tokens, index, newNode)) {
            if(tokens[index].type == (int)TokenTypes::PUNCTUATOR && tokens[index].subType == (int)Punctuators::CPARN) {
                addTerminalToTree(tokens, index, newNode);
                valid = true;
            }
        }
    }
    else if(isFunctionCall(tokens, index, newNode)) {
        valid = true;
    }
    else if(tokens[index].type == (int)TokenTypes::NUMBER || tokens[index].type == (int)TokenTypes::STRING || tokens[index].type == (int)TokenTypes::IDENTIFIER) {
        addTerminalToTree(tokens, index, newNode);
        valid = true;
    }

    if(valid) {
        isPostfixOperator(tokens, index, newNode);

        if(tokens[index].type == (int)TokenTypes::OPERATOR) {
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

    delete newNode;
    index = temp;
    return false;
}

static bool isLValue(Token* tokens, uint32_t& index, GrammarTree* tree) {
    uint32_t temp = index;
    GrammarTree* newNode = new GrammarTree(LVALUE, &tokens[index]);

    if(tokens[index].type == (int)TokenTypes::PUNCTUATOR && tokens[index].type == (int)Punctuators::OPARN) {
        if(isLValue(tokens, index, newNode)) {
            if(tokens[index].type == (int)TokenTypes::PUNCTUATOR && tokens[index].type == (int)Punctuators::CPARN) {
                return true;
            }
        }
    }
    else if(tokens[index].type == (int)TokenTypes::IDENTIFIER) {
        addTerminalToTree(tokens, index, newNode);
        tree->addSubNode(newNode);
        return true;
    }

    delete newNode;
    index = temp;
    return false;
}

static bool isInitialization(Token* tokens, uint32_t& index, GrammarTree* tree) {
    uint32_t temp = index;
    GrammarTree* newNode = new GrammarTree(INTIALIZATION, &tokens[index]);

    if(isType(tokens, index, newNode)) {
        if(isLValue(tokens, index, newNode)) {
            if(tokens[index].type == (int)TokenTypes::OPERATOR && tokens[index].subType == (int)Operators::EQ) {
                addTerminalToTree(tokens, index, newNode);
                if(isExpression(tokens, index, newNode)) {
                    tree->addSubNode(newNode);
                    return true;
                }
            }
        }
    }

    delete newNode;
    index = temp;
    return false;
}

static bool isDeclaration(Token* tokens, uint32_t& index, GrammarTree* tree) {
    uint32_t temp = index;
    GrammarTree* newNode = new GrammarTree(DECLARATION, &tokens[index]);

    //check for variable declaration
    if(isType(tokens, index, newNode)) {
        if(tokens[index].type == (int)TokenTypes::IDENTIFIER) {
            addTerminalToTree(tokens, index, newNode);
            tree->addSubNode(newNode);
            return true;
        }
    }

    delete newNode;
    index = temp;
    return false;
}

static bool isTypedef(Token* tokens, uint32_t& index, GrammarTree* tree) {
    uint32_t temp = index;
    GrammarTree* newNode = new GrammarTree(TYPEDEF, &tokens[index]);

    if(tokens[index].type == (int)TokenTypes::CONSTRUCT && tokens[index].subType == (int)Constructs::TYPEDEF) {
        addTerminalToTree(tokens, index, newNode);
        if(tokens[index].type == (int)TokenTypes::IDENTIFIER) {
            addTerminalToTree(tokens, index, newNode);
            if(isType(tokens, index, newNode)) {
                tree->addSubNode(newNode);
                return true;
            }
        }
    }

    delete newNode;
    index = temp;
    return false;
}

static bool isFunctionDecOperand(Token* tokens, uint32_t& index, GrammarTree* tree) {
    uint32_t temp = index;
    GrammarTree* newNode = new GrammarTree(FUNCTION_DEC_OPERAND, &tokens[index]);

    if(isType(tokens, index, newNode)) {
        if(tokens[index].type == (int)TokenTypes::IDENTIFIER) {
            addTerminalToTree(tokens, index, newNode);
        }

        tree->addSubNode(newNode);
        return true;
    }

    delete newNode;
    index = temp;
    return false;
}

static bool isFunctionDecOperandList(Token* tokens, uint32_t& index, GrammarTree* tree) {
    uint32_t temp = index;
    GrammarTree* newNode = new GrammarTree(FUNCTION_DEC_ARG_LIST, &tokens[index]);

    if(isFunctionDecOperand(tokens, index, newNode)) {
        if(tokens[index].type == (int)TokenTypes::PUNCTUATOR && tokens[index].subType == (int)Punctuators::COMMA) {
            addTerminalToTree(tokens, index, newNode);
            if(isFunctionDecOperandList(tokens, index, newNode)) {
                tree->addSubNode(newNode);
                return true;
            }
            else {
                delete newNode;
                index = temp;
                return false;
            }
        }

        tree->addSubNode(newNode);
        return true;
    }

    delete newNode;
    index = temp;
    return false;
}

static bool isFunctionDecOperands(Token* tokens, uint32_t& index, GrammarTree* tree) {
    uint32_t temp = index;
    GrammarTree* newNode = new GrammarTree(FUNCTION_DEC_ARG_LIST, &tokens[index]);

    if(tokens[index].type == (int)TokenTypes::PUNCTUATOR && tokens[index].subType == (int)Punctuators::OPARN) {
        addTerminalToTree(tokens, index, newNode);
        isFunctionDecOperandList(tokens, index, newNode);
        if(tokens[index].type == (int)TokenTypes::PUNCTUATOR && tokens[index].subType == (int)Punctuators::CPARN) {
            addTerminalToTree(tokens, index, newNode);
            tree->addSubNode(newNode);
            return true;
        }
    }

    delete newNode;
    index = temp;
    return false;
}

static bool isFunctionDeclaration(Token* tokens, uint32_t& index, GrammarTree* tree) {
    //depricated
    return false;

    uint32_t temp = index;
    GrammarTree* newNode = new GrammarTree(FUNCTION_DECLARATION, &tokens[index]);

    if(isType(tokens, index, newNode)) {
        if(tokens[index].type == (int)TokenTypes::IDENTIFIER) {
            addTerminalToTree(tokens, index, newNode);
            if(isFunctionDecOperands(tokens, index, newNode)) {
                tree->addSubNode(newNode);
                return true;
            }
        }
    }

    delete newNode;
    index = temp;
    return false;
}

static bool isFunctionOperand(Token* tokens, uint32_t& index, GrammarTree* tree) {
    uint32_t temp = index;
    GrammarTree* newNode = new GrammarTree(FUNCTION_DEC_OPERAND, &tokens[index]);

    if(isType(tokens, index, newNode)) {
        if(tokens[index].type == (int)TokenTypes::IDENTIFIER) {
            addTerminalToTree(tokens, index, newNode);
            tree->addSubNode(newNode);
            return true;
        }
    }

    delete newNode;
    index = temp;
    return false;
}

static bool isFunctionOperandList(Token* tokens, uint32_t& index, GrammarTree* tree) {
    uint32_t temp = index;
    GrammarTree* newNode = new GrammarTree(FUNCTION_DEC_ARG_LIST, &tokens[index]);

    if(isFunctionOperand(tokens, index, newNode)) {
        if(tokens[index].type == (int)TokenTypes::PUNCTUATOR && tokens[index].subType == (int)Punctuators::COMMA) {
            addTerminalToTree(tokens, index, newNode);
            if(isFunctionOperandList(tokens, index, newNode)) {
                tree->addSubNode(newNode);
                return true;
            }
            else {
                delete newNode;
                index = temp;
                return false;
            }
        }

        tree->addSubNode(newNode);
        return true;
    }

    delete newNode;
    index = temp;
    return false;
}

static bool isFunctionOperands(Token* tokens, uint32_t& index, GrammarTree* tree) {
    uint32_t temp = index;
    GrammarTree* newNode = new GrammarTree(FUNCTION_DEC_ARG_LIST, &tokens[index]);

    if(tokens[index].type == (int)TokenTypes::PUNCTUATOR && tokens[index].subType == (int)Punctuators::OPARN) {
        addTerminalToTree(tokens, index, newNode);
        isFunctionOperandList(tokens, index, newNode);
        if(tokens[index].type == (int)TokenTypes::PUNCTUATOR && tokens[index].subType == (int)Punctuators::CPARN) {
            addTerminalToTree(tokens, index, newNode);
            tree->addSubNode(newNode);
            return true;
        }
    }

    delete newNode;
    index = temp;
    return false;
}

static bool isFunction(Token* tokens, uint32_t& index, GrammarTree* tree) {
    uint32_t temp = index;
    GrammarTree* newNode = new GrammarTree(FUNCTION, &tokens[index]);

    if(isType(tokens, index, newNode)) {
        if(tokens[index].type == (int)TokenTypes::IDENTIFIER) {
            addTerminalToTree(tokens, index, newNode);

            if(isFunctionOperands(tokens, index, newNode)) {
                if(isCodeBlock(tokens, index, newNode) || isStatement(tokens, index, newNode)) {
                    tree->addSubNode(newNode);
                    return true;
                }
            }
        }
    }

    delete newNode;
    index = temp;
    return false;
}

static bool isLoopArg(Token* tokens, uint32_t& index, GrammarTree* tree) {
    uint32_t temp = index;
    GrammarTree* newNode = new GrammarTree(SUB_STATEMENT, &tokens[index]);

    if(isExpression(tokens, index, newNode) || isInitialization(tokens, index, newNode) || isDeclaration(tokens, index, newNode)) {
        tree->addSubNode(newNode);
        return true;
    }

    delete newNode;
    index = temp;
    return false;
}

static bool isCodeBlock(Token* tokens, uint32_t& index, GrammarTree* tree) {
    uint32_t temp = index;
    GrammarTree* newNode = new GrammarTree(CODE_BLOCK, &tokens[index]);

    if(tokens[index].type == (int)TokenTypes::PUNCTUATOR && tokens[index].subType == (int)Punctuators::OBRACE) {
        addTerminalToTree(tokens, index, newNode);
        if(isStatementList(tokens, index, newNode)) {
            if(tokens[index].type == (int)TokenTypes::PUNCTUATOR && tokens[index].subType == (int)Punctuators::CBRACE) {
                addTerminalToTree(tokens, index, newNode);
                tree->addSubNode(newNode);
                return true;
            }
        }
    }

    delete newNode;
    index = temp;
    return false;
}

static bool isDoWhileLoop(Token* tokens, uint32_t& index, GrammarTree* tree) {
    uint32_t temp = index;
    GrammarTree* newNode = new GrammarTree(DO_WHILE_LOOP, &tokens[index]);

    if(tokens[index].type == (int)TokenTypes::CONSTRUCT && tokens[index].subType == (int)Constructs::DO) {
        addTerminalToTree(tokens, index, newNode);
        if(isCodeBlock(tokens, index, newNode) || isStatement(tokens, index, newNode)) {
            if(tokens[index].type == (int)TokenTypes::CONSTRUCT && tokens[index].subType == (int)Constructs::WHILE) {
                addTerminalToTree(tokens, index, newNode);
                if(tokens[index].type == (int)TokenTypes::PUNCTUATOR && tokens[index].subType == (int)Punctuators::OPARN) {
                    addTerminalToTree(tokens, index, newNode);
                    if(isLoopArg(tokens, index, newNode)) {
                        if(tokens[index].type == (int)TokenTypes::PUNCTUATOR && tokens[index].subType == (int)Punctuators::CPARN) {
                            addTerminalToTree(tokens, index, newNode);
                            tree->addSubNode(newNode);
                            return true;
                        }
                    }
                }
            }
        }
    }

    delete newNode;
    index = temp;
    return false;
}

static bool isWhileLoop(Token* tokens, uint32_t& index, GrammarTree* tree) {
    uint32_t temp = index;
    GrammarTree* newNode = new GrammarTree(WHILE_LOOP, &tokens[index]);

    if(tokens[index].type == (int)TokenTypes::CONSTRUCT && tokens[index].subType == (int)Constructs::WHILE) {
        addTerminalToTree(tokens, index, newNode);
        if(tokens[index].type == (int)TokenTypes::PUNCTUATOR && tokens[index].subType == (int)Punctuators::OPARN) {
            addTerminalToTree(tokens, index, newNode);
            if(isLoopArg(tokens, index, newNode)) {
                if(tokens[index].type == (int)TokenTypes::PUNCTUATOR && tokens[index].subType == (int)Punctuators::CPARN) {
                    addTerminalToTree(tokens, index, newNode);
                    if(isCodeBlock(tokens, index, newNode) || isStatement(tokens, index, newNode)) {
                        tree->addSubNode(newNode);
                        return true;
                    }
                }
            }
        }
    }

    delete newNode;
    index = temp;
    return false;
}

static bool isSubStatement(Token* tokens, uint32_t& index, GrammarTree* tree) {
    uint32_t temp = index;
    GrammarTree* newNode = new GrammarTree(SUB_STATEMENT, &tokens[index]);

    if(isLoopArg(tokens, index, newNode)) {
        if(tokens[index].type == (int)TokenTypes::PUNCTUATOR && tokens[index].subType == (int)Punctuators::EOL) {
            addTerminalToTree(tokens, index, newNode);
            if(isLoopArg(tokens, index, newNode)) {
                if(tokens[index].type == (int)TokenTypes::PUNCTUATOR && tokens[index].subType == (int)Punctuators::EOL) {
                    addTerminalToTree(tokens, index, newNode);
                    isLoopArg(tokens, index, newNode);
                    tree->addSubNode(newNode);
                    return true;
                }
            }
        }
    }

    delete newNode;
    index = temp;
    return false;
}

static bool isForLoop(Token* tokens, uint32_t& index, GrammarTree* tree) {
    uint32_t temp = index;
    GrammarTree* newNode = new GrammarTree(FOR_LOOP, &tokens[index]);

    if(tokens[index].type == (int)TokenTypes::CONSTRUCT && tokens[index].subType == (int)Constructs::FOR) {
        addTerminalToTree(tokens, index, newNode);

        if(tokens[index].type == (int)TokenTypes::PUNCTUATOR && tokens[index].subType == (int)Punctuators::OPARN) {
            addTerminalToTree(tokens, index, newNode);
            if(isSubStatement(tokens, index, newNode)) {
                if(tokens[index].type == (int)TokenTypes::PUNCTUATOR && tokens[index].subType == (int)Punctuators::CPARN) {
                    addTerminalToTree(tokens, index, newNode);
                    if(isCodeBlock(tokens, index, newNode)) {
                        tree->addSubNode(newNode);
                        return true;
                    }
                    else {
                        if(isStatement(tokens, index, newNode)) {
                            tree->addSubNode(newNode);
                            return true;
                        }
                    }
                }
            }
        }
    }

    delete newNode;
    index = temp;
    return false;
}

static bool isLoop(Token* tokens, uint32_t& index, GrammarTree* tree) {
    uint32_t temp = index;
    GrammarTree* newNode = new GrammarTree(LOOP, &tokens[index]);

    if(isDoWhileLoop(tokens, index, newNode)) {
        if(tokens[index].type == (int)TokenTypes::PUNCTUATOR && tokens[index].subType == (int)Punctuators::EOL) {
            addTerminalToTree(tokens, index, newNode);
            tree->addSubNode(newNode);
            return true;
        }
    }
    else if(isWhileLoop(tokens, index, newNode) || isForLoop(tokens, index, newNode)) {
        tree->addSubNode(newNode);
        return true;
    }

    delete newNode;
    index = temp;
    return false;
}

static bool isIf(Token* tokens, uint32_t& index, GrammarTree* tree) {
    uint32_t temp = index;
    GrammarTree* newNode = new GrammarTree(IF_STATEMENT, &tokens[index]);

    if(tokens[index].type == (int)TokenTypes::CONSTRUCT && tokens[index].subType == (int)Constructs::IF) {
        addTerminalToTree(tokens, index, newNode);
        if(tokens[index].type == (int)TokenTypes::PUNCTUATOR && tokens[index].subType == (int)Punctuators::OPARN) {
            addTerminalToTree(tokens, index, newNode);
            if(isLoopArg(tokens, index, newNode)) {
                if(tokens[index].type == (int)TokenTypes::PUNCTUATOR && tokens[index].subType == (int)Punctuators::CPARN) {
                    addTerminalToTree(tokens, index, newNode);
                    if(isCodeBlock(tokens, index, newNode) || isStatement(tokens, index, newNode)) {
                        if(tokens[index].type == (int)TokenTypes::CONSTRUCT && tokens[index].subType == (int)Constructs::ELSE) {
                            addTerminalToTree(tokens, index, newNode);
                            if(isIf(tokens, index, newNode) || isCodeBlock(tokens, index, newNode) || isStatement(tokens, index, newNode)) {
                                tree->addSubNode(newNode);
                                return true;
                            }
                        }
                        else {
                            tree->addSubNode(newNode);
                            return true;
                        }
                    }
                }
            }
        }
    }

    delete newNode;
    index = temp;
    return false;
}

static bool isCase(Token* tokens, uint32_t& index, GrammarTree* tree) {
    uint32_t temp = index;
    GrammarTree* newNode = new GrammarTree(CASE, &tokens[index]);

    if(tokens[index].type == (int)TokenTypes::CONSTRUCT && tokens[index].subType == (int)Constructs::CASE) {
        addTerminalToTree(tokens, index, newNode);
        if(isExpression(tokens, index, newNode)) {
            if(tokens[index].type == (int)TokenTypes::PUNCTUATOR && tokens[index].subType == (int)Punctuators::COLON) {
                addTerminalToTree(tokens, index, newNode);
                if(isStatementList(tokens, index, newNode)) {
                    tree->addSubNode(newNode);
                    return true;
                }
            }
        }
    }

    delete newNode;
    index = temp;
    return false;
}

static bool isCaseList(Token* tokens, uint32_t& index, GrammarTree* tree) {
    uint32_t temp = index;
    GrammarTree* newNode = new GrammarTree(CASE_LIST, &tokens[index]);

    if(isCase(tokens, index, newNode)) {
        isCaseList(tokens, index, newNode);
        tree->addSubNode(newNode);
        return true;
    }

    delete newNode;
    index = temp;
    return true;
}

static bool isSwitchStatement(Token* tokens, uint32_t& index, GrammarTree* tree) {
    uint32_t temp = index;
    GrammarTree* newNode = new GrammarTree(SWITCH_STATEMENT, &tokens[index]);


    if(tokens[index].type == (int)TokenTypes::CONSTRUCT && tokens[index].subType == (int)Constructs::SWITCH) {
        addTerminalToTree(tokens, index, newNode);
        if(tokens[index].type == (int)TokenTypes::PUNCTUATOR && tokens[index].subType == (int)Punctuators::OPARN) {
            addTerminalToTree(tokens, index, newNode);
            if(isLoopArg(tokens, index, newNode)) {
                if(tokens[index].type == (int)TokenTypes::PUNCTUATOR && tokens[index].subType == (int)Punctuators::CPARN) {
                    addTerminalToTree(tokens, index, newNode);
                    if(tokens[index].type == (int)TokenTypes::PUNCTUATOR && tokens[index].subType == (int)Punctuators::OBRACE) {
                        addTerminalToTree(tokens, index, newNode);
                        if(isCaseList(tokens, index, newNode)) {
                            if(tokens[index].type == (int)TokenTypes::PUNCTUATOR && tokens[index].subType == (int)Punctuators::CBRACE) {
                                addTerminalToTree(tokens, index, newNode);
                                tree->addSubNode(newNode);
                                return true;
                            }
                        }
                    }
                }
            }
        }
    }

    delete newNode;
    index = temp;
    return false;
}

static bool isReturn(Token* tokens, uint32_t& index, GrammarTree* tree) {
    uint32_t temp = index;
    GrammarTree* newNode = new GrammarTree(RETURN_STATEMENT, &tokens[index]);

    if(tokens[index].type == (int)TokenTypes::CONSTRUCT && tokens[index].subType == (int)Constructs::RETURN) {
        addTerminalToTree(tokens, index, newNode);
        isExpression(tokens, index, newNode);
        tree->addSubNode(newNode);
        return true;
    }

    delete newNode;
    index = temp;
    return false;
}

static bool isStatement(Token* tokens, uint32_t& index, GrammarTree* tree) {
    uint32_t temp = index;
    GrammarTree* newNode = new GrammarTree(STATEMENT, &tokens[index]);

    if(tokens[index].type == (int)TokenTypes::PUNCTUATOR && tokens[index].subType == (int)Punctuators::EOL) {
        addTerminalToTree(tokens, index, newNode);
        tree->addSubNode(newNode);
        return true;
    }
    else if(isCodeBlock(tokens, index, newNode) || isFunction(tokens, index, newNode) || isLoop(tokens, index, newNode) || isIf(tokens, index, newNode) || isSwitchStatement(tokens, index, newNode)) {
        tree->addSubNode(newNode);
        return true;
    }
    else if((isFunctionDeclaration(tokens, index, newNode) || isInitialization(tokens, index, newNode) || 
        isDeclaration(tokens, index, newNode) || isExpression(tokens, index, newNode) || isReturn(tokens, index, newNode)) &&
        (tokens[index].type == (int)TokenTypes::PUNCTUATOR && tokens[index].subType == (int)Punctuators::EOL)) {
            addTerminalToTree(tokens, index, newNode);
            tree->addSubNode(newNode);
            return true;
    }

    delete newNode;
    index = temp;
    return false;
}

static bool isStatementList(Token* tokens, uint32_t& index, GrammarTree* tree) {
    uint32_t temp = index;
    GrammarTree* newNode = new GrammarTree(STATEMENT_LIST, &tokens[index]);

    if(isStatement(tokens, index, newNode)) {
        isStatementList(tokens, index, newNode);
        tree->addSubNode(newNode);
    }

    return true;
}

static bool isStructureStatement(Token* tokens, uint32_t& index, GrammarTree* tree) {
    uint32_t temp = index;
    GrammarTree* newNode = new GrammarTree(STATEMENT, &tokens[index]);

    //a structure statement is either a declaration
    if(isStructure(tokens, index, newNode) || isFunction(tokens, index, newNode)) {
        tree->addSubNode(newNode);
        return true;
    }
    else if((isFunctionDeclaration(tokens, index, newNode) || isInitialization(tokens, index, newNode) || isDeclaration(tokens, index, newNode)) && 
            tokens[index].type == (int)TokenTypes::PUNCTUATOR && tokens[index].subType == (int)Punctuators::EOL) {
        addTerminalToTree(tokens, index, newNode);
        tree->addSubNode(newNode);
        return true;
    }

    delete newNode;
    index = temp;
    return false;
}

static bool isStructureStatementList(Token* tokens, uint32_t& index, GrammarTree* tree) {
    uint32_t temp = index;
    GrammarTree* newNode = new GrammarTree(STATEMENT_LIST, &tokens[index]);

    if(isStructureStatement(tokens, index, newNode)) {
        isStructureStatementList(tokens, index, newNode);
        tree->addSubNode(newNode);
    }

    return true;
}

static bool isStructureDefinition(Token* tokens, uint32_t& index, GrammarTree* tree) {
    uint32_t temp = index;
    GrammarTree* newNode = new GrammarTree(STRUCT, &tokens[index]);

    if(tokens[index].type == (int)TokenTypes::IDENTIFIER) {
        addTerminalToTree(tokens, index, newNode);
        if(tokens[index].type == (int)TokenTypes::PUNCTUATOR && tokens[index].subType == (int)Punctuators::OBRACE) {
            addTerminalToTree(tokens, index, newNode);
            isStructureStatementList(tokens, index, newNode);
            if(tokens[index].type == (int)TokenTypes::PUNCTUATOR && tokens[index].subType == (int)Punctuators::CBRACE) {
                addTerminalToTree(tokens, index, newNode);
                tree->addSubNode(newNode);
                return true;
            }
        }
    }

    delete newNode;
    index = temp;
    return false;
}

static bool isStruct(Token* tokens, uint32_t& index, GrammarTree* tree) {
    uint32_t temp = index;
    GrammarTree* newNode = new GrammarTree(STRUCT, &tokens[index]);

    if(tokens[index].type == (int)TokenTypes::CONSTRUCT && tokens[index].subType == (int)Constructs::STRUCT) {
        addTerminalToTree(tokens, index, newNode);
        if(isStructureDefinition(tokens, index, newNode)) {
            tree->addSubNode(newNode);
            return true;
        }
    }

    delete newNode;
    index = temp;
    return false;
}

static bool isClass(Token* tokens, uint32_t& index, GrammarTree* tree) {
    uint32_t temp = index;
    GrammarTree* newNode = new GrammarTree(CLASS, &tokens[index]);

    if(tokens[index].type == (int)TokenTypes::CONSTRUCT && tokens[index].subType == (int)Constructs::CLASS) {
        addTerminalToTree(tokens, index, newNode);
        if(isStructureDefinition(tokens, index, newNode)) {
            tree->addSubNode(newNode);
            return true;
        }
    }

    delete newNode;
    index = temp;
    return false;
}

static bool isStructure(Token* tokens, uint32_t& index, GrammarTree* tree) {
    uint32_t temp = index;
    GrammarTree* newNode = new GrammarTree(STRUCTURE, &tokens[index]);

    if(isClass(tokens, index, newNode) || isStruct(tokens, index, newNode)) {
        tree->addSubNode(newNode);
        return true;
    }

    delete newNode;
    index = temp;
    return false;
}

static bool isGlobal(Token* tokens, uint32_t& index, GrammarTree* tree) {
    uint32_t temp = index;
    GrammarTree* newNode = new GrammarTree(GLOBAL, &tokens[index]);

    //a global is either a declaration
    if(isStructure(tokens, index, newNode) || isFunction(tokens, index, newNode)) {
        tree->addSubNode(newNode);
        return true;
    }
    else if((isFunctionDeclaration(tokens, index, newNode) || isInitialization(tokens, index, newNode) || isDeclaration(tokens, index, newNode) || isTypedef(tokens, index, newNode)) && 
            tokens[index].type == (int)TokenTypes::PUNCTUATOR && tokens[index].subType == (int)Punctuators::EOL) {
        addTerminalToTree(tokens, index, newNode);
        tree->addSubNode(newNode);
        return true;
    }

    delete newNode;
    index = temp;
    return false;
}

static bool isGlobalList(Token* tokens, uint32_t& index, GrammarTree* tree) {
    uint32_t temp = index;
    GrammarTree* newNode = new GrammarTree(GLOBAL_LIST, &tokens[index]);
    
    //a global list is a global or end of file
    if(isGlobal(tokens, index, newNode)) {
        isGlobalList(tokens, index, newNode);
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

static bool isValidFile(Token* tokens, uint32_t& index, GrammarTree* tree) {
    uint32_t temp = index;
    GrammarTree* newNode = new GrammarTree(LANGUAGE_FILE, &tokens[index]);

    //its valid if it has a list of globals
    if(isGlobalList(tokens, index, newNode) || isEOF(tokens, index, newNode)) {
        tree->addSubNode(newNode);
        return true;
    }

    delete newNode;
    index = temp;
    return false;
}

static bool generateParseTree(Token* tokens, GrammarTree* tree) {
    uint32_t tokenIndex = 0;

    if(isValidFile(tokens, tokenIndex, tree)) {
        std::cout << "true\n";
        return true;
    }

    std::cout << "false\n";
    return false;
}

int compileFile(const std::string& inputFile, const std::string& outputFile) {
    line = 0;
    FILE* file = fopen(inputFile.c_str(), "rb");

    if (!file) {
        std::cout << "Failed to load file\n";
        return 0;
    }

    fseek(file, 0L, SEEK_END);
    size_t fileSize = ftell(file);
    fseek(file, 0L, SEEK_SET);

    char* lFile = new char[fileSize + 1];
    Token* tokenStream = new Token[fileSize + 1];
    fread(lFile, sizeof(char), fileSize, file);
    fclose(file);
    lFile[fileSize] = 0;

    std::ofstream outputStream(outputFile, std::ios::binary);
    GrammarTree* grammarTree = new GrammarTree(FILE_START, &tokenStream[0]);

    generateTokenStream(lFile, fileSize, tokenStream);
    generateParseTree(tokenStream, grammarTree);
    grammarTree->print();

    outputStream.close();
    delete[] lFile;
    delete[] tokenStream;
    return 0;
}