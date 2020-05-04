/************************************************************
 * Check license.txt in project root for license information *
 *********************************************************** */

#ifndef CPU2AO3_H
#define CPU2AO3_H

#include "defs.h"
#include "bus.h"

// Basicly 6502 cpu implementation
// cpu is connected to bus with address lines and data lines

// https://en.wikipedia.org/wiki/MOS_Technology_6502

// Different intructions take differnet amount of clock cycles
// 56 legal instuctions

#define STACK_START             0x0100
#define STACK_SIZE              0xFF
#define PROGRAM_START_POINTER   0xFFFC

// https://www.pagetable.com/?p=410
#define NMI_PC_LOCATION         0xFFFA
#define RESET_PC_LOCATION       0xFFFC
#define IRQ_OR_BRK_PC_LOCATION  0xFFFE

typedef enum CpuStatus {
    Carry           = (1 << 0),
    Zero            = (1 << 1),
    DisableIterups  = (1 << 2),
    DecimalMode     = (1 << 3),
    Break           = (1 << 4),
    Unused          = (1 << 5),
    Overflow        = (1 << 6),
    Negative        = (1 << 7),
} CpuStatus ;

// http://archive.6502.org/datasheets/rockwell_r650x_r651x.pdf
// http://www.obelisk.me.uk/6502/addressing.html
typedef enum AddressMode {
    IMP = 0,
    ACCUM,
    IMM,
    ZP,
    ZPX,
    ZPY,
    REL,
    ABS,
    ABSX,
    ABSY,
    IND,
    INDX,
    INDY,
} AddressMode ;

// https://www.masswerk.at/6502/6502_instruction_set.html
typedef enum Instructions {

    ADC = 0, //add with carry
    AND, //and (with accumulator)
    ASL, //arithmetic shift left
    BCC, //branch on carry clear
    BCS, //branch on carry set
    BEQ, //branch on equal (zero set)
    BIT, //bit test
    BMI, //branch on minus (negative set)
    BNE, //branch on not equal (zero clear)
    BPL, //branch on plus (negative clear)
    BRK, //break / interrupt
    BVC, //branch on overflow clear
    BVS, //branch on overflow set
    CLC, //clear carry
    CLD, //clear decimal
    CLI, //clear interrupt disable
    CLV, //clear overflow
    CMP, //compare (with accumulator)
    CPX, //compare with X
    CPY, //compare with Y
    DEC, //decrement
    DEX, //decrement X
    DEY, //decrement Y
    EOR, //exclusive or (with accumulator)
    INC, //increment
    INX, //increment X
    INY, //increment Y
    JMP, //jump
    JSR, //jump subroutine
    LDA, //load accumulator
    LDX, //load X
    LDY, //load Y
    LSR, //logical shift right
    NOP, //no operation
    ORA, //or with accumulator
    PHA, //push accumulator
    PHP, //push processor status (SR)
    PLA, //pull accumulator
    PLP, //pull processor status (SR)
    ROL, //rotate left
    ROR, //rotate right
    RTI, //return from interrupt
    RTS, //return from subroutine
    SBC, //subtract with carry
    SEC, //set carry
    SED, //set decimal
    SEI, //set interrupt disable
    STA, //store accumulator
    STX, //store X
    STY, //store Y
    TAX, //transfer accumulator to X
    TAY, //transfer accumulator to Y
    TSX, //transfer stack pointer to X
    TXA, //transfer X to accumulator
    TXS, //transfer X to stack pointer
    TYA, //transfer Y to accumulator
    XXX // Unknown
} Instructions ;

// might need extra cycle according to
// http://archive.6502.org/datasheets/rockwell_r65c00_microprocessors.pdf
const u64 extraCycleFlags =
((u64)1 << ADC) |
((u64)1 << AND) |
((u64)1 << EOR) |
((u64)1 << LDA) |
((u64)1 << LDX) |
((u64)1 << LDY) |
((u64)1 << ORA) |
((u64)1 << SBC);
// add CMP?

static inline u8
check_extra_cycle(Instructions opcode) {
    return extraCycleFlags & ((u64)1 << opcode);
}

typedef struct cpu2ao3 {
    // reqisters
    u8 Xreq;
    u8 Yreq;
    u8 accumReq;

    // Processor status
    u8 flags;
    // Program counter
    u16 pc;

    u8 stackPointer;
    u8 cycles;
} cpu2ao3;

// global cpu variable
cpu2ao3 cpu = {
    .Xreq = 0, .Yreq = 0, .accumReq = 0, .flags = Unused, .pc = 0x0, // pc is read from program start ptr
    .stackPointer = STACK_SIZE, .cycles = 0
};

static inline void
cpu_set_flag(CpuStatus flag,u8 cond) {
    if(cond) {
        cpu.flags |= flag;
    } else {
        cpu.flags &= ~flag;
    }
}

static inline u8
cpu_get_flag(CpuStatus flag) {
    return   (cpu.flags |= flag) > 0;
}

// Instruction, addressmode, cycles TODO clean unknown ones
#define INSTRUCTION_TABLE(FN) \
    FN(BRK , IMM, 7) FN(ORA, INDX, 6) FN(XXX, IMP, 2) FN(XXX, IMP, 8) FN(NOP, IMP, 3) FN(ORA, ZPX, 3) FN(ASL, ZPX, 5) FN(XXX, IMP, 5) FN(PHP, IMP, 3) FN(ORA, IMM, 2) FN(ASL, IMP, 2) FN(XXX, IMP, 2) FN(NOP, IMP, 4) FN(ORA, ABS, 4) FN(ASL, ABS, 6) FN(XXX, IMP, 6) \
    \
    FN(BPL, REL, 2) FN(ORA, INDY, 5) FN(XXX, IMP, 2) FN(XXX, IMP, 8) FN(NOP, IMP, 4) FN(ORA, ZPX, 4) FN(ASL, ZPX, 6) FN(XXX, IMP, 6) FN(CLC, IMP, 2) FN(ORA, ABSY, 4) FN(NOP, IMP, 2) FN(XXX, IMP, 7) FN(NOP, IMP, 4) FN(ORA, ABSX, 4) FN(ASL, ABSX, 7) FN(XXX, IMP, 7) \
    \
    FN(JSR, ABS, 6 ) FN(AND, INDX, 6 ) FN(XXX, IMP, 2 ) FN(XXX, IMP, 8 ) FN(BIT, ZP, 3 ) FN(AND, ZP, 3 ) FN(ROL, ZP, 5 ) FN(XXX, IMP, 5 ) FN(PLP, IMP, 4 ) FN(AND, IMM, 2 ) FN(ROL, ACCUM, 2 ) FN(XXX, IMP, 2 ) FN(BIT, ABS, 4 ) FN(AND, ABS, 4 ) FN(ROL, ABS, 6 ) FN(XXX, IMP, 6 ) \
    \
    FN(BMI, REL, 2 ) FN(AND, INDY, 5 ) FN(XXX, IMP, 2 ) FN(XXX, IMP, 8 ) FN(NOP, IMP, 4 ) FN(AND, ZPX, 4 ) FN(ROL, ZPX, 6 ) FN(XXX, IMP, 6 ) FN(SEC, IMP, 2 ) FN(AND, ABSY, 4 ) FN(NOP, IMP, 2 ) FN(XXX, IMP, 7 ) FN(NOP, IMP, 4 ) FN(AND, ABSX, 4 ) FN(ROL, ABSX, 7 ) FN(XXX, IMP, 7 ) \
    \
    FN(RTI, IMP, 6 ) FN(EOR, INDX, 6 ) FN(XXX, IMP, 2 ) FN(XXX, IMP, 8 ) FN(NOP, IMP, 3 ) FN(EOR, ZP, 3 ) FN(LSR, ZP, 5 ) FN(XXX, IMP, 5 ) FN(PHA, IMP, 3 ) FN(EOR, IMM, 2 ) FN(LSR, ACCUM, 2 ) FN(XXX, IMP, 2 ) FN(JMP, ABS, 3 ) FN(EOR, ABS, 4 ) FN(LSR, ABS, 6 ) FN(XXX, IMP, 6 ) \
    \
    FN(BVC, REL, 2 ) FN(EOR, INDY, 5 ) FN(XXX, IMP, 2 ) FN(XXX, IMP, 8 ) FN(NOP, IMP, 4 ) FN(EOR, ZPX, 4 ) FN(LSR, ZPX, 6 ) FN(XXX, IMP, 6 ) FN(CLI, IMP, 2 ) FN(EOR, ABSY, 4 ) FN(NOP, IMP, 2 ) FN(XXX, IMP, 7 ) FN(NOP, IMP, 4 ) FN(EOR, ABSX, 4 ) FN(LSR, ABSX, 7 ) FN(XXX, IMP, 7 ) \
    \
    FN(RTS, IMP, 6 ) FN(ADC, INDX, 6 ) FN(XXX, IMP, 2 ) FN(XXX, IMP, 8 ) FN(NOP, IMP, 3 ) FN(ADC, ZP, 3 ) FN(ROR, ZP, 5 ) FN(XXX, IMP, 5 ) FN(PLA, IMP, 4 ) FN(ADC, IMM, 2 ) FN(ROR, IMP, 2 ) FN(XXX, IMP, 2 ) FN(JMP, IND, 5 ) FN(ADC, ABS, 4 ) FN(ROR, ABS, 6 ) FN(XXX, IMP, 6 ) \
    \
    FN(BVS, REL, 2 ) FN(ADC, INDY, 5 ) FN(XXX, IMP, 2 ) FN(XXX, IMP, 8 ) FN(NOP, IMP, 4 ) FN(ADC, ZPX, 4 ) FN(ROR, ZPX, 6 ) FN(XXX, IMP, 6 ) FN(SEI, IMP, 2 ) FN(ADC, ABSY, 4 ) FN(NOP, IMP, 2 ) FN(XXX, IMP, 7 ) FN(NOP, IMP, 4 ) FN(ADC, ABSX, 4 ) FN(ROR, ABSX, 7 ) FN(XXX, IMP, 7 ) \
    \
    FN(NOP, IMP, 2 ) FN(STA, INDX, 6 ) FN(NOP, IMP, 2 ) FN(XXX, IMP, 6 ) FN(STY, ZP, 3 ) FN(STA, ZP, 3 ) FN(STX, ZP, 3 ) FN(XXX, IMP, 3 ) FN(DEY, IMP, 2 ) FN(NOP, IMP, 2 ) FN(TXA, IMP, 2 ) FN(XXX, IMP, 2 ) FN(STY, ABS, 4 ) FN(STA, ABS, 4 ) FN(STX, ABS, 4 ) FN(XXX, IMP, 4 ) \
    \
    FN(BCC, REL, 2 ) FN(STA, INDY, 6 ) FN(XXX, IMP, 2 ) FN(XXX, IMP, 6 ) FN(STY, ZPX, 4 ) FN(STA, ZPX, 4 ) FN(STX, ZPY, 4 ) FN(XXX, IMP, 4 ) FN(TYA, IMP, 2 ) FN(STA, ABSY, 5 ) FN(TXS, IMP, 2 ) FN(XXX, IMP, 5 ) FN(NOP, IMP, 5 ) FN(STA, ABSX, 5 ) FN(XXX, IMP, 5 ) FN(XXX, IMP, 5 ) \
    \
    FN(LDY, IMM, 2 ) FN(LDA, INDX, 6 ) FN(LDX, IMM, 2 ) FN(XXX, IMP, 6 ) FN(LDY, ZP, 3 ) FN(LDA, ZP, 3 ) FN(LDX, ZP, 3 ) FN(XXX, IMP, 3 ) FN(TAY, IMP, 2 ) FN(LDA, IMM, 2 ) FN(TAX, IMP, 2 ) FN(XXX, IMP, 2 ) FN(LDY, ABS, 4 ) FN(LDA, ABS, 4 ) FN(LDX, ABS, 4 ) FN(XXX, IMP, 4 ) \
    \
    FN(BCS, REL, 2 ) FN(LDA, INDY, 5 ) FN(XXX, IMP, 2 ) FN(XXX, IMP, 5 ) FN(LDY, ZPX, 4 ) FN(LDA, ZPX, 4 ) FN(LDX, ZPY, 4 ) FN(XXX, IMP, 4 ) FN(CLV, IMP, 2 ) FN(LDA, ABSY, 4 ) FN(TSX, IMP, 2 ) FN(XXX, IMP, 4 ) FN(LDY, ABSX, 4 ) FN(LDA, ABSX, 4 ) FN(LDX, ABSY, 4 ) FN(XXX, IMP, 4 ) \
    \
    FN(CPY, IMM, 2 ) FN(CMP, INDX, 6 ) FN(NOP, IMP, 2 ) FN(XXX, IMP, 8 ) FN(CPY, ZP, 3 ) FN(CMP, ZP, 3 ) FN(DEC, ZP, 5 ) FN(XXX, IMP, 5 ) FN(INY, IMP, 2 ) FN(CMP, IMM, 2 ) FN(DEX, IMP, 2 ) FN(XXX, IMP, 2 ) FN(CPY, ABS, 4 ) FN(CMP, ABS, 4 ) FN(DEC, ABS, 6 ) FN(XXX, IMP, 6 ) \
    \
    FN(BNE, REL, 2 ) FN(CMP, INDY, 5 ) FN(XXX, IMP, 2 ) FN(XXX, IMP, 8 ) FN(NOP, IMP, 4 ) FN(CMP, ZPX, 4 ) FN(DEC, ZPX, 6 ) FN(XXX, IMP, 6 ) FN(CLD, IMP, 2 ) FN(CMP, ABSY, 4 ) FN(NOP, IMP, 2 ) FN(XXX, IMP, 7 ) FN(NOP, IMP, 4 ) FN(CMP, ABSX, 4 ) FN(DEC, ABSX, 7 ) FN(XXX, IMP, 7 ) \
    \
    FN(CPX, IMM, 2 ) FN(SBC, INDX, 6 ) FN(NOP, IMP, 2 ) FN(XXX, IMP, 8 ) FN(CPX, ZP, 3 ) FN(SBC, ZP, 3 ) FN(INC, ZP, 5 ) FN(XXX, IMP, 5 ) FN(INX, IMP, 2 ) FN(SBC, IMM, 2 ) FN(NOP, IMP, 2 ) FN(SBC, IMP, 2 ) FN(CPX, ABS, 4 ) FN(SBC, ABS, 4 ) FN(INC, ABS, 6 ) FN(XXX, IMP, 6 ) \
    \
    FN(BEQ, REL, 2 ) FN(SBC, INDY, 5 ) FN(XXX, IMP, 2 ) FN(XXX, IMP, 8 ) FN(NOP, IMP, 4 ) FN(SBC, ZPX, 4 ) FN(INC, ZPX, 6 ) FN(XXX, IMP, 6 ) FN(SED, IMP, 2 ) FN(SBC, ABSY, 4 ) FN(NOP, IMP, 2 ) FN(XXX, IMP, 7 ) FN(NOP, IMP, 4 ) FN(SBC, ABSX, 4 ) FN(INC, ABSX, 7 ) FN(XXX, IMP, 7 ) \

typedef struct Instruction {
    u8 instructionCode;
    u8 addressMode;
    u8 cycles;
} Instruction;


#define CREATE_INSTRUCTION_TABLE(IN, ADDR, CYCLE) { .instructionCode = IN, .addressMode = ADDR, .cycles = CYCLE },

Instruction instructionTable[] = {
    INSTRUCTION_TABLE(CREATE_INSTRUCTION_TABLE)
};



#define FETCH do{                                                       \
    if(instruct.addressMode != IMP && instruct.addressMode != ACCUM)    \
    fetched = bus_read8(addr);                                          \
} while(0)                                                              \

// https://wiki.nesdev.com/w/index.php/Stack
// 6502 had a descending stack, with "empty stack" pointer (points to empty place)
static inline void
stack_push (u8 val) {

    if(cpu.stackPointer == 0) ABORT("stack overflow\n");

    bus_write8(STACK_START + cpu.stackPointer, val);
    cpu.stackPointer -= 1;
}

static inline u8
stack_pop () {

    if(cpu.stackPointer == STACK_SIZE) ABORT("stack underflow\n");

    cpu.stackPointer += 1;
    return bus_read8(STACK_START + cpu.stackPointer);
}

static void
cpu_clock() {

    // execute the intruction
    if(cpu.cycles == 0) {
        u8 opcode = bus_read8(cpu.pc);
        cpu.pc += 1;

        Instruction instruct = instructionTable[opcode];
        cpu.cycles = instruct.cycles;

        u16 addr = 0;
        u8 fetched = 0;
        // fetch required data
        switch(instruct.addressMode) {
            // http://www.emulator101.com/6502-addressing-modes.html
            // first two are bit cryptic bit ACCUM might be accumulator address
            // and https://github.com/OneLoneCoder/olcNES/blob/master/Part%232%20-%20CPU/olc6502.cpp
            // used IMP as ACCUM equilevant
            case IMP:
                {
                    fetched = cpu.accumReq;
                } break;
            case ACCUM:
                {
                    fetched = cpu.accumReq;
                    break;
                } break;
            case IMM:
                {
                    addr = cpu.pc;
                    cpu.pc++;
                } break;
            case ZP: // zero page addressing
                {
                    addr = bus_read8(cpu.pc);
                    cpu.pc += 1;
                } break;
            case ZPX: // zero page addressing with x
                {
                    addr = (bus_read8(cpu.pc) + cpu.Xreq) % 256;
                    cpu.pc += 1;
                } break;
            case ZPY: // zero page addressing with y
                {
                    addr = (bus_read8(cpu.pc) + cpu.Yreq) % 256;
                    cpu.pc += 1;
                } break;
            case REL: // Branch instructions (e.g. BEQ, BCS) have a relative addressing mode
                // that specifies an 8-bit signed offset relative to the current PC.
                {
                    addr += (i8)bus_read8(cpu.pc); //TODO might be incorrect
                }
                break;
            case ABS:
                {
                    addr = bus_read16(cpu.pc);
                    cpu.pc += 2;
                } break;
            case ABSX:
                {
                    addr = (bus_read16(cpu.pc));
                    cpu.pc += 2;

                    // implement the oops cycle on page change
                    // https://wiki.nesdev.com/w/index.php/CPU_addressing_modes
                    u16 temp = addr + cpu.Xreq;
                    if( check_extra_cycle(instruct.instructionCode ) &&
                            ((addr & 0xFF00) != (temp & 0xFF00))) {
                        cpu.cycles += 1;
                    }
                    addr = temp;
                } break;
            case ABSY:
                {
                    addr = (bus_read16(cpu.pc));
                    cpu.pc += 2;

                    // implement the oops cycle on page change
                    // https://wiki.nesdev.com/w/index.php/CPU_addressing_modes
                    u16 temp = addr + cpu.Yreq;
                    if( check_extra_cycle(instruct.instructionCode ) &&
                            ((addr & 0xFF00) != (temp & 0xFF00)) ) {
                        cpu.cycles += 1;
                    }
                    addr = temp;
                } break;
            case IND:  //  The JMP instruction has a special indirect addressing mode that can
                // jump to the address stored in a 16-bit pointer anywhere in memory.
                // this contains bug http://forum.6502.org/viewtopic.php?t=770
                {
                    u16 low = (u16)bus_read8(cpu.pc);
                    u16 high = (u16)bus_read8(cpu.pc + 1);

                    if (low == 0x00FF) { // Simulate page boundary hardware bug
                        high = bus_read8(cpu.pc & 0xFF00); // read pages start to higher byte
                    }

                    addr = bus_read16( (high << 8) | low );
                    cpu.pc += 2;
                } break;
            case INDX: // indirect zero page addressing with x
                {
                    u16 ptr = (u16)bus_read8(cpu.pc);

                    u16 low = bus_read8((ptr + cpu.Xreq) % 256) << 8;
                    u16 high = bus_read8((ptr + cpu.Xreq + 1) % 256);

                    addr = low | high;

                    cpu.pc += 1;
                } break;
            case INDY: // indirect zero page addressing with y
                {
                    u16 ptr = (u16)bus_read8(cpu.pc);

                    u16 low = bus_read8(ptr % 256) << 8;
                    u16 high = bus_read8((ptr + 1) % 256);

                    u16 temp = (low | (high << 8)) + cpu.Yreq;
                    addr = (low | (high << 8)) + cpu.Yreq;

                    // implement the oops cycle on page change
                    // https://wiki.nesdev.com/w/index.php/CPU_addressing_modes
                    if( check_extra_cycle(instruct.instructionCode ) &&
                            ((addr & 0xFF00) != (temp & 0xFF00)) ) {
                        cpu.cycles += 1;
                    }
                    cpu.pc += 1;
                } break;
            default:
                ABORT("Error addressing mode");
        }


        // perform the instruction (this is hopefully optimized to jumptable)
        switch(instruct.instructionCode) {

            case ADC: //add with carry A + M + C -> A, C
                {
                    FETCH;
                    u16 temp = (u16)cpu.accumReq + (u16)fetched + (u16)cpu_get_flag(Carry);
                    cpu_set_flag(Carry, temp > 0xFF);
                    cpu_set_flag(Zero, temp == 0x0);
                    cpu_set_flag(Negative, (temp & 0x80) == 0x80);

                    // check overflow
                    // (2 positives result negative) and (2 negatives result positive)
                    // so if two high bits are same on operants and different on result set it
                    cpu_set_flag(Overflow, (cpu.accumReq ^ (u8)(temp & 0x00FF)) &
                            (fetched ^ (u8)(temp & 0x00FF)) & 0x80);
#if 0
                    cpu_set_flag(Overflow,
                            (~((uint16_t)cpu.accumReq ^ (uint16_t)fetched) &
                             ((uint16_t)cpu.accumReq ^ (uint16_t)temp)) & 0x0080);
#endif

                    cpu.accumReq = temp & 0x00FF;
                } break;
            case AND: //and (with accumulator), A AND M -> A
                {
                    FETCH;
                    cpu.accumReq &= fetched;

                    cpu_set_flag(Negative, cpu.accumReq & 0x80);
                    cpu_set_flag(Zero, cpu.accumReq == 0);
                } break;
            case ASL: //arithmetic shift left, C <- [76543210] <- 0
                {
                    FETCH;
                    u16 temp = ((u16)fetched) << 1;

                    cpu_set_flag(Negative, temp & 0x80);
                    cpu_set_flag(Zero, temp == 0);
                    cpu_set_flag(Carry, (temp & 0x0100) > 0x0);

                    if(instruct.addressMode == IMP && instruct.addressMode == ACCUM) {
                        cpu.accumReq = (u8)temp;
                    } else {
                        bus_write8(addr, (u8)temp);
                    }
                } break;
            case BCC: //branch on carry clear
                {
                    // http://archive.6502.org/datasheets/rockwell_r65c00_microprocessors.pdf
                    // 1 cycle if same page 2 if different
                    if(cpu_get_flag(Carry) == 0) {
                        cpu.cycles += 1;

                        if((cpu.pc ) && (addr & 0xFF00)) {
                            cpu.cycles += 1;
                        }
                        cpu.pc = addr;
                    }
                } break;
            case BCS: //branch on carry set
                {
                    // http://archive.6502.org/datasheets/rockwell_r65c00_microprocessors.pdf
                    // 1 cycle if same page 2 if different
                    if(cpu_get_flag(Carry) == 1) {
                        cpu.cycles += 1;

                        if((cpu.pc ) && (addr & 0xFF00)) {
                            cpu.cycles += 1;
                        }
                        cpu.pc = addr;
                    }
                } break;
            case BEQ: //branch on equal (zero set)
                {
                    // http://archive.6502.org/datasheets/rockwell_r65c00_microprocessors.pdf
                    // 1 cycle if same page 2 if different
                    if(cpu_get_flag(Zero) == 1) {
                        cpu.cycles += 1;

                        if(cpu.pc && (addr & 0xFF00)) {
                            cpu.cycles += 1;
                        }
                        cpu.pc = addr;
                    }
                } break;
            case BIT: // bit test, A AND M, M7 -> N, M6 -> V (V=overflow)
                // bits 7 and 6 of operand are transfered to bit 7 and 6 of SR (N,V);
                // the zeroflag is set to the result of operand AND accumulator.
                {
                    FETCH;
                    u8 temp = cpu.accumReq & fetched;
                    cpu_set_flag(Negative, (temp & (1 << 7)) > 0);
                    cpu_set_flag(Overflow, (temp & (1 << 6)) > 0);
                    cpu_set_flag(Zero, temp == 0x0);
                } break;
            case BMI: //branch on minus (negative set)
                {
                    // http://archive.6502.org/datasheets/rockwell_r65c00_microprocessors.pdf
                    // 1 cycle if same page 2 if different
                    if(cpu_get_flag(Negative) == 1) {
                        cpu.cycles += 1;

                        if((cpu.pc) && (addr & 0xFF00)) {
                            cpu.cycles += 1;
                        }
                        cpu.pc = addr;
                    }
                } break;
            case BNE: //branch on not equal (zero clear)
                {
                    // http://archive.6502.org/datasheets/rockwell_r65c00_microprocessors.pdf
                    // 1 cycle if same page 2 if different
                    if(cpu_get_flag(Zero) == 0) {
                        cpu.cycles += 1;

                        if((cpu.pc ) && (addr & 0xFF00)) {
                            cpu.cycles += 1;
                        }
                        cpu.pc = addr;
                    }
                } break;
            case BPL: //branch on plus (negative clear)
                {
                    // http://archive.6502.org/datasheets/rockwell_r65c00_microprocessors.pdf
                    // 1 cycle if same page 2 if different
                    if(cpu_get_flag(Negative) == 0) {
                        cpu.cycles += 1;

                        if((cpu.pc ) && (addr & 0xFF00)) {
                            cpu.cycles += 1;
                        }
                        cpu.pc = addr;
                    }
                } break;
            case BRK: //break / interrupt
                {
                    printf("TODO\n");
                } break;
            case BVC: //branch on overflow clear
                {
                    printf("TODO\n");
                } break;
            case BVS: //branch on overflow set
                {
                    printf("TODO\n");
                } break;
            case CLC: //clear carry
                {
                    cpu_set_flag(Carry, 0);
                } break;
            case CLD: //clear decimal
                {
                    cpu_set_flag(DecimalMode, 0);
                    ABORT("Decimal clearing should not happen");
                } break;
            case CLI: //clear interrupt disable
                {
                    printf("TODO\n");
                } break;
            case CLV: //clear overflow
                {
                    cpu_set_flag(Overflow, 0);
                } break;
            case CMP: //compare (with accumulator)
                {
                    printf("TODO\n");
                } break;
            case CPX: //compare with X
                {
                    printf("TODO\n");
                } break;
            case CPY: //compare with Y
                {
                    printf("TODO\n");
                } break;
            case DEC: //decrement
                {
                    printf("TODO\n");
                } break;
            case DEX: //decrement X
                {
                    printf("TODO\n");
                } break;
            case DEY: //decrement Y
                {
                    printf("TODO\n");
                } break;
            case EOR: //exclusive or (with accumulator)
                {
                    printf("TODO\n");
                } break;
            case INC: //increment
                {
                    printf("TODO\n");
                } break;
            case INX: //increment X
                {
                    printf("TODO\n");
                } break;
            case INY: //increment Y
                {
                    printf("TODO\n");
                } break;
            case JMP: //jump
                {
                    printf("TODO\n");
                } break;
            case JSR: //jump subroutine
                {
                    printf("TODO\n");
                } break;
            case LDA: //load accumulator
                {
                    printf("TODO\n");
                } break;
            case LDX: //load X
                {
                    printf("TODO\n");
                } break;
            case LDY: //load Y
                {
                    printf("TODO\n");
                } break;
            case LSR: //logical shift right
                {
                    printf("TODO\n");
                } break;
            case NOP: //no operation
                {
                    printf("TODO\n");
                } break;
            case ORA: //or with accumulator
                {
                    printf("TODO\n");
                } break;
            case PHA: //push accumulator
                {
                    stack_push(cpu.accumReq);
                } break;
            case PHP: //push processor status (SR)
                {
                    printf("TODO\n");
                } break;
            case PLA: //pull accumulator
                {
                    cpu.accumReq = stack_pop();
                    cpu_set_flag(Negative, cpu.accumReq & 0x80);
                    cpu_set_flag(Zero, cpu.accumReq == 0x0);
                } break;
            case PLP: //pull processor status (SR)
                {
                    printf("TODO\n");
                } break;
            case ROL: //rotate left
                {
                    printf("TODO\n");
                } break;
            case ROR: //rotate right
                {
                    printf("TODO\n");
                } break;
            case RTI: //return from interrupt
                {
                    printf("TODO\n");
                } break;
            case RTS: //return from subroutine
                {
                    printf("TODO\n");
                } break;
            case SBC: //subtract with carry, A - M - (1 - C) -> A (1 - C is borrow bit)
                {
                    // same as ADC but with inverted M
                    FETCH;
                    fetched ^= 0xFF;

                    u16 temp = (u16)cpu.accumReq + (u16)fetched + (u16)cpu_get_flag(Carry);
                    cpu_set_flag(Carry, temp > 0xFF);
                    cpu_set_flag(Zero, temp == 0x0);
                    cpu_set_flag(Negative, (temp & 0x80) == 0x80);

                    // check overflow
                    // (2 positives result negative) and (2 negatives result positive)
                    // so if two high bits are same on operants and different on result set it
                    cpu_set_flag(Overflow, (cpu.accumReq ^ (u8)(temp & 0x00FF)) &
                            (fetched ^ (u8)(temp & 0x00FF)) & 0x80);

                    cpu.accumReq = temp & 0x00FF;
                } break;
            case SEC: //set carry
                {
                    printf("TODO\n");
                } break;
            case SED: //set decimal
                {
                    printf("TODO\n");
                } break;
            case SEI: //set interrupt disable
                {
                    printf("TODO\n");
                } break;
            case STA: //store accumulator
                {
                    printf("TODO\n");
                } break;
            case STX: //store X
                {
                    printf("TODO\n");
                } break;
            case STY: //store Y
                {
                    printf("TODO\n");
                } break;
            case TAX: //transfer accumulator to X
                {
                    printf("TODO\n");
                } break;
            case TAY: //transfer accumulator to Y
                {
                    printf("TODO\n");
                } break;
            case TSX: //transfer stack pointer to X
                {
                    printf("TODO\n");
                } break;
            case TXA: //transfer X to accumulator
                {
                    printf("TODO\n");
                } break;
            case TXS: //transfer X to stack pointer
                {
                    printf("TODO\n");
                } break;
            case TYA: //transfer Y to accumulator
                {
                    printf("TODO\n");
                } break;
            case XXX: // Unknown
                {
                    ABORT("not legal instruction");
                } break;
            default:
                {
                    ABORT("Unknown instruction");
                }
        }
    }

    cpu.cycles -= 1;
}

static void
cpu_reset() {

    cpu = (cpu2ao3) {
        .Xreq = 0, .Yreq = 0, .accumReq = 0, .flags = Unused, .pc = 0x0,
            .stackPointer = STACK_SIZE, .cycles = 0
    };

    cpu.pc = bus_read16(PROGRAM_START_POINTER);
    cpu.cycles += 8;
}

static void
cpu_iterrupt_request() {

    if(cpu_get_flag(DisableIterups) == 0) {
        // write current pc to stack
        stack_push( (cpu.pc >> 8) & 0xFF );
        stack_push( cpu.pc & 0xFF );

        cpu_set_flag(DisableIterups, 1);
        cpu_set_flag(Break, 1); // TODO has to be set??
        cpu_set_flag(Unused, 1); // TODO has to be set??

        // TODO flags before or after?
        stack_push(cpu.flags);
    }
}

static void
cpu_no_mask_iterrupt() {

}


#endif /*CPU2AO3_H*/
