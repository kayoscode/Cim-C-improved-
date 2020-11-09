#include <iostream>

//1 GB of memory
#define MEMORY_SIZE 1073741824 / 32

#include "Cpu.h"

int main() {
    LINIT();
    LTRACE("Starting emulator: memory size {d}", MEMORY_SIZE);
    Cpu cpu(MEMORY_SIZE);
    int instructionCount = 0;

    cpu.loadBinaryFile("out.bin", 0);

    LTRACE("START");
    while(!cpu.stopSet) {
        cpu.executeInstruction();
        instructionCount++;
    }
    LTRACE("STOP");

    cpu.printRegister(R0);
    cpu.printRegister(R1);
    cpu.printRegister(R2);
    LTRACE("EXECUTED {u} instructions", instructionCount);

    LTRACE("Closing program SUCCESS");
    return 0;
}