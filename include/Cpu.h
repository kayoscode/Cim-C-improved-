#ifndef INCLUDE_CPU_H
#define INCLUDE_CPU_H

#include <stdint.h>
#include <fstream>

#include "defs.h"

#include "Register.h"
#include "Memory.h"

class Cpu {
    public:
        Cpu(uint32_t memorySize);
        virtual ~Cpu();

        void loadBinary(uint32_t* binary, int size, uint32_t location);
        void loadBinaryFile(const char* fileName, uint32_t location);

        inline void setPC(uint32_t location) {
            registers[PC].setValue(location);
        }

        inline uint32_t getPC() {
            return registers[PC].getValue();
        }

        void executeProgram();
        void executeInstruction();

        inline bool isFlagSet(uint32_t flag) {
            return ((registers[FR].value & flag) != 0);
        }

        inline void setFlag(uint32_t flag) {
            registers[FR].value |= flag;
        }

        inline void clearFlag(uint32_t flag) {
            registers[FR].value &= ~(flag);
        }

        void printFlags() {
            std::cout << "Negative: " << isFlagSet(FL_NEGATIVE) << "\n";
            std::cout << "Zero: " << isFlagSet(FL_ZERO) << "\n";
            std::cout << "CARRY: " << isFlagSet(FL_CARRY) << "\n";
            std::cout << "OVERFLOW: " << isFlagSet(FL_OVERFLOW) << "\n";
        }

        inline void setZeroAndNeg(uint32_t result1) {
            int32_t result = (int32_t)result1;

            if (result == 0) {
                setFlag(FL_ZERO);
            }
            else {
                clearFlag(FL_ZERO);
            }

            if (result < 0) {
                setFlag(FL_NEGATIVE);
            }
            else{
                clearFlag(FL_NEGATIVE);
            }
        }

        inline void setZeroAndNeg(float result) {
            if (result < FP_EPSILON && result > -FP_EPSILON) {
                setFlag(FL_ZERO);
            }
            else {
                clearFlag(FL_ZERO);
            }

            if (result < 0) {
                setFlag(FL_NEGATIVE);
            }
            else{
                clearFlag(FL_NEGATIVE);
            }
        }

        inline void setZero(uint32_t result) {
            if (result == 0) {
                setFlag(FL_ZERO);
            }
            else {
                clearFlag(FL_ZERO);
            }
        }

        inline void setOverFlow(int32_t operand1, int32_t operand2) {
            int64_t answer = (int64_t)operand1 + (int64_t)operand2;
            int32_t answer2 = (int32_t)answer;

            if (answer == answer2) {
                clearFlag(FL_OVERFLOW);
            }
            else {
                setFlag(FL_OVERFLOW);
            }
        }

        inline void setCarry(uint32_t operand1, uint32_t operand2) {
            uint64_t answer = (uint32_t)operand1 + (uint64_t)operand2;
            uint32_t answer2 = (uint32_t)answer;

            if (answer == answer2) {
                clearFlag(FL_CARRY);
            }
            else {
                setFlag(FL_CARRY);
            }
        }

        void printRegister(int i, bool fl = false) {
            if(!fl) {
                LTRACE("{s} = 0x{x}, {d} {c}", registerNames[i], registers[i].value, registers[i].value, registers[i].value);
            }
            else {
                LTRACE("{s} = {f}", fregisterNames[i], fRegisters[i].value);
            }
        }

        void printMem(uint32_t location, int count = 1) {
            for(int i = 0; i < count; i++) {
                LTRACE("Mem[0x{x}] = {d} 0x{x} {f}", location + i * 4, mem.getWord(location + i * 4), mem.getWord(location + i * 4), mem.getWord(location + i * 4));
            }
        }

        uint32_t getMem(uint32_t location) {
            return mem.getWord(location);
        }

        uint32_t getRegister(uint8_t reg, bool fl = false) {
            if(fl) {
                return *(uint32_t*)&fRegisters[reg].value;
            }
            else {
                return registers[reg].value;
            }
        }

        float getFloatRegister(uint8_t reg) {
            return fRegisters[reg].value;
        }

        bool stopSet = false;

    private:
        Memory mem;
        Register registers[REGISTER_COUNT];
        FRegister fRegisters[FREGISTER_COUNT];

        friend void decodeStackInstruction(Cpu& decoder, uint32_t instruction, uint32_t& programCounter);
        friend void decodeArithmeticLogic(Cpu& decoder, uint32_t instruction, uint32_t& programCounter);
        friend void decodeSpecialArithmetic(Cpu& decoder, uint32_t instruction, uint32_t& programCounter);
        friend void decodeFloatingPointArithmetic(Cpu& decoder, uint32_t instruction, uint32_t& programCounter);
        friend void decodeFloatingPointSpecialArithmetic(Cpu& decoder, uint32_t instruction, uint32_t& programCounter);
        friend void decodeMemInstruction(Cpu& decoder, uint32_t instruction, uint32_t& programCounter);
        friend void decodeJmpInstruction(Cpu& decoder, uint32_t instruction, uint32_t& programCounter);
        friend void decodeInterruptInstruction(Cpu& decoder, uint32_t instruction, uint32_t& programCounter);

        friend void AL_LSL(Cpu& decoder, uint8_t dest, uint8_t src, uint32_t arg, bool immediateFlag);
        friend void AL_LSR(Cpu& decoder, uint8_t dest, uint8_t src, uint32_t arg, bool immediateFlag);
        friend void AL_ADD(Cpu& decoder, uint8_t dest, uint8_t src, uint32_t arg, bool immediateFlag);
        friend void AL_SUB(Cpu& decoder, uint8_t dest, uint8_t src, uint32_t arg, bool immediateFlag);
        friend void AL_MUL(Cpu& decoder, uint8_t dest, uint8_t src, uint32_t arg, bool immediateFlag);
        friend void AL_DIV(Cpu& decoder, uint8_t dest, uint8_t src, uint32_t arg, bool immediateFlag);
        friend void AL_ULSL(Cpu& decoder, uint8_t dest, uint8_t src, uint32_t arg, bool immediateFlag);
        friend void AL_ULSR(Cpu& decoder, uint8_t dest, uint8_t src, uint32_t arg, bool immediateFlag);
        friend void AL_UADD(Cpu& decoder, uint8_t dest, uint8_t src, uint32_t arg, bool immediateFlag);
        friend void AL_USUB(Cpu& decoder, uint8_t dest, uint8_t src, uint32_t arg, bool immediateFlag);
        friend void AL_AND(Cpu& decoder, uint8_t dest, uint8_t src, uint32_t arg, bool immediateFlag);
        friend void AL_OR(Cpu& decoder, uint8_t dest, uint8_t src, uint32_t arg, bool immediateFlag);
        friend void AL_XOR(Cpu& decoder, uint8_t dest, uint8_t src, uint32_t arg, bool immediateFlag);
        friend void SA_MOV(Cpu& decoder, uint8_t dest, uint32_t arg, bool immediateFlag);
        friend void SA_CMP(Cpu& decoder, uint8_t dest, uint32_t arg, bool immediateFlag);
        friend void SA_LA(Cpu& decoder, uint8_t dest, uint32_t arg, bool immediateFlag);
        friend void SA_MOV_GE(Cpu& decoder, uint8_t dest, uint32_t arg, bool immediateFlag);
        friend void SA_MOV_G(Cpu& decoder, uint8_t dest, uint32_t arg, bool immediateFlag);
        friend void SA_MOV_LE(Cpu& decoder, uint8_t dest, uint32_t arg, bool immediateFlag);
        friend void SA_MOV_L(Cpu& decoder, uint8_t dest, uint32_t arg, bool immediateFlag);
        friend void SA_MOV_E(Cpu& decoder, uint8_t dest, uint32_t arg, bool immediateFlag);
        friend void SA_MOV_C(Cpu& decoder, uint8_t dest, uint32_t arg, bool immediateFlag);
        friend void SA_MOV_NE(Cpu& decoder, uint8_t dest, uint32_t arg, bool immediateFlag);
        friend void SA_MOV_FTOR(Cpu& decoder, uint8_t dest, uint32_t arg, bool immediateFlag);
        friend void FP_ADD(Cpu& decoder, uint8_t reg1, uint8_t reg2, uint8_t reg3);
        friend void FP_SUB(Cpu& decoder, uint8_t reg1, uint8_t reg2, uint8_t reg3);
        friend void FP_MUL(Cpu& decoder, uint8_t reg1, uint8_t reg2, uint8_t reg3);
        friend void FP_DIV(Cpu& decoder, uint8_t reg1, uint8_t reg2, uint8_t reg3);
        friend void FP_CMP(Cpu& decoder, uint8_t reg1, uint32_t reg2, bool immediateFlag);
        friend void FP_MOV(Cpu& decoder, uint8_t reg1, uint32_t reg2, bool immediateFlag);
        friend void FP_FTOR(Cpu& decoder, uint8_t reg1, uint32_t reg2, bool immediateFlag);
        friend void FP_S(Cpu& decoder, uint8_t addr, uint32_t reg, bool immediateFlag);
        friend void FP_L(Cpu& decoder, uint8_t addr, uint32_t reg, bool immediateFlag);
        friend void MEM_SB(Cpu& decoder, uint8_t reg, uint8_t addr, uint32_t offset, uint32_t count);
        friend void MEM_SH(Cpu& decoder, uint8_t reg, uint8_t addr, uint32_t offset, uint32_t count);
        friend void MEM_SW(Cpu& decoder, uint8_t reg, uint8_t addr, uint32_t offset, uint32_t count);
        friend void MEM_LB(Cpu& decoder, uint8_t reg, uint8_t addr, uint32_t offset, uint32_t count);
        friend void MEM_LH(Cpu& decoder, uint8_t reg, uint8_t addr, uint32_t offset, uint32_t count);
        friend void MEM_LW(Cpu& decoder, uint8_t reg, uint8_t addr, uint32_t offset, uint32_t count);
        friend void J_JMP(Cpu& decoder, uint32_t arg, bool immediate);
        friend void J_CALL(Cpu& decoder, uint32_t arg, bool immediate);
        friend void J_JGE(Cpu& decoder, uint32_t arg, bool immediate);
        friend void J_JG(Cpu& decoder, uint32_t arg, bool immediate);
        friend void J_JLE(Cpu& decoder, uint32_t arg, bool immediate);
        friend void J_JL(Cpu& decoder, uint32_t arg, bool immediate);
        friend void J_JE(Cpu& decoder, uint32_t arg, bool immediate);
        friend void J_JC(Cpu& decoder, uint32_t arg, bool immediate);
        friend void J_JNE(Cpu& decoder, uint32_t arg, bool immediate);
        friend void S_PUSH(Cpu& decoder, uint8_t reg, uint8_t registers[4]);
        friend void S_POP(Cpu& decoder, uint8_t reg, uint8_t registers[4]);
        friend void FP_RTOF(Cpu& cpu, uint8_t dest, uint32_t arg, bool immediateFlag);
};

#endif