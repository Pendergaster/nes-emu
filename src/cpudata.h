/************************************************************
 * Check license.txt in project root for license information *
 *********************************************************** */

#ifndef CPUDATA_H
#define CPUDATA_H

#include "defs.h"

#define STACK_START             0x0100
#define STACK_SIZE              0xFF
#define PROGRAM_START_POINTER   0xFFFC

// https://www.pagetable.com/?p=410
#define NMI_PC_LOCATION         0xFFFA
#define RESET_PC_LOCATION       0xFFFC
#define IRQ_OR_BRK_PC_LOCATION  0xFFFE

typedef struct cpu2ao3 {
    //  reqisters
    u8  Xreq;
    u8  Yreq;
    u8  accumReq;

    //  Processor status
    u8  flags;
    //  Program counter
    u16 pc;

    u8  stackPointer;
    u32 cycles;

    u64 instructionCount;
} cpu2ao3;

// global cpu variable
cpu2ao3 cpu = {
    .Xreq = 0, .Yreq = 0, .accumReq = 0, .flags = 0, .pc = 0x0, // pc is read from program start ptr
    .stackPointer = STACK_SIZE, .cycles = 0
};

typedef enum CpuStatus {
    Carry           = (1 << 0),
    Zero            = (1 << 1),
    DisableIterups  = (1 << 2),
    DecimalMode     = (1 << 3),
    // only PHP and BRK sets this flag on push to the stack
    Break           = (1 << 4),
    // this is kinda always set on?
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

// Instruction, addressmode, cycles TODO clean unknown ones
#define INSTRUCTION_TABLE(FN) \
    FN(BRK , IMM, 7) FN(ORA, INDX, 6) FN(XXX, IMP, 2) FN(XXX, IMP, 8) FN(NOP, IMP, 3) FN(ORA, ZP, 3) FN(ASL, ZP, 5) FN(XXX, IMP, 5) FN(PHP, IMP, 3) FN(ORA, IMM, 2) FN(ASL, IMP, 2) FN(XXX, IMP, 2) FN(NOP, IMP, 4) FN(ORA, ABS, 4) FN(ASL, ABS, 6) FN(XXX, IMP, 6) \
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

#define CREATE_CPU_TABLE_STING(NAME, ADDR, XXX) \
#NAME " " #ADDR,

//##ADDR##,
char* cpuInstructionStrings[] = {
    INSTRUCTION_TABLE(CREATE_CPU_TABLE_STING)
};

#endif /* CPUDATA_H */
