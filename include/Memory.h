
#ifndef INCLUDE_MEMORY_H
#define INCLUDE_MEMORY_H

#include "defs.h"

struct Memory {
    public: 
        Memory(uint32_t memorySize) {
            wordMem = new uint32_t[memorySize / 4];
            halfWordMem = (uint16_t*)wordMem;
            byteMem = (uint8_t*)wordMem;
        }

        virtual ~Memory() {
            delete[] wordMem;
        }

        inline void setWord(uint32_t word, uint32_t location) {
            #ifdef DEBUG_MEMORY
            LTRACE("Write WORD[0x{x}]: {d} 0x{x}", location, word, word);
            #endif

            wordMem[location / 4] = word;
        }

        inline uint32_t getWord(uint32_t location) { 
            #ifdef DEBUG_MEMORY
            LTRACE("Read WORD[0x{x}]: {d} 0x{x}", location, wordMem[location / 4], wordMem[location / 4]);
            #endif

            return wordMem[location / 4];
        }

        inline void setHalf(uint16_t half, uint32_t location) {
            #ifdef DEBUG_MEMORY
            LTRACE("Write HALF[0x{x}]: {d} 0x{x}", location, half, half);
            #endif

            halfWordMem[location / 2] = half;
        }

        inline uint16_t getHalf(uint32_t location) {
            #ifdef DEBUG_MEMORY
            LTRACE("Read HALF[0x{x}]: {d} 0x{x}", location, halfWordMem[location / 2], halfWordMem[location / 2]);
            #endif

            return halfWordMem[location / 2];
        }

        inline void setByte(uint8_t byte, uint32_t location) {
            #ifdef DEBUG_MEMORY
            LTRACE("Write BYTE[0x{x}]: {d} 0x{x}", location, byte, byte);
            #endif

            byteMem[location] = byte;
        }

        inline uint8_t getByte(uint32_t location) {
            #ifdef DEBUG_MEMORY
            LTRACE("Read BYTE[0x{x}]: {d} 0x{x}", location, byteMem[location], byteMem[location]);
            #endif

            return byteMem[location];
        }

        uint8_t* getMemPointer() { 
            return byteMem;
        }

    private:
        uint32_t* wordMem;
        uint16_t* halfWordMem;
        uint8_t* byteMem;
};

#endif