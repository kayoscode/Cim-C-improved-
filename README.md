# Cim-C-improved-
The goal of this project is to reinvent the popular language C and give it more modern features while maintaining the versitility that made it such a popular language.
<br>
For now, I have preserved a lot of the syntax of C, but I have added a few things that should make it easier to work with. <br>
<ol>
<li>NO MORE FUNCTION/CLASS DECLARATIONS (its about time native code can be compiled without that with a decent language syntax)</li>
<li>Single line functions</li>
<li>Flipped the order of typedefs</li>
<li>Reworking function pointer syntax: ret_type() makes it a function: but ret_type()() makes it a function that
  returns a function pointer. ret_type() ptrName; is a function pointer returning the return type, etc.</li>
<li>Both array and pointer indicators (* and []) are prefixed before the variable name: TODO: add comma deliminated variable definitions</li>
<li>Classes/Structs with the same syntax for classes as Java. Structs are identical to classes in this langauge!</li>
<li>Class/function/method metadata at compiletime - similar to how C++ does it - not like java or C# - Perhaps in the future, that metadata can be stored next to the variable with an updated vtable</li>
<li>Hopefully in the future I implement optimizations if I can figure it out.</li>
<li>A linker</li>
</ol>

At this point, the project is in it's infancy. List of currently implemented features:
<ol>
<li>An intermediate bytecode language architecture needing updating to allow 64 bit operations</li>
<li>An emulator fully emulating the functionlity of that intermediate langauge</li>
<li>A mostly complete assembler for that intermediate langauge - TODO: floating point support, double floating point support, ERROR LOGGING instead of infinite looping, etc</li>
<li>The start of a compiler built from scratch - currently only parses the syntax of the language and is missing out of Array/object initializers</li>
</ol>

TODO: There is a lot todo, so don't expect this to be comprehensive:
<ol>
<li>A linker so we can use multiple files</li>
<li>Assembler 64 bit targeting / doubles/longs</li>
<li>Improve assembler design (The code isn't easy to modify or read)</li>
<li>Finish the compiler</li>
<li>Optmizations</li>
<li>Finish the compiler</li>
<li>Add features which improve the life of the programmer while preserving the speed and control of C</li>
</ol>

Problems:
<ol>
<li>To avoid stack overflow when compiling large files, I increased the stack size to 50Mb which can support up to 500k global symbols (or around 2.5M lines of code) - Check my math on that</li>
<li>My intermediate architecture might be too far from x64_86 so it may need to be modified accordingly</li>
<li>At this point, the assembler doesn't know where the text segment starts, so if you put the DATA segment before the text segment, you will have to put a text segment above it jumping to a lable in the main text segment!!!!!</li>
<li>I dont know if the emulator works on big endian systems, but it should if you compile it on one - I would appreciate if someone would test</li>
<li>Missing an instruction converting floating point register to int register</li>
</ol>

Instructions:
The assembly language has a very simple syntax, and I can't go over all the defined functionality here. As a rule, each .SEGMENT_NAME is alligned to the min byte compression for the datatype.
The following are directives:
<ol>
<li>.data - store data</li>
  <ol>
  <li>.const - DOES NOT INCLUDE DATA BUT MUST BE PUT IN A DATA SEGMENT, its simply an identifier: .const variable_name = (expr), ...</li>
  <li>.word - Stores a list of 4 byte integers: .word expr, expr, expr,...<li>
  <li>.half - -Stores a list of 2 byte integers .half expr, expr, expr,...</li>
  <li>.byte - Stores a list of 1 byte integers: .byte expr, expr, expr,...</li>
  <li>.float - Stores a list of floats same syntax as above - CURRENTLY NOT WORKING</li>
  <li>.ascii - Stores a string - escape characters not included yet .ascii "STRING", "STRING" <li>
  <li>.asciiz - Stores a null terminated string same as above</li>
  </ol>
<li>.text - stores instructions in the program Currently, there are 54 instructions, I will list them here.</li>
<br> There are either categories each with their own expected arguments: AL, SA, FPA, FPS, MEM, INT, STACK, JMP
<br>
These are the instructions as listed in the code: "Encode.h"
//NOTE: THESE NEED TO BE IN THE EXACT SAME ORDER AS THE STRINGS DEFINED IN defs.cpp because it uses their index in the list as the opcode number
<code>
enum class ALOps {
	LSL, LSR,
	ADD, SUB, 
	MUL, DIV,
	ULSL, ULSR,
	UADD, USUB,
	AND, 
	OR, XOR
};

enum class SAOps {
	MOV, CMP, 
    LA, MOV_GE, 
    MOV_G, MOV_LE, 
    MOV_L, MOV_E, 
    MOV_C, MOV_NE
}; //TODO FTOR

enum class FPAOps {
    F_ADD, F_SUB,
    F_MUL, F_DIV
};

enum class FPSAOps {
    MOV, CMP,
    F_S, F_L,
    RTOF
};

enum class MEMOps {
    SB, SH,
    SW, LB,
    LH, LW
};

enum class INTOps {
    INT, HWINT
};

enum class STACKOps {
    PUSH, POP
};

enum class JMPOps {
    JMP, JGE, 
	JG, JLE, 
	JL, JE,
	JC, JNE,
	CALL
};

enum class Registers {
    R0, R1, R2, R3,
    R4, R5, R6, R7,
    R8, R9, R10, R11,
    R12, R13, R14, R15,
    PC, BP, SP, FR, RA, 
    F0, F1, F2, F3, 
    F4, F5, F6, F7, 
    F8, F9, F10, F11, 
    F12, F13, F14, F15, 
};
</code>
All those instructions are defined in the documentation that I will upload very soon.
<br> 
Instrutions are encoded with the low bytes representing the instruciton type, and the next couple bytes representing the sub-opcode. Flags are updated after each instruction that updates them.
<br>
Here are a few examples:
push [ra, pc, r1] -> pushes the registers in order listed <br>
pop [ra, pc, r1] -> pops the registers in reverse order listed <br>
call label/register -> if its a label, its a relative offset, otherwise its absolute <br>
la r0, label -> loads the relative address of a label into r0 <br>
div r0, r0, r1 -> stores the result of r0/r1 and stores the remainder in r15. Divide by zero doesn't trigger an exception.
</ol>

As the compiler is currenly not funcitonal, outside of parsing the language into a tree, I don't have any further details as of now. I will update this README as I update the code. <br>
Feel free to reach out to me if you'd like to contribute.
