//setup the tree for each expression & ensure that each intermediate value of the expression 
//is the right type or can be converted to the right type for the upper layers. Then make sure
//that the final result matches that correct output type. This goes for function arguments, and assignments and return types
//make sure all types and identifiers have been defined at the right place of their use
//identifiers must be declared above their first use and types must be declared in the same scope or a parent scope

#include <string>
#include <vector>
#include "Semantics.h"
#define ENFORCECON(con, str) if(!enforceCondition(con, str)) return false; else;

std::vector<std::string> errorList;
bool SCModifierList(GrammarTree* tree, bool dataAccessModifier, bool signedSpecifier, bool constAllowed, bool accessModifierAllowed);

static bool enforceCondition(bool condition, std::string failError = "Syntax error") {
    if(!condition) {
        errorList.push_back(failError);
    }

    return condition;
}

bool SCStructure(GrammarTree* tree) {
    //not yet
    return false;
}

bool SCModifier(GrammarTree* tree, bool& dataAccessModifier, bool& signedSpecifier, bool& accessModifierAllowed) {
    ENFORCECON(tree->subNodes.size() > 0, "");
    switch(tree->nodeStart->subType) {
        case (int)Modifiers::CONST:
        break;
        case (int)Modifiers::PRIVATE:
        case (int)Modifiers::PUBLIC:
        ENFORCECON(accessModifierAllowed, "Invalid use of access modifier");
        break;
        case (int)Modifiers::SIGNED:
        case (int)Modifiers::UNSIGNED:
        ENFORCECON(!signedSpecifier, "Invalid combination of modifiers");
        signedSpecifier = true;
        break;
        case (int)Modifiers::EXTERN:
        case (int)Modifiers::STATIC:
        ENFORCECON(!dataAccessModifier, "Invalid combination of access modifiers");
        dataAccessModifier = true;
        break;
    }

    return true;
}

bool SCModifierList(GrammarTree* tree, bool dataAccessModifier, bool signedSpecifier, bool accessModifierAllowed) {
    ENFORCECON(tree->subNodes.size() > 0, "");
    if(SCModifier(tree->subNodes[0], dataAccessModifier, signedSpecifier, accessModifierAllowed)) {
        if(tree->subNodes.size() > 1) {
            return SCModifierList(tree->subNodes[1], dataAccessModifier, signedSpecifier, accessModifierAllowed);
        }

        return true;
    }

    return false;
}

bool SCType(GrammarTree* tree, bool accessModifierAllowed) {
    int nodeIndex = 0;
    if(tree->subNodes[0]->nodeType == MODIFIER_LIST) {
        if(!SCModifierList(tree->subNodes[nodeIndex++], false, false, accessModifierAllowed)) {
            return false;
        }
    }

    //parse the type
    return true;
}

bool SCFunctionOperand(GrammarTree* tree) {
    if(SCType(tree->subNodes[0], false)) {
        return true;
    }

    return false;
}

bool SCFunctionOperandList(GrammarTree* tree) {
    if(SCFunctionOperand(tree->subNodes[0])) {
        if(tree->subNodes.size() > 2) {
            return SCFunctionOperandList(tree->subNodes[2]);
        }

        return true;
    }

    return false;
}

bool SCFunctionOperands(GrammarTree* tree) {
    if(tree->subNodes.size() > 2) {
        return SCFunctionOperandList(tree->subNodes[1]);
    }

    return true;
}

bool SCCodeBlock(GrammarTree* tree) {
    return true;
}

bool SCStatement(GrammarTree* tree) {
    return true;
}

bool SCFunction(GrammarTree* tree) {
    //parse the return type
    ENFORCECON(tree->subNodes.size() == 4, "");
    ENFORCECON(SCType(tree->subNodes[0], false), "");
    //ENFORCECON(SCIdt(tree->subNodes[1]), "");
    ENFORCECON(SCFunctionOperands(tree->subNodes[2]), "");
    ENFORCECON(SCCodeBlock(tree->subNodes[3]) || SCStatement(tree->subNodes[3]), "");
    return true;
}

bool SCInitialization(GrammarTree* tree) {
    return false;
}

bool SCDeclaration(GrammarTree* tree) {
    return false;
}

bool SCTypedef(GrammarTree* tree) {
    return false;
}

bool SCEOF(GrammarTree* tree) {
    if(tree->nodeStart->type == (int)TokenTypes::ENDOFFILE) {
        return true;
    }

    return false;
}

bool SCGlobal(GrammarTree* tree) {
    ENFORCECON(tree->subNodes.size() > 0, "");
    if(SCStructure(tree->subNodes[0]) || SCFunction(tree->subNodes[0])) {
        return true;
    }
    else if(SCInitialization(tree->subNodes[0]) || SCDeclaration(tree->subNodes[0]) || SCTypedef(tree->subNodes[0])) {
        return true;
    }

    ENFORCECON(false, "");
    return false;
}

bool SCGlobalList(GrammarTree* tree) {
    ENFORCECON(tree->subNodes.size() > 0, "Invalid Cim file");
    if(SCGlobal(tree->subNodes[0]) || SCEOF(tree->subNodes[0])) {
        if(tree->subNodes.size() > 1) {
            return SCGlobalList(tree->subNodes[1]);
        }

        return true;
    }

    return false;
}

bool SCFile(GrammarTree* tree) {
    ENFORCECON(tree->subNodes.size() > 0, "");
    ENFORCECON(SCGlobalList(tree->subNodes[0]), "");
    return true;
}

bool semanticsPass(GrammarTree* tree) {
    ENFORCECON(tree->subNodes.size() > 0, "Invalid Cim file");

    if(SCFile(tree->subNodes[0])) {
        return true;
    }
    else {
        std::cout << "Printing Errors: \n";
        for(int i = 0; i < errorList.size(); i++) {
            std::cout << errorList[i] << "\n";
        }
    }

    return false;
}