
#include "Cpu.h"

uint32_t** bitMasks = nullptr;
#define GETBITS(number, start, end) ((number & bitMasks[start][end]) >> start)
#define GETWORD(location) (mem.getWord(location))
#define GETHALF(location) (mem.getHalf(location))
#define GETBYTE(location) (mem.getByte(location))

#define FLTTOINT(data) (*(uint32_t*)&data)
#define INTTOFLT(data) (*(float*)&data)

#define INST_GETDATA(arg, immediateFlag, op) op = (immediateFlag? arg : cpu.registers[arg].value)
#define INST_GETDATA_WORD(arg, immediateFlag, op) (INST_GETDATA(arg, immediateFlag, op))
#define INST_GETDATA_BYTE(arg, immediateFlag, op) (INST_GETDATA(arg, immediateFlag, op) & 0xFF)
#define INST_GETDATA_HALF(arg, immediateFlag, op) (INST_GETDATA(arg, immediateFlag, op) & 0xFFFF)
#define INST_GETFP(arg, immediateFlag, op) op = (immediateFlag? *((float*)&arg) : cpu.fRegisters[arg].value)

void initBitMasks() {
    if(bitMasks == nullptr) {
        bitMasks = new uint32_t*[33];

        for (int i = 0; i < 33; i++) {
            bitMasks[i] = new uint32_t[33];
        }

        for (int i = 0; i < 33; i++) {
            for (int j = 0; j < 33; j++) {
                int v = 0;

                for (int k = i; k < j; k++) {
                    v |= (1 << k);
                }

                bitMasks[i][j] = v;
            }
        }
    }
}

static void AL_LSL(Cpu& cpu, uint8_t dest, uint8_t src, uint32_t arg, bool immediateFlag) {
#ifdef DEBUG_INSTRUCTIONS
    if(!immediateFlag) {
        LINFO("LSL {s} {s} {s}", registerNames[dest], registerNames[src], registerNames[arg]);
    }
    else {
        LINFO("{s} {s} {d}", registerNames[dest], registerNames[src], arg);
    }
#endif
    uint32_t op;
    INST_GETDATA(arg, immediateFlag, op);
	cpu.registers[dest].value = ((int32_t)cpu.registers[src].value) << op;

	cpu.setZeroAndNeg(cpu.registers[dest].value);
}

static void AL_LSR(Cpu& cpu, uint8_t dest, uint8_t src, uint32_t arg, bool immediateFlag) {
#ifdef DEBUG_INSTRUCTIONS
    if(!immediateFlag) {
        LINFO("LSR {s} {s} {s}", registerNames[dest], registerNames[src], registerNames[arg]);
    }
    else {
        LINFO("LSR {s} {s} {d}", registerNames[dest], registerNames[src], arg);
    }
#endif

    uint32_t op;
    INST_GETDATA(arg, immediateFlag, op);
	cpu.registers[dest].value = ((int32_t)cpu.registers[src].value) >> op;

	cpu.setZeroAndNeg(cpu.registers[dest].value);
}

static void AL_ADD(Cpu& cpu, uint8_t dest, uint8_t src, uint32_t arg, bool immediateFlag) {
#ifdef DEBUG_INSTRUCTIONS
    if(!immediateFlag) {
        LINFO("ADD {s} {s} {s}", registerNames[dest], registerNames[src], registerNames[arg]);
    }
    else {
        LINFO("ADD {s} {s} {d}", registerNames[dest], registerNames[src], arg);
    }
#endif

	uint32_t op;
    INST_GETDATA(arg, immediateFlag, op);
	cpu.setOverFlow(cpu.registers[src].value, op);
	cpu.registers[dest].value = ((int32_t)cpu.registers[src].value) + op;
	cpu.setZeroAndNeg(cpu.registers[dest].value);
}

static void AL_SUB(Cpu& cpu, uint8_t dest, uint8_t src, uint32_t arg, bool immediateFlag) {
#ifdef DEBUG_INSTRUCTIONS
    if(!immediateFlag) {
        LINFO("SUB {s} {s} {s}", registerNames[dest], registerNames[src], registerNames[arg]);
    }
    else {
        LINFO("SUB {s} {s} {d}", registerNames[dest], registerNames[src], arg);
    }
#endif
	uint32_t op;
    INST_GETDATA(arg, immediateFlag, op);
	cpu.registers[dest].value = ((int32_t)cpu.registers[src].value) - op;
	cpu.setZeroAndNeg(cpu.registers[dest].value);
}

static void AL_MUL(Cpu& cpu, uint8_t dest, uint8_t src, uint32_t arg, bool immediateFlag) {
#ifdef DEBUG_INSTRUCTIONS
    if(!immediateFlag) {
        LINFO("MUL {s} {s} {s}", registerNames[dest], registerNames[src], registerNames[arg]);
    }
    else {
        LINFO("MUL {s} {s} {d}", registerNames[dest], registerNames[src], arg);
    }
#endif
	uint32_t op;
    INST_GETDATA(arg, immediateFlag, op);
	cpu.registers[dest].value = ((int32_t)cpu.registers[src].value) * op;
	cpu.setZeroAndNeg(cpu.registers[dest].value);
}

static void AL_DIV(Cpu& cpu, uint8_t dest, uint8_t src, uint32_t arg, bool immediateFlag) {
#ifdef DEBUG_INSTRUCTIONS
    if(!immediateFlag) {
        LINFO("DIV {s} {s} {s}", registerNames[dest], registerNames[src], registerNames[arg]);
    }
    else {
        LINFO("DIV {s} {s} {d}", registerNames[dest], registerNames[src], arg);
    }
#endif
    uint32_t op;
	uint32_t remainder;
	uint32_t value;

    INST_GETDATA(arg, immediateFlag, op);

	if (op != 0) {
		value = ((int32_t)cpu.registers[src].value) / op;
		remainder = ((int32_t)cpu.registers[src].value) % op;
	}
	else {
		value = 0;
		remainder = 0;
		//HWINT(HWINT_DIV_ZERO);
        LCRITICAL("DIVIDE BY ZERO");
	}

	cpu.registers[dest].value = value;

	if (cpu.registers[dest].value == 0) {
		cpu.setFlag(FL_ZERO);
	}

	cpu.registers[R15].value = remainder;

	cpu.setZeroAndNeg(cpu.registers[dest].value);
}

static void AL_ULSL(Cpu& cpu, uint8_t dest, uint8_t src, uint32_t arg, bool immediateFlag) {
#ifdef DEBUG_INSTRUCTIONS
    if(!immediateFlag) {
        LINFO("ULSL {s} {s} {s}", registerNames[dest], registerNames[src], registerNames[arg]);
    }
    else {
        LINFO("ULSL {s} {s} {d}", registerNames[dest], registerNames[src], arg);
    }
#endif
	uint32_t op;
    INST_GETDATA(arg, immediateFlag, op);
	cpu.registers[dest].value = (cpu.registers[src].value) << op;

	cpu.setZero(cpu.registers[dest].value);
}

static void AL_ULSR(Cpu& cpu, uint8_t dest, uint8_t src, uint32_t arg, bool immediateFlag) {
#ifdef DEBUG_INSTRUCTIONS
    if(!immediateFlag) {
        LINFO("ULSR {s} {s} {s}", registerNames[dest], registerNames[src], registerNames[arg]);
    }
    else {
        LINFO("ULSR {s} {s} {d}", registerNames[dest], registerNames[src], arg);
    }
#endif
	uint32_t op;
    INST_GETDATA(arg, immediateFlag, op);
	cpu.registers[dest].value = (cpu.registers[src].value) >> op;

	cpu.setZero(cpu.registers[dest].value);
}

static void AL_UADD(Cpu& cpu, uint8_t dest, uint8_t src, uint32_t arg, bool immediateFlag) {
#ifdef DEBUG_INSTRUCTIONS
    if(!immediateFlag) {
        LINFO("UADD {s} {s} {s}", registerNames[dest], registerNames[src], registerNames[arg]);
    }
    else {
        LINFO("UADD {s} {s} {d}", registerNames[dest], registerNames[src], arg);
    }
#endif
    uint32_t op;
    INST_GETDATA(arg, immediateFlag, op);
	cpu.setCarry(cpu.registers[src].value, op);
	cpu.registers[dest].value = (cpu.registers[src].value) + op;
	cpu.setZero(cpu.registers[dest].value);
}

static void AL_USUB(Cpu& cpu, uint8_t dest, uint8_t src, uint32_t arg, bool immediateFlag) {
#ifdef DEBUG_INSTRUCTIONS
    if(!immediateFlag) {
        LINFO("USUB {s} {s} {s}", registerNames[dest], registerNames[src], registerNames[arg]);
    }
    else {
        LINFO("USUB {s} {s} {d}", registerNames[dest], registerNames[src], arg);
    }
#endif
    uint32_t op;
    INST_GETDATA(arg, immediateFlag, op);
	cpu.registers[dest].value = (cpu.registers[src].value) - op;
	cpu.setZero(cpu.registers[dest].value);
}

static void AL_AND(Cpu& cpu, uint8_t dest, uint8_t src, uint32_t arg, bool immediateFlag) {
#ifdef DEBUG_INSTRUCTIONS
    if(!immediateFlag) {
        LINFO("AND {s} {s} {s}", registerNames[dest], registerNames[src], registerNames[arg]);
    }
    else {
        LINFO("AND {s} {s} {d}", registerNames[dest], registerNames[src], arg);
    }
#endif
    uint32_t op;
    INST_GETDATA(arg, immediateFlag, op);
	cpu.registers[dest].value = (cpu.registers[src].value) & op;

	cpu.setZero(cpu.registers[dest].value);
}

static void AL_OR(Cpu& cpu, uint8_t dest, uint8_t src, uint32_t arg, bool immediateFlag) {
#ifdef DEBUG_INSTRUCTIONS
    if(!immediateFlag) {
        LINFO("OR {s} {s} {s}", registerNames[dest], registerNames[src], registerNames[arg]);
    }
    else {
        LINFO("OR {s} {s} {d}", registerNames[dest], registerNames[src], arg);
    }
#endif

    uint32_t op;
    INST_GETDATA(arg, immediateFlag, op);
	cpu.registers[dest].value = (cpu.registers[src].value) | op;

	cpu.setZero(cpu.registers[dest].value);
}

static void AL_XOR(Cpu& cpu, uint8_t dest, uint8_t src, uint32_t arg, bool immediateFlag) {
#ifdef DEBUG_INSTRUCTIONS
    if(!immediateFlag) {
        LINFO("XOR {s} {s} {s}", registerNames[dest], registerNames[src], registerNames[arg]);
    }
    else {
        LINFO("XOR {s} {s} {d}", registerNames[dest], registerNames[src], arg);
    }
#endif
    uint32_t op;
    INST_GETDATA(arg, immediateFlag, op);
	cpu.registers[dest].value = (cpu.registers[src].value) ^ op;

	cpu.setZero(cpu.registers[dest].value);
}

static void SA_MOV(Cpu& cpu, uint8_t dest, uint32_t arg, bool immediateFlag) {
#ifdef DEBUG_INSTRUCTIONS
    if(!immediateFlag) {
        LINFO("MOV {s} {s}", registerNames[dest], registerNames[arg]);
    }
    else {
        LINFO("MOV {s} {d}", registerNames[dest], arg);
    }
#endif
    uint32_t op;
    INST_GETDATA(arg, immediateFlag, op);
	cpu.registers[dest].value = op;
}

static void SA_CMP(Cpu& cpu, uint8_t dest, uint32_t arg, bool immediateFlag) {
#ifdef DEBUG_INSTRUCTIONS
    if(!immediateFlag) {
        LINFO("CMP {s} {s}", registerNames[dest], registerNames[arg]);
    }
    else {
        LINFO("CMP {s} {d}", registerNames[dest], arg);
    }
#endif

	uint32_t op;
    INST_GETDATA(arg, immediateFlag, op);
	uint32_t value = ((int32_t)cpu.registers[dest].value) - op;

	cpu.setZeroAndNeg(value);
}

static void SA_LA(Cpu& cpu, uint8_t dest, uint32_t arg, bool immediateFlag) {
    //convert program relative address to actual address
#ifdef DEBUG_INSTRUCTIONS
    if(!immediateFlag) {
        LINFO("LA {s} {s}", registerNames[dest], registerNames[arg]);
    }
    else {
        LINFO("LA {s} {d}", registerNames[dest], (int32_t)arg);
    }
#endif
    uint32_t op;
    INST_GETDATA(arg, immediateFlag, op);
	uint32_t value = cpu.registers[PC].value + op;

	cpu.registers[dest].value = value;
}

static void SA_MOV_GE(Cpu& cpu, uint8_t dest, uint32_t arg, bool immediateFlag) {
#ifdef DEBUG_INSTRUCTIONS
    if(!immediateFlag) {
        LINFO("MOV.GE {s} {s}", registerNames[dest], registerNames[arg]);
    }
    else {
        LINFO("MOV.GE {s} {d}", registerNames[dest], arg);
    }
#endif
	if (cpu.isFlagSet(FL_ZERO) | !cpu.isFlagSet(FL_NEGATIVE)) {
        uint32_t op;
        INST_GETDATA(arg, immediateFlag, op);
        cpu.registers[dest].value = op;
	}
}

static void SA_MOV_G(Cpu& cpu, uint8_t dest, uint32_t arg, bool immediateFlag) {
#ifdef DEBUG_INSTRUCTIONS
    if(!immediateFlag) {
        LINFO("MOV.G {s} {s}", registerNames[dest], registerNames[arg]);
    }
    else {
        LINFO("MOV.G {s} {d}", registerNames[dest], arg);
    }
#endif
	if (!cpu.isFlagSet(FL_NEGATIVE) && !cpu.isFlagSet(FL_ZERO)) {
        uint32_t op;
        INST_GETDATA(arg, immediateFlag, op);
        cpu.registers[dest].value = op;
	}
}

static void SA_MOV_LE(Cpu& cpu, uint8_t dest, uint32_t arg, bool immediateFlag) {
#ifdef DEBUG_INSTRUCTIONS
    if(!immediateFlag) {
        LINFO("MOV.LE {s} {s}", registerNames[dest], registerNames[arg]);
    }
    else {
        LINFO("MOV.LE {s} {d}", registerNames[dest], arg);
    }
#endif
	if (cpu.isFlagSet(FL_ZERO) || cpu.isFlagSet(FL_NEGATIVE)) {
        uint32_t op;
        INST_GETDATA(arg, immediateFlag, op);
        cpu.registers[dest].value = op;
	}
}

static void SA_MOV_L(Cpu& cpu, uint8_t dest, uint32_t arg, bool immediateFlag) {
#ifdef DEBUG_INSTRUCTIONS
    if(!immediateFlag) {
        LINFO("MOV.L {s} {s}", registerNames[dest], registerNames[arg]);
    }
    else {
        LINFO("MOV.L {s} {d}", registerNames[dest], arg);
    }
#endif
    if (cpu.isFlagSet(FL_NEGATIVE) && !cpu.isFlagSet(FL_ZERO)) {
        uint32_t op;
        INST_GETDATA(arg, immediateFlag, op);
        cpu.registers[dest].value = op;
	}
}

static void SA_MOV_E(Cpu& cpu, uint8_t dest, uint32_t arg, bool immediateFlag) {
#ifdef DEBUG_INSTRUCTIONS
    if(!immediateFlag) {
        LINFO("MOV.E {s} {s}", registerNames[dest], registerNames[arg]);
    }
    else {
        LINFO("MOV.E {s} {d}", registerNames[dest], arg);
    }
#endif
    if(cpu.isFlagSet(FL_ZERO)) {
        uint32_t op;
        INST_GETDATA(arg, immediateFlag, op);
        cpu.registers[dest].value = op;
    }
}

static void SA_MOV_C(Cpu& cpu, uint8_t dest, uint32_t arg, bool immediateFlag) {
#ifdef DEBUG_INSTRUCTIONS
    if(!immediateFlag) {
        LINFO("MOV.C {s} {s}", registerNames[dest], registerNames[arg]);
    }
    else {
        LINFO("MOV.C {s} {d}", registerNames[dest], arg);
    }
#endif
    if(cpu.isFlagSet(FL_CARRY)) {
        uint32_t op;
        INST_GETDATA(arg, immediateFlag, op);
        cpu.registers[dest].value = op;
    }
}

static void SA_MOV_NE(Cpu& cpu, uint8_t dest, uint32_t arg, bool immediateFlag) {
#ifdef DEBUG_INSTRUCTIONS
    if(!immediateFlag) {
        LINFO("MOV.NE {s} {s}", registerNames[dest], registerNames[arg]);
    }
    else {
        LINFO("MOV.NE {s} {d}", registerNames[dest], arg);
    }
#endif
    if(!cpu.isFlagSet(FL_ZERO)) {
        uint32_t op;
        INST_GETDATA(arg, immediateFlag, op);
        cpu.registers[dest].value = op;
    }
}

static void FP_RTOF(Cpu& cpu, uint8_t dest, uint32_t arg, bool immediateFlag) {
#ifdef DEBUG_INSTRUCTIONS
    LINFO("RTOF {s} {s}", fregisterNames[dest], registerNames[arg]);
#endif
    cpu.fRegisters[dest].value = (float)cpu.registers[arg].value;
}

static void SA_MOV_FTOR(Cpu& cpu, uint8_t dest, uint32_t arg, bool immediateFlag) {
#ifdef DEBUG_INSTRUCTIONS
    LINFO("FTOR {s} {s}", registerNames[dest], fregisterNames[arg]);
#endif
    cpu.registers[dest].value = (int)cpu.fRegisters[arg].value;
}

static void FP_ADD(Cpu& cpu, uint8_t reg1, uint8_t reg2, uint8_t reg3) {
#ifdef DEBUG_INSTRUCTIONS
    LINFO("FADD {s} {s} {s}", fregisterNames[reg1], fregisterNames[reg2], fregisterNames[reg3]);
#endif
	cpu.fRegisters[reg1].value = cpu.fRegisters[reg2].value + cpu.fRegisters[reg3].value;
	cpu.setZeroAndNeg(cpu.fRegisters[reg1].value);
}

static void FP_SUB(Cpu& cpu, uint8_t reg1, uint8_t reg2, uint8_t reg3) {
#ifdef DEBUG_INSTRUCTIONS
    LINFO("FSUB {s} {s} {s}", fregisterNames[reg1], fregisterNames[reg2], fregisterNames[reg3]);
#endif
	cpu.fRegisters[reg1].value = cpu.fRegisters[reg2].value - cpu.fRegisters[reg3].value;
	cpu.setZeroAndNeg(cpu.fRegisters[reg1].value);
}

static void FP_MUL(Cpu& cpu, uint8_t reg1, uint8_t reg2, uint8_t reg3) {
#ifdef DEBUG_INSTRUCTIONS
    LINFO("FMUL {s} {s} {s}", fregisterNames[reg1], fregisterNames[reg2], fregisterNames[reg3]);
#endif
	cpu.fRegisters[reg1].value = cpu.fRegisters[reg2].value * cpu.fRegisters[reg3].value;
	cpu.setZeroAndNeg(cpu.fRegisters[reg1].value);
}

static void FP_DIV(Cpu& cpu, uint8_t reg1, uint8_t reg2, uint8_t reg3) {
#ifdef DEBUG_INSTRUCTIONS
    LINFO("FDIV {s} {s} {s}", fregisterNames[reg1], fregisterNames[reg2], fregisterNames[reg3]);
#endif
	cpu.fRegisters[reg1].value = cpu.fRegisters[reg2].value / cpu.fRegisters[reg3].value;
	cpu.setZeroAndNeg(cpu.fRegisters[reg1].value);
}

static void FP_CMP(Cpu& cpu, uint8_t reg1, uint32_t reg2, bool immediateFlag){
#ifdef DEBUG_INSTRUCTIONS
    if(!immediateFlag) {
        LINFO("FCMP {s} {s}", fregisterNames[reg1], fregisterNames[reg2]);
    }
    else {
        LINFO("FCMP {s} {f}", fregisterNames[reg1], *(float*)&reg2);
    }
#endif
	float op;
    INST_GETFP(reg2, immediateFlag, op);
	float value = cpu.fRegisters[reg1].value - op;

	cpu.setZeroAndNeg(value);
}

static void FP_MOV(Cpu& cpu, uint8_t reg1, uint32_t reg2, bool immediateFlag){
#ifdef DEBUG_INSTRUCTIONS
    if(!immediateFlag) {
        LINFO("FMOV {s} {s}", fregisterNames[reg1], fregisterNames[reg2]);
    }
    else {
        LINFO("FMOV {s} {f}", fregisterNames[reg1], *(float*)&reg2);
    }
#endif
	float op;
    INST_GETFP(reg2, immediateFlag, op);

	cpu.fRegisters[reg1].value = op;
}

static void FP_S(Cpu& cpu, uint8_t reg, uint32_t immreg, bool immediateFlag) {
#ifdef DEBUG_INSTRUCTIONS
    LINFO("FS {s} {d}", fregisterNames[reg], registerNames[immreg]);
#endif
    float op;
    INST_GETFP(immreg, immediateFlag, op);
	uint32_t ieee = FLTTOINT(op);
	uint32_t addr = cpu.registers[reg].value;

    cpu.mem.setWord(ieee, addr);
}

static void FP_L(Cpu& cpu, uint8_t reg, uint32_t immreg, bool immediateFlag) {
#ifdef DEBUG_INSTRUCTIONS
    LINFO("FL {s} {d}", fregisterNames[reg], registerNames[immreg]);
#endif
	uint32_t addr = cpu.registers[reg].value;
    uint32_t word = cpu.mem.getWord(addr);
	float value = INTTOFLT(word);

	cpu.fRegisters[immreg] = value;
}

static void MEM_SB(Cpu& cpu, uint8_t addr, uint8_t reg, uint32_t offset, uint32_t count) {
#ifdef DEBUG_INSTRUCTIONS
    if(count < 2) {
        LINFO("SB {s} {s}[{d}]", registerNames[reg], registerNames[addr], (int32_t)offset);
    }
    else {
        LINFO("SB {s} {s}[{d}]:{d}", registerNames[reg], registerNames[addr], (int32_t)offset, count);
    }
#endif
    for(uint32_t i = 0; i < count; i++) {
        uint8_t byte = cpu.registers[reg].value & 0xFF;
        uint32_t address = cpu.registers[addr].value;

        cpu.mem.setByte(byte, address + offset + i);
    }
}

static void MEM_SH(Cpu& cpu, uint8_t addr, uint8_t reg, uint32_t offset, uint32_t count) {
#ifdef DEBUG_INSTRUCTIONS
    if(count < 2) {
        LINFO("SH {s} {s}[{d}]", registerNames[reg], registerNames[addr], (int32_t)offset);
    }
    else {
        LINFO("SH {s} {s}[{d}]:{d}", registerNames[reg], registerNames[addr], (int32_t)offset, count);
    }
#endif
    for(uint32_t i = 0; i < count; i++) {
        uint16_t byte = cpu.registers[reg].value & 0xFFFF;
        uint32_t address = cpu.registers[addr].value;

        cpu.mem.setHalf(byte, address + offset + i * 2);
    }
}

static void MEM_SW(Cpu& cpu, uint8_t addr, uint8_t reg, uint32_t offset, uint32_t count) {
#ifdef DEBUG_INSTRUCTIONS
    if(count < 2){
        LINFO("SW {s} {s}[{d}]", registerNames[reg], registerNames[addr], (int32_t)offset);
    }
    else {
        LINFO("SW {s} {s}[{d}]:{d}", registerNames[reg], registerNames[addr], (int32_t)offset, count);
    }
#endif
    for(uint32_t i = 0; i < count; i++) {
        uint16_t byte = cpu.registers[reg].value;
        uint32_t address = cpu.registers[addr].value;

        cpu.mem.setWord(byte, address + offset + i * 4);
    }
}

static void MEM_LB(Cpu& cpu, uint8_t addr, uint8_t reg, uint32_t offset, uint32_t count) {
#ifdef DEBUG_INSTRUCTIONS
    LINFO("LB {s} {s}[{d}]", registerNames[reg], registerNames[addr], (int32_t)offset);
#endif
    uint8_t byte;
    uint32_t address = cpu.registers[addr].value;

    byte = cpu.mem.getByte(address + offset);
    cpu.registers[reg].value = (int8_t)byte;
}

static void MEM_LH(Cpu& cpu, uint8_t addr, uint8_t reg, uint32_t offset, uint32_t count) {
#ifdef DEBUG_INSTRUCTIONS
    LINFO("LH {s} {s}[{d}]", registerNames[reg], registerNames[addr], (int32_t)offset);
#endif
    uint16_t byte;
    uint32_t address = cpu.registers[addr].value;

    byte = cpu.mem.getByte(address + offset * 2);
    cpu.registers[reg].value = (int16_t)byte;
}

static void MEM_LW(Cpu& cpu, uint8_t addr, uint8_t reg, uint32_t offset, uint32_t count) {
#ifdef DEBUG_INSTRUCTIONS
    LINFO("LW {s} {s}[{d}]", registerNames[reg], registerNames[addr], (int32_t)offset);
#endif
    uint32_t word;
    uint32_t address = cpu.registers[addr].value;

    word = cpu.mem.getByte(address + offset * 4);
    cpu.registers[reg].value = (int32_t)word;
}

static void J_JMP(Cpu& cpu, uint32_t arg, bool immediate) {
#ifdef DEBUG_INSTRUCTIONS
    if(!immediate) {
        LINFO("JMP {s}", registerNames[arg]);
    }
    else {
        LINFO("JMP {d}", (int32_t)arg);
    }
#endif
	uint32_t op;
	uint32_t sop;
    INST_GETDATA(arg, immediate, op);
	sop = (int32_t)op;

	if (immediate) {
		cpu.registers[PC].value += sop;
	}
	else {
		cpu.registers[PC].value = sop;
	}
}

static void J_CALL(Cpu& cpu, uint32_t arg, bool immediate) {
#ifdef DEBUG_INSTRUCTIONS
    if(!immediate) {
        LINFO("CALL {s}", registerNames[arg]);
    }
    else {
        LINFO("CALL {d}", (int32_t)arg);
    }
#endif
    cpu.registers[RA].value = cpu.registers[PC].value;

    uint32_t op;
	uint32_t sop;
    INST_GETDATA(arg, immediate, op);
	sop = (int32_t)op;

	if (immediate) {
		cpu.registers[PC].value += sop;
	}
	else {
		cpu.registers[PC].value = sop;
	}
}

static void J_JGE(Cpu& cpu, uint32_t arg, bool immediate) {
#ifdef DEBUG_INSTRUCTIONS
    if(!immediate) {
        LINFO("JGE {s}", registerNames[arg]);
    }
    else {
        LINFO("JGE {d}", (int32_t)arg);
    }
#endif
	if (cpu.isFlagSet(FL_ZERO) | !cpu.isFlagSet(FL_NEGATIVE)) {
        uint32_t op;
        uint32_t sop;
        INST_GETDATA(arg, immediate, op);
        sop = (int32_t)op;

        if (immediate) {
            cpu.registers[PC].value += sop;
        }
        else {
            cpu.registers[PC].value = sop;
        }
	}
}

static void J_JG(Cpu& cpu, uint32_t arg, bool immediate) {
#ifdef DEBUG_INSTRUCTIONS
    if(!immediate) {
        LINFO("JG {s}", registerNames[arg]);
    }
    else {
        LINFO("JG {d}", (int32_t)arg);
    }
#endif
    if(!cpu.isFlagSet(FL_NEGATIVE) && !cpu.isFlagSet(FL_ZERO)) {
        uint32_t op;
        uint32_t sop;
        INST_GETDATA(arg, immediate, op);
        sop = (int32_t)op;

        if (immediate) {
            cpu.registers[PC].value += sop;
        }
        else {
            cpu.registers[PC].value = sop;
        }
	}
}

static void J_JLE(Cpu& cpu, uint32_t arg, bool immediate) {
#ifdef DEBUG_INSTRUCTIONS
    if(!immediate) {
        LINFO("JLE {s}", registerNames[arg]);
    }
    else {
        LINFO("JLE {d}", (int32_t)arg);
    }
#endif
    if (cpu.isFlagSet(FL_ZERO) | cpu.isFlagSet(FL_NEGATIVE)) {
        uint32_t op;
        uint32_t sop;
        INST_GETDATA(arg, immediate, op);
        sop = (int32_t)op;

        if (immediate) {
            cpu.registers[PC].value += sop;
        }
        else {
            cpu.registers[PC].value = sop;
        }
	}
}

static void J_JL(Cpu& cpu, uint32_t arg, bool immediate) {
#ifdef DEBUG_INSTRUCTIONS
    if(!immediate)
        LINFO("JL {s}", registerNames[arg]);
    else 
        LINFO("JL {d}", (int32_t)arg);
#endif
	if (cpu.isFlagSet(FL_NEGATIVE) && !cpu.isFlagSet(FL_ZERO)) {
        uint32_t op;
        uint32_t sop;
        INST_GETDATA(arg, immediate, op);
        sop = (int32_t)op;

        if (immediate) {
            cpu.registers[PC].value += sop;
        }
        else {
            cpu.registers[PC].value = sop;
        }
	}
}

static void J_JE(Cpu& cpu, uint32_t arg, bool immediate) {
#ifdef DEBUG_INSTRUCTIONS
    if(!immediate) 
        LINFO("JE {s}", registerNames[arg]);
    else 
        LINFO("JE {d}", (int32_t)arg);
#endif
    if(cpu.isFlagSet(FL_ZERO)) {
        uint32_t op;
        uint32_t sop;
        INST_GETDATA(arg, immediate, op);
        sop = (int32_t)op;

        if (immediate) {
            cpu.registers[PC].value += sop;
        }
        else {
            cpu.registers[PC].value = sop;
        }
    }
}

static void J_JC(Cpu& cpu, uint32_t arg, bool immediate) {
#ifdef DEBUG_INSTRUCTIONS
    if(!immediate)
        LINFO("JC {s}", registerNames[arg]);
    else
        LINFO("JC {d}", (int32_t)arg);
#endif
    if(cpu.isFlagSet(FL_CARRY)) {
        uint32_t op;
        uint32_t sop;
        INST_GETDATA(arg, immediate, op);
        sop = (int32_t)op;

        if (immediate) {
            cpu.registers[PC].value += sop;
        }
        else {
            cpu.registers[PC].value = sop;
        }
    }
}

static void J_JNE(Cpu& cpu, uint32_t arg, bool immediate) {
#ifdef DEBUG_INSTRUCTIONS
    if(!immediate) {
        LINFO("JNE {s}", registerNames[arg]);
    }
    else {
        LINFO("JNE {d}", (int32_t)arg);
    }
#endif
    if(!cpu.isFlagSet(FL_ZERO)) {
        uint32_t op;
        uint32_t sop;
        INST_GETDATA(arg, immediate, op);
        sop = (int32_t)op;

        if (immediate) {
            cpu.registers[PC].value += sop;
        }
        else {
            cpu.registers[PC].value = sop;
        }
    }
}

static void S_PUSH(Cpu& cpu, uint8_t registerCount, uint8_t registers[4]) {
#ifdef DEBUG_INSTRUCTIONS
    if(registerCount == 1) {
        if((registers[0] >> 5)  & 1) {
            LINFO("PUSH {s}", fregisterNames[registers[0] & ~(1 << 5)]);
        }
        else {
            LINFO("PUSH {s}", registerNames[registers[0]]);
        }
    }
    else {
        std::string instruction("[");

        for(int i = 0; i < registerCount; i++) {
            if((registers[i] >> 5) & 1){
                instruction += (fregisterNames[registers[i] & ~(1 << 5)]);
            }
            else {
                instruction += registerNames[registers[i]];
            }

            if(i != registerCount - 1) {
                instruction += ", ";
            }
        }

        instruction.push_back(']');

        LINFO("PUSH {s}", instruction.c_str());
    }
#endif
    for(int i = 0; i < registerCount; i++) {
        cpu.registers[SP].value += 4;

        if((registers[i] >> 5) & 1) {
            float value = cpu.fRegisters[registers[i] & ~(1 << 5)].value;
            cpu.mem.setWord(*(uint32_t*)&value, cpu.registers[SP].value);
        }
        else {
            cpu.mem.setWord(cpu.registers[registers[i]].value, cpu.registers[SP].value);
        }
    }
}

static void S_POP(Cpu& cpu, uint8_t registerCount, uint8_t registers[4]) {
#ifdef DEBUG_INSTRUCTIONS
    if(registerCount == 1) {
        if((registers[0] >> 5)  & 1) {
            LINFO("POP {s}", fregisterNames[registers[0] & ~(1 << 5)]);
        }
        else {
            LINFO("POP {s}", registerNames[registers[0]]);
        }
    }
    else {
        std::string instruction("[");

        for(int i = 0; i < registerCount; i++) {
            if((registers[i] >> 5) & 1){
                instruction += (fregisterNames[registers[i] & ~(1 << 5)]);
            }
            else {
                instruction += registerNames[registers[i]];
            }

            if(i != registerCount - 1) {
                instruction += ", ";
            }
        }

        instruction.push_back(']');

        LINFO("POP {s}", instruction.c_str());
    }
#endif
    for(int i = registerCount - 1; i >= 0; i--) {
        uint32_t value = cpu.mem.getWord(cpu.registers[SP].value);

        if((registers[i] >> 5) & 1) {
            cpu.fRegisters[registers[i] & ~(1 << 5)] = *(float*)&value;
        }
        else {
            cpu.registers[registers[i]] = value;
        }

        cpu.registers[SP].value -= 4;
    }
}

#define ARITH_COUNT 13
void(*arithmeticLogicFunctions[ARITH_COUNT])(Cpu&, uint8_t, uint8_t, uint32_t, bool) {
	AL_LSL, AL_LSR, AL_ADD,
	AL_SUB, AL_MUL, AL_DIV, 
	AL_ULSL, AL_ULSR, AL_UADD,
	AL_USUB, AL_AND, AL_OR, AL_XOR
};	

#define SPECIAL_ARITH_COUNT 16
void(*specialArithmeticFunctions[SPECIAL_ARITH_COUNT])(Cpu&, uint8_t, uint32_t, bool) {
	SA_MOV, SA_CMP,
	SA_LA, SA_MOV_GE, SA_MOV_G, 
	SA_MOV_LE, SA_MOV_L, SA_MOV_E,
	SA_MOV_C, SA_MOV_NE, 

	//added 2 of these cause I am going to be using the immediate flag as part of the op code and it needs to be at 11
	SA_MOV_FTOR,
	SA_MOV_FTOR,
	SA_MOV_FTOR,
	SA_MOV_FTOR,
	SA_MOV_FTOR,
	SA_MOV_FTOR,
};

#define FLT_ARITHMETIC_COUNT 4
void(*floatingPointArithmeticFunctions[FLT_ARITHMETIC_COUNT])(Cpu&, uint8_t, uint8_t, uint8_t) {
	FP_ADD, FP_SUB, FP_MUL, FP_DIV
};

#define FLT_SPECIAL_ARITH_COUNT 8
void(*floatingPointSpecialArithmetic[FLT_SPECIAL_ARITH_COUNT])(Cpu&, uint8_t, uint32_t, bool) {
	FP_MOV, FP_CMP,
	FP_S, FP_L,

	FP_RTOF,
	FP_RTOF,
	FP_RTOF,
	FP_RTOF,
};

#define MEMORY_COUNT 6
void(*memoryInstructions[MEMORY_COUNT])(Cpu&, uint8_t, uint8_t, uint32_t, uint32_t) {
	MEM_SB, MEM_SH,
	MEM_SW, MEM_LB,
	MEM_LH, MEM_LW
};

#define JUMP_COUNT 9
void(*jumpInstructions[JUMP_COUNT])(Cpu&, uint32_t, bool) {
	J_JMP, J_JGE, J_JG, 
	J_JLE, J_JL, J_JE,
	J_JC, J_JNE, J_CALL
};

#define STACK_COUNT 4
void(*stackInstructions[STACK_COUNT])(Cpu&, uint8_t, uint8_t[4]) {
	S_PUSH, S_POP
};

static void decodeStackInstruction(Cpu& cpu, uint32_t instruction, uint32_t& programCounter) {
	uint32_t opCode = GETBITS(instruction, 3, 4);
    uint32_t registerCount = GETBITS(instruction, 4, 7);
    uint8_t registers[4];

    for(uint32_t i = 0; i < registerCount; i++) {
        registers[i] = GETBITS(instruction, (7 + 6 * i), (7 + 6 * i + 6));
    }

	stackInstructions[opCode](cpu, registerCount, registers);
}

static void decodeArithmeticLogic(Cpu& cpu, uint32_t instruction, uint32_t& programCounter) {
	uint32_t opCode = GETBITS(instruction, 3, 7);
	bool immediateFlag = (bool)GETBITS(instruction, 7, 8);
	uint8_t destinationRegister = GETBITS(instruction, 8, 13);
	uint8_t sourceRegister = GETBITS(instruction, 13, 18);
	uint32_t immReg;

	if (immediateFlag) {
		immReg = cpu.mem.getWord(programCounter);
		programCounter += 4;
	}
	else {
		immReg = GETBITS(instruction, 18, 23);
	}

	if (opCode < ARITH_COUNT) {
		arithmeticLogicFunctions[opCode](cpu, destinationRegister, sourceRegister, immReg, immediateFlag);
    }
	else {
		//cpu.computer->HWINT(HWINT_INVALID_OP);
        LCRITICAL("Invalid op code [Arithmetic/Logic]");
    }
}

static void decodeSpecialArithmetic(Cpu& cpu, uint32_t instruction, uint32_t& programCounter) {
	uint32_t opCode = GETBITS(instruction, 3, 7);
	bool immediateFlag = GETBITS(instruction, 7, 8);
	uint32_t reg = GETBITS(instruction, 8, 13);
	uint32_t immReg;

	if (immediateFlag) {
		immReg = cpu.mem.getWord(programCounter);
		programCounter += 4;
	}
	else {
		immReg = GETBITS(instruction, 13, 18);
	}

	specialArithmeticFunctions[opCode](cpu, reg, immReg, immediateFlag);
}

static void decodeFloatingPointArithmetic(Cpu& cpu, uint32_t instruction, uint32_t& programCounter) {
	uint32_t opCode = GETBITS(instruction, 3, 5);

	uint8_t destReg = GETBITS(instruction, 5, 10);
	uint8_t src1 = GETBITS(instruction, 10, 15);
	uint8_t src2 = GETBITS(instruction, 15, 20);

	floatingPointArithmeticFunctions[opCode](cpu, destReg, src1, src2);
}

static void decodeFloatingPointSpecialArithmetic(Cpu& cpu, uint32_t instruction, uint32_t& programCounter) {
	uint32_t opCode = GETBITS(instruction, 3, 6);
	uint32_t immediateFlag = GETBITS(instruction, 6, 7);
	uint8_t destReg = GETBITS(instruction, 7, 12);
	
	uint32_t regimm;
	
	if (immediateFlag) {
		regimm = cpu.mem.getWord(programCounter);
		programCounter += 4;
	}
	else {
		regimm = GETBITS(instruction, 12, 17);
	}

	floatingPointSpecialArithmetic[opCode](cpu, destReg, regimm, immediateFlag? 1 : 0);
}

static void decodeMemInstruction(Cpu& cpu, uint32_t instruction, uint32_t& programCounter) {
	uint32_t opCode = GETBITS(instruction, 3, 6);
	uint32_t addr = GETBITS(instruction, 6, 11);
	uint8_t reg = GETBITS(instruction, 11, 16);
    bool offsetFlag = GETBITS(instruction, 16, 17);
    bool countFlag = GETBITS(instruction, 17, 18);
	uint32_t offset = 0, count = 1;

    if(offsetFlag) {
        offset = cpu.mem.getWord(programCounter);
        programCounter += 4;
    }

    if(countFlag) {
        count = cpu.mem.getWord(programCounter);
        programCounter += 4;
    }

	if (opCode < MEMORY_COUNT) {
		memoryInstructions[opCode](cpu, reg, addr, offset, count);
    }
	else {
		//cpu.computer->HWINT(HWINT_INVALID_OP);
        LCRITICAL("Invalid op code: [Memory instructions]");
    }
}

static void decodeJmpInstruction(Cpu& cpu, uint32_t instruction, uint32_t& programCounter) {
	uint32_t opCode = GETBITS(instruction, 3, 7);
	uint32_t immediateFlag = GETBITS(instruction, 7, 8);
	uint32_t immreg;

	if (immediateFlag) {
		immreg = cpu.mem.getWord(programCounter);
		programCounter += 4;
	}
	else {
		immreg = GETBITS(instruction, 8, 13);
	}

	if (opCode < JUMP_COUNT) {
		jumpInstructions[opCode](cpu, immreg, immediateFlag);
    }
	else {
		//cpu.computer->HWINT(HWINT_INVALID_OP);
        LCRITICAL("Invalid op code: [Jump instructions]");
    }
}

static void decodeInterruptInstruction(Cpu& cpu, uint32_t instruction, uint32_t& programCounter) {
	//cpu.computer->INT();
    cpu.stopSet = true;
    LINFO("INTERRUPT");
}

void (*instructionFunctions[])(
	Cpu& computer, 
	uint32_t instruction, 
	uint32_t& programCounter) = {
		decodeArithmeticLogic, 
		decodeSpecialArithmetic, 
		decodeFloatingPointArithmetic, 
		decodeMemInstruction, 
		decodeJmpInstruction,
		decodeFloatingPointSpecialArithmetic,
		decodeInterruptInstruction,
		decodeStackInstruction
};

Cpu::Cpu(uint32_t memorySize)
    :mem(memorySize)
{
    initBitMasks();

    for(int i = 0; i < REGISTER_COUNT; i++) {
        registers[i].setRegisterName(registerNames[i]);
    }

    for(int i = 0; i < FREGISTER_COUNT; i++) {
        fRegisters[i].setRegisterName(fregisterNames[i]);
    }
}

void Cpu::loadBinary(uint32_t* binary, int size, uint32_t location) {
    for(int i = 0; i < size; i++) {
        mem.setWord(binary[i], location + (i * 4));
    }
}

void Cpu::loadBinaryFile(const char* fileName, uint32_t location) {
    //FILES SHOULD BE ENCODED IN little ENDIAN, THE ASSEMBLER IS EXPECTED TO OUTPUT little ENDIAN FILES NO MATTER WHAT
    std::fstream input(fileName, std::ios::in | std::ios::binary | std::ios::ate);
    size_t fileSize = input.tellg();
    input.seekg(0, std::ios::beg);

    input.read((char*)mem.getMemPointer(), fileSize);
}

void Cpu::executeProgram() {

}

void Cpu::executeInstruction() {
    uint32_t instruction = GETWORD(registers[PC].value);
	uint32_t operation = GETBITS(instruction, 0, 3);

    registers[PC].value += 4;

    instructionFunctions[operation](*this, instruction, registers[PC].value);
}

Cpu::~Cpu() {
    for (int i = 0; i < 33; i++) {
		delete[] bitMasks[i];
	}

	delete[] bitMasks;
}