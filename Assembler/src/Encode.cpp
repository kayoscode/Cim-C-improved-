#include "Encode.h"

#define ALOPCODE 0
#define SAOPCODE 1
#define FPAOPCODE 2
#define MEMOPCODE 3
#define JMPOPCODE 4
#define FPSAOPCODE 5
#define INTOPCODE 6
#define STACKOPCODE 7

void encodeJMP(uint32_t* buffer, int& index, JMPOps op, uint32_t arg, bool immediate) {
    buffer[index] = JMPOPCODE;
    buffer[index] |= (int)op << 3;
    buffer[index] |= (immediate) << 7;

    if(immediate) {
        buffer[index + 1] = arg;
        index += 2;
    }
    else {
        buffer[index] |= arg << 8;
        index += 1;
    }
}

void encodeSTACK(uint32_t* buffer, int& index, STACKOps op, uint32_t registerCount, uint8_t registers[4]) {
    buffer[index] = STACKOPCODE;
    buffer[index] |= (int)op << 3;
    buffer[index] |= registerCount << 4;

    if(registerCount > 4) {
        registerCount = 4;
    }

    for(int i = 0; i < registerCount; i++) {
        buffer[index] |= registers[i] << (7 + 6 * i);
    }

    index++;
}

void encodeINT(uint32_t* buffer, int& index, INTOps op, uint32_t interruptId) {
    buffer[index] = INTOPCODE;
    buffer[index] |= interruptId << 3;
    index++;
}

void encodeMEM(uint32_t* buffer, int& index, MEMOps op, uint8_t reg, uint8_t addrReg, uint32_t offset, uint32_t count) {
    buffer[index] = MEMOPCODE;
    buffer[index] |= (int)op << 3;
    buffer[index] |= reg << 6;
    buffer[index] |= addrReg << 11;
    buffer[index] |= 1 << 16;
    buffer[index] |= 1 << 17;
    index++;

    buffer[index] = offset;
    index++;

    buffer[index] = count;
    index++;
}

void encodeFPA(uint32_t* buffer, int& index, FPAOps op, uint8_t dest, uint8_t src1, uint8_t src2) {
    buffer[index] = FPAOPCODE;
    buffer[index] |= (((int)op) << 3);
    buffer[index] |= dest << 5;
    buffer[index] |= src1 << 10;
    buffer[index] |= src2 << 15;
    index += 1;
}

void encodeFPSA(uint32_t* buffer, int& index, FPSAOps op, uint8_t dest, uint32_t arg, bool immediate) {
    buffer[index] = FPSAOPCODE;
    buffer[index] |= (((int)op) << 3);
    buffer[index] |= immediate << 6;
    buffer[index] |= dest << 7;

    if(immediate) {
        buffer[index + 1] = arg;
        index += 2;
    }
    else {
        buffer[index] |= arg << 12;
        index += 1;
    }
}

void encodeAL(uint32_t* buffer, int& index, ALOps op, uint8_t dest, uint8_t src, uint32_t val, bool immediate) {
    buffer[index] = ALOPCODE;
    buffer[index] |= (((int)op) << 3);
    buffer[index] |= immediate << 7;
    buffer[index] |= (dest << 8);
    buffer[index] |= (src << 13);

    if(immediate) {
        buffer[index + 1] = val; 
        index += 2;
    }
    else {
        buffer[index] |= val << 18;
        index += 1;
    }
}

void encodeSA(uint32_t* buffer, int& index, SAOps op, uint8_t dest, uint32_t arg, bool immediate) {
    buffer[index] = SAOPCODE;
    buffer[index] |= (((int)op) << 3);
    buffer[index] |= immediate << 7;
    buffer[index] |= dest << 8;

    if(immediate) {
        buffer[index + 1] = arg;
        index += 2;
    }
    else {
        buffer[index] |= arg << 13;
        index += 1;
    }
}