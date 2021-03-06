/************************************************************
 * Check license.txt in project root for license information *
 *********************************************************** */

#ifndef CPU2AO3_H
#define CPU2AO3_H

#include "defs.h"
#include "cpudata.h"
#include "bus.h"
// http://www.6502.org/tutorials/6502opcodes.html#ROR opcode explanations

// Basicly 6502 cpu implementation
// cpu is connected to bus with address lines and data lines

// https://en.wikipedia.org/wiki/MOS_Technology_6502

// Different intructions take differnet amount of clock cycles
// 56 legal instuctions


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
    return extraCycleFlags & (/*(u64)1 << */opcode);
}

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
    return   (cpu.flags & flag) > 0;
}


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

    if(cpu.stackPointer == STACK_SIZE) ABORT("stack underflow");

    cpu.stackPointer += 1;
    return bus_read8(STACK_START + cpu.stackPointer);
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
cpu_iterrupt_request() { //irq

    if(cpu_get_flag(DisableIterups) == 0) {
        // write current pc to stack
        stack_push( (cpu.pc >> 8) & 0xFF );
        stack_push( cpu.pc & 0xFF );

        // https://www.pagetable.com/?p=410
        cpu_set_flag(DisableIterups, 1);

        //  op      Unused and Break    After push
        //  PHP     11                  None
        //  BRK     11                  Break is set to 1
        //  IRQ     10                  Break is set to 1
        //  NMI     10                  Break is set to 1

        cpu_set_flag(Break, 1); // TODO has to be set??

        // TODO flags before or after?
        stack_push(cpu.flags | Unused);

        // read new pc
        cpu.pc = bus_read16(IRQ_OR_BRK_PC_LOCATION);

        cpu.cycles += 7;
    }
}

static void
cpu_no_mask_iterrupt() { //nmi

    //  op      Unused and Break    After push
    //  PHP     11                  None
    //  BRK     11                  Break is set to 1
    //  IRQ     10                  Break is set to 1
    //  NMI     10                  Break is set to 1

    stack_push( (cpu.pc >> 8) & 0xFF );
    stack_push( cpu.pc & 0xFF );

    // https://www.pagetable.com/?p=410
    cpu_set_flag(DisableIterups, 1);

    cpu_set_flag(Break, 0); // TODO has to be set??

    // TODO flags before or after?
    stack_push(cpu.flags | Unused);

    cpu_set_flag(Break, 1); // TODO has to be set??

    // read new pc
    cpu.pc = bus_read16(NMI_PC_LOCATION);

    cpu.cycles = 8;
}

static void
cpu_return_from_interrupt() { // RTI
    cpu.flags = stack_pop();
    u16 low = stack_pop();
    u16 high = stack_pop();

    cpu_set_flag(Break, 0);
    cpu_set_flag(Unused, 1); //TODO has to be set??

    cpu.pc = (high << 8) | low;
}


int debug = 0;
i32 breakpoint = 0x10000;
u16 instructionCountBreakPoint = 0;
static u8
cpu_clock() {

    // execute the intruction

    u8 ret = cpu.cycles == 0;

    if(cpu.cycles == 0) {

        u8 opcode = bus_read8(cpu.pc);

        // TODO remove all logs
        CHECKLOG;

#ifdef LOGFILE
        LOG("opcode 0x%04X pc 0x%04X accum 0x%04X, Yreq 0x%04X Xreq 0x%04X opcount %ld \n%s",
                opcode, cpu.pc, cpu.accumReq, cpu.Yreq, cpu.Xreq, cpu.instructionCount,
                cpuInstructionStrings[opcode]);
#endif

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
                    i8 rel = (i8)bus_read8(cpu.pc);
                    cpu.pc += 1;
                    addr = rel + (cpu.pc);
                }
                break;
            case ABS:
                {
                    addr = bus_read16(cpu.pc);
                    cpu.pc += 2;
                } break;
            case ABSX:
                {
                    addr = bus_read16(cpu.pc);
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
                    addr = bus_read16(cpu.pc);
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
                    u16 tempAddr = (high << 8) | low;

                    if (low == 0x00FF) { // Simulate page boundary hardware bug
                        high = (tempAddr & 0xFF00);
                        low = tempAddr;
                    } else {
                        high = tempAddr + 1;
                        low = tempAddr;
                    }

                    addr = (bus_read8(high) << 8) | bus_read8(low);
                    cpu.pc += 2;
                } break;
            case INDX: // indirect zero page addressing with x
                {
                    u16 ptr = (u16)bus_read8(cpu.pc);

                    u16 low = bus_read8((ptr + cpu.Xreq) & 0xFF);
                    u16 high = bus_read8((ptr + cpu.Xreq + 1) & 0xFF);

                    addr = low | (high << 8);

                    cpu.pc += 1;

                } break;
            case INDY: // indirect zero page addressing with y
                {
                    u16 ptr = (u16)bus_read8(cpu.pc);

                    u16 low = bus_read8(ptr & 0xFF); // TODO jotain on vaarin
                    u16 high = bus_read8((ptr + 1) & 0xFF); // TODO jotain on vaarin

                    addr = (low | (high << 8)) + cpu.Yreq;

                    // implement the oops cycle on page change
                    // https://wiki.nesdev.com/w/index.php/CPU_addressing_modes
                    if( check_extra_cycle(instruct.instructionCode ) &&
                            ((addr & 0xFF00) != (high << 8)) ) {
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
                    //LOG("ADC accum 0x%04X, fetched 0x%04X, carry 0x%04X",
                    //cpu.accumReq, fetched, cpu_get_flag(Carry));
                    u16 temp = (u16)cpu.accumReq + (u16)fetched + (u16)cpu_get_flag(Carry);

                    cpu_set_flag(Carry, temp > 0xFF);
                    cpu_set_flag(Zero, (temp & 0x00FF) == 0x0);
                    cpu_set_flag(Negative, temp & 0x80);

                    // check overflow
                    // (2 positives result negative) and (2 negatives result positive)
                    // so if two high bits are same on operants and different on result set it
                    cpu_set_flag(Overflow, (cpu.accumReq ^ (u8)(temp & 0x00FF)) &
                            (fetched ^ (u8)(temp & 0x00FF)) & 0x80);

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
                    cpu_set_flag(Zero, (temp & 0xFF) == 0);
                    cpu_set_flag(Carry, (temp & 0x0100) > 0);

                    if(instruct.addressMode == IMP || instruct.addressMode == ACCUM) {
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

                        if((cpu.pc & 0xFF00) != (addr & 0xFF00)) { //TODO wtf
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

                        if((cpu.pc & 0xFF00) != (addr & 0xFF00)) { //TODO wtf
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

                        if((cpu.pc & 0xFF00) != (addr & 0xFF00)) { //TODO wtf
                            cpu.cycles += 1;
                        }
                        cpu.pc = addr;
                    }
                } break;
            case BIT: // bit test, A AND M, M7 -> N, M6 -> V (V = overflow)
                // bits 7 and 6 of operand are transfered to bit 7 and 6 of SR (N,V);
                // the zeroflag is set to the result of operand AND accumulator.
                {
                    FETCH;
                    u8 temp = cpu.accumReq & fetched;
                    cpu_set_flag(Negative, fetched & 0x80);
                    cpu_set_flag(Overflow, fetched & 0x40);
                    cpu_set_flag(Zero, temp == 0x0);
                } break;
            case BMI: //branch on minus (negative set)
                {
                    // http://archive.6502.org/datasheets/rockwell_r65c00_microprocessors.pdf
                    // 1 cycle if same page 2 if different
                    if(cpu_get_flag(Negative) == 1) {
                        cpu.cycles += 1;

                        if((cpu.pc & 0xFF00) != (addr & 0xFF00)) {
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

                        if((cpu.pc & 0xFF00) != (addr & 0xFF00)) {
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

                        if( (cpu.pc & 0xFF00) != (addr & 0xFF00) ) {
                            cpu.cycles += 1;
                        }
                        cpu.pc = addr;
                    }
                } break;
            case BRK: //break interrupt, push PC+2, push SR
                {
                    //  op      Unused and Break    After push
                    //  PHP     11                  None
                    //  BRK     11                  Break is set to 1
                    //  IRQ     10                  Break is set to 1
                    //  NMI     10                  Break is set to 1

                    stack_push( (cpu.pc >> 8) & 0xFF );
                    stack_push( cpu.pc & 0xFF );

                    cpu_set_flag(Break, 1);

                    stack_push(cpu.flags | Unused);

                    cpu.pc = bus_read16(IRQ_OR_BRK_PC_LOCATION);
                } break;
            case BVC: //branch on overflow clear
                {
                    // http://archive.6502.org/datasheets/rockwell_r65c00_microprocessors.pdf
                    // 1 cycle if same page 2 if different
                    if(cpu_get_flag(Overflow) == 0) {
                        cpu.cycles += 1;

                        if( (cpu.pc & 0xFF00) != (addr & 0xFF00) ) {
                            cpu.cycles += 1;
                        }
                        cpu.pc = addr;
                    }
                } break;
            case BVS: //branch on overflow set
                {
                    // http://archive.6502.org/datasheets/rockwell_r65c00_microprocessors.pdf
                    // 1 cycle if same page 2 if different
                    if(cpu_get_flag(Overflow) == 1) {
                        cpu.cycles += 1;

                        if( (cpu.pc & 0xFF00) != (addr & 0xFF00) ) {
                            cpu.cycles += 1;
                        }
                        cpu.pc = addr;
                    }
                } break;
            case CLC: //clear carry
                {
                    cpu_set_flag(Carry, 0);
                } break;
            case CLD: //clear decimal
                {
                    cpu_set_flag(DecimalMode, 0);
                    // might happen on some tests
                    //ABORT("Decimal clearing should not happen");
                } break;
            case CLI: //clear interrupt disable
                {
                    cpu_set_flag(DisableIterups, 0);
                } break;
            case CLV: //clear overflow
                {
                    cpu_set_flag(Overflow, 0);
                } break;
            case CMP: //compare (with accumulator) A - M
                {
                    FETCH;
                    u16 temp = (u16)cpu.accumReq - (u16)fetched;

                    cpu_set_flag(Carry, cpu.accumReq >= fetched);
                    cpu_set_flag(Negative, temp  & 0x0080);
                    cpu_set_flag(Zero, (temp & 0x00FF) == 0x0);
                } break;
            case CPX: //compare with X - M
                {
                    FETCH;
                    u8 temp = (u16)cpu.Xreq - (u16)fetched;
                    cpu_set_flag(Carry, cpu.Xreq >= fetched);
                    cpu_set_flag(Negative, temp  & 0x0080);
                    cpu_set_flag(Zero, (temp & 0x00FF) == 0x0);
                } break;
            case CPY: //compare with Y, Y - M
                {
                    FETCH;
                    u8 temp = (u16)cpu.Yreq - (u16)fetched;
                    cpu_set_flag(Carry, cpu.Yreq >= fetched);
                    cpu_set_flag(Negative, temp  & 0x0080);
                    cpu_set_flag(Zero, (temp & 0x00FF) == 0x0);
                } break;
            case DEC: //decrement, M - 1 -> M or (A - 1 ?? TODO)
                {
                    FETCH;
                    u16 temp = fetched - 1;
                    cpu_set_flag(Negative, temp & 0x80);
                    cpu_set_flag(Zero, (temp & 0x00FF) == 0);
                    bus_write8(addr, temp & 0x00FF);
                } break;
            case DEX: //decrement X, X - 1 -> X
                {
                    cpu.Xreq -= 1;
                    cpu_set_flag(Negative, cpu.Xreq & 0x80);
                    cpu_set_flag(Zero, cpu.Xreq == 0x0);
                } break;
            case DEY: //decrement Y, Y - 1 -> Y
                {
                    cpu.Yreq -= 1;
                    cpu_set_flag(Negative, cpu.Yreq & 0x80);
                    cpu_set_flag(Zero, cpu.Yreq == 0x0);
                } break;
            case EOR: //exclusive or (with accumulator), A EOR M -> A
                {
                    FETCH;
                    cpu.accumReq = cpu.accumReq ^ fetched;
                    cpu_set_flag(Negative, cpu.accumReq & 0x80);
                    cpu_set_flag(Zero, cpu.accumReq == 0x0);
                } break;
            case INC: //increment M + 1 -> M (A - 1 -> A ??TODO)
                {
                    FETCH;
                    u16 temp = (u16)fetched + 1;
                    bus_write8(addr, temp & 0x00FF);

                    cpu_set_flag(Negative, temp & 0x0080);
                    cpu_set_flag(Zero, (temp & 0xFF) == 0x0);
                } break;
            case INX: //increment X, X + 1 -> X
                {
                    cpu.Xreq += 1;

                    cpu_set_flag(Negative, cpu.Xreq & 0x0080);
                    cpu_set_flag(Zero, cpu.Xreq == 0x0);
                } break;
            case INY: //increment Y, Y + 1 -> Y
                {
                    cpu.Yreq += 1;

                    cpu_set_flag(Negative, cpu.Yreq & 0x0080);
                    cpu_set_flag(Zero, cpu.Yreq == 0x0);
                } break;
            case JMP: //jump  (PC+1) -> PCL    (PC+2) -> PCH
                {
                    cpu.pc = addr;
                } break;
            case JSR: //jump subroutine
                {
                    // our actual instruction
                    cpu.pc -= 1;

                    //stack_push(cpu.pc);

                    // push pc
                    stack_push( (cpu.pc >> 8) & 0x00FF );
                    stack_push( cpu.pc & 0x00FF );

                    cpu.pc = addr;
                } break;
            case LDA: //load accumulator, M -> A
                {
                    FETCH;
                    cpu.accumReq = fetched;

                    cpu_set_flag(Negative, cpu.accumReq & 0x0080);
                    cpu_set_flag(Zero, cpu.accumReq == 0x0);
                } break;
            case LDX: //load X
                {
                    FETCH;
                    cpu.Xreq = fetched;

                    cpu_set_flag(Negative, cpu.Xreq & 0x0080);
                    cpu_set_flag(Zero, cpu.Xreq == 0x0);
                } break;
            case LDY: //load Y
                {
                    FETCH;
                    //LOG("LDY Y req 0x%04X Fetched 0x%04X flags 0x%04X", cpu.Yreq, fetched, cpu.flags);
                    cpu.Yreq = fetched;

                    cpu_set_flag(Negative, cpu.Yreq & 0x0080);
                    cpu_set_flag(Zero, cpu.Yreq == 0x0);

                    //LOG("flags 0x%04X", cpu.flags);
                } break;
            case LSR: //logical shift right, 0 -> [76543210] -> C
                {
                    FETCH;
                    u8 temp = fetched >> 1;
                    cpu_set_flag(Carry, fetched & 0x1);
                    cpu_set_flag(Negative, 0);
                    cpu_set_flag(Zero, temp == 0x0);


                    if(instruct.addressMode == IMP || instruct.addressMode == ACCUM) {
                        cpu.accumReq = temp;
                    } else {
                        bus_write8(addr, temp);
                    }

                } break;
            case NOP: //no operation
                {
                    // TODO
                    //ABORT("not legal instruction (NOP TODO implementation)");
                } break;
            case ORA: //or with accumulator,  A OR M -> A
                {
                    FETCH;
                    cpu.accumReq |= fetched;

                    cpu_set_flag(Negative, cpu.accumReq & 0x0080);
                    cpu_set_flag(Zero, cpu.accumReq == 0x0);
                } break;
            case PHA: //push accumulator
                {
                    stack_push(cpu.accumReq);
                } break;
            case PHP: //push processor status (SR)
                {
                    // In the byte pushed, bit 5 is always set to 1,
                    // and bit 4 is 1 if from an instruction (PHP or BRK)

                    //  op      Unused and Break    After push
                    //  PHP     11                  None
                    //  BRK     11                  Break is set to 1
                    //  IRQ     10                  Break is set to 1
                    //  NMI     10                  Break is set to 1

                    stack_push(cpu.flags | Break | Unused);

                    cpu_set_flag(Break, 0);
                    //cpu_set_flag(Unused, 0); // TODO
                } break;
            case PLA: //pull accumulator
                {
                    cpu.accumReq = stack_pop();
                    cpu_set_flag(Negative, cpu.accumReq & 0x80);
                    cpu_set_flag(Zero, cpu.accumReq == 0x0);
                } break;
            case PLP: //pull processor status (SR)
                {
                    cpu.flags = stack_pop();
                } break;
            case ROL: //rotate left,  C <- [76543210] <- C (M or A)
                {
                    FETCH;

                    //LOG("ROL fetched 0x%04X carry 0x%04X", fetched, cpu_get_flag(Carry));

                    u16 temp = (u16)((fetched << 1) | cpu_get_flag(Carry));

                    cpu_set_flag(Negative, temp & 0x0080);
                    cpu_set_flag(Zero, (temp & 0x00FF) == 0x0);
                    cpu_set_flag(Carry, (temp & 0xFF00) > 0);

                    if(instruct.addressMode == IMP || instruct.addressMode == ACCUM) {
                        cpu.accumReq = temp & 0x00FF;
                    } else {
                        bus_write8(addr, temp & 0x00FF);
                    }

                } break;
            case ROR: //rotate right, C -> [76543210] -> C
                {
                    FETCH;
                    u16 temp = (u16)((fetched >> 1) | (cpu_get_flag(Carry) << 7));

                    cpu_set_flag(Negative, temp & 0x0080);
                    cpu_set_flag(Zero, (temp & 0x00FF) == 0x0);
                    cpu_set_flag(Carry, fetched & 0x1);

                    if(instruct.addressMode == IMP || instruct.addressMode == ACCUM) {
                        cpu.accumReq = temp & 0x00FF;
                    } else {
                        bus_write8(addr, temp & 0x00FF);
                    }

                } break;
            case RTI: //return from interrupt
                {
                    cpu_return_from_interrupt();
                } break;
            case RTS: //return from subroutine
                {
                    u16 low = stack_pop();
                    u16 high = stack_pop();
                    cpu.pc = low | (high << 8);
                    cpu.pc += 1;
                } break;
            case SBC: //subtract with carry, A - M - (1 - C) -> A (1 - C is borrow bit)
                {
                    // same as ADC but with inverted M
                    FETCH;

                    fetched ^= 0xFF;

                    // TODO fix
                    u16 temp = (u16)cpu.accumReq + (u16)fetched + (u16)cpu_get_flag(Carry);

                    cpu_set_flag(Carry, temp > 0xFF);
                    cpu_set_flag(Zero, (temp & 0x00FF) == 0x0);
                    cpu_set_flag(Negative, (temp & 0x80) == 0x80);

                    // check overflow
                    // (2 positives result negative) and (2 negatives result positive)
                    // so if two high bits are same on operants and different on result set it

                    // TODO check
                    cpu_set_flag(Overflow, (cpu.accumReq ^ (u8)(temp & 0x00FF)) &
                            (fetched ^ (u8)(temp & 0x00FF)) & 0x80);

                    cpu.accumReq = temp & 0x00FF;

                } break;
            case SEC: //set carry
                {
                    cpu_set_flag(Carry, 1);
                } break;
            case SED: //set decimal
                {
                    cpu_set_flag(DecimalMode, 1);
                    //ABORT("Set decimal should not be called!");
                } break;
            case SEI: //set interrupt disable
                {
                    cpu_set_flag(DisableIterups, 1);
                } break;
            case STA: //store accumulator,  A -> M
                {
                    bus_write8(addr, cpu.accumReq);
                } break;
            case STX: //store X, X -> M
                {
                    bus_write8(addr, cpu.Xreq);
                } break;
            case STY: //store Y, Y -> M
                {
                    bus_write8(addr, cpu.Yreq);
                } break;
            case TAX: //transfer accumulator to X, A -> X
                {
                    cpu.Xreq = cpu.accumReq;
                    cpu_set_flag(Negative, cpu.Xreq & 0x80);
                    cpu_set_flag(Zero, cpu.Xreq == 0x0);
                } break;
            case TAY: //transfer accumulator to Y, A -> Y
                {
                    cpu.Yreq = cpu.accumReq;
                    cpu_set_flag(Negative, cpu.Yreq & 0x80);
                    cpu_set_flag(Zero, cpu.Yreq == 0x0);
                } break;
            case TSX: //transfer stack pointer to X
                {
                    cpu.Xreq = cpu.stackPointer;
                    cpu_set_flag(Negative, cpu.Xreq & 0x80);
                    cpu_set_flag(Zero, cpu.Xreq == 0x0);
                } break;
            case TXA: //transfer X to accumulator, X -> A
                {
                    cpu.accumReq = cpu.Xreq;
                    cpu_set_flag(Negative, cpu.accumReq & 0x80);
                    cpu_set_flag(Zero, cpu.accumReq == 0x0);
                } break;
            case TXS: //transfer X to stack pointer, X -> SP
                {
                    //LOG("TXS stack pointer 0x%04x xreq 0x%04x", cpu.stackPointer, cpu.Xreq);
                    cpu.stackPointer = cpu.Xreq;
                } break;
            case TYA: //transfer Y to accumulator, Y -> A
                {
                    cpu.accumReq = cpu.Yreq;
                    cpu_set_flag(Negative, cpu.accumReq & 0x80);
                    cpu_set_flag(Zero, cpu.accumReq == 0x0);
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
        cpu.instructionCount++;

        if(cpu.pc == breakpoint || cpu.instructionCount == instructionCountBreakPoint) {
            LOG("breakpoint! %d %d", cpu.instructionCount, instructionCountBreakPoint);
            debug = 0;
        }
    }

    cpu.cycles -= 1;


    return ret;
}

#endif /*CPU2AO3_H*/
