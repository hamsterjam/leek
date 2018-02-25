#include "Processor.hpp"
#include "Operation.hpp"
#include <cstdlib>
#include <cstdint>
#include <stdexcept>

Processor::Processor(size_t memWords): mem(memWords) {
    // Do nothing
}

void Processor::run(uint16_t instruction) {
    Operation& op = Operation::fromInstruction(instruction);

    // We need to decrease the stack pointer before resolving inputs if the
    // operation is POP
    if (op == Operation::POP) reg.stack -= 1;

    uint8_t mask = (1 << 4) - 1;

    // Break the instruction into 4 bit chunks
    uint8_t litA = (instruction >> 8) & mask;
    uint8_t litB = (instruction >> 4) & mask;
    uint8_t litC = (instruction >> 0) & mask;

    // Figure out the inputs based on the mode
    uint16_t inA;
    uint16_t inB;

    switch (op.getMode()) {
        case (Operation::IIR):
            inA = (litA << 4) | litB;
            inB = reg[litC];
            break;
        case (Operation::RIR):
            inA = reg[litA];
            inB = litB;
            break;
        case (Operation::RRR):
            inA = reg[litA];
            inB = reg[litB];
            break;
        case (Operation::IR):
            inA = litB;
            inB = reg[litC];
            break;
        case (Operation::RR):
            inA = reg[litB];
            inB = reg[litC];
            break;
    }

    // For some operations inB is an address in memory
    if (op == Operation::LOAD || op == Operation::POP) {
        inA = mem[inA];
    }

    // Compute the result
    // Some operations write to memory, most write to a register
    uint16_t* _res;
    if (op == Operation::STORE || op == Operation::PUSH) {
        _res = &mem[reg[litC]];
    }
    else if (litC == 0) {
        uint16_t dummy = 0;
        _res = &dummy;
    }
    else {
        _res = &reg[litC];
    }
    uint16_t& res = *_res;

    bool setStateFlags = false;

    //
    // Move and Set
    //
    if      (op == Operation::NOP) {
        res = 0;
    }
    else if (op == Operation::MOV) {
        res = inA;
    }
    else if (op == Operation::HSET) {
        res = inB;
        // Clear high 8 bits
        res &= (1 << 8) - 1;
        // Set high 8 bits to inA
        res |= inA << 8;
    }
    else if (op == Operation::LSET) {
        res = inB;
        // Clear low 8 bits
        res &= ~((1 << 8) - 1);
        // Set low 8 bits to inA
        res |= inA;
    }

    //
    // Arithmetic
    //
    else if (op == Operation::ADD || op == Operation::ADDi) {
        res = inA + inB;

        setStateFlags = true;
        // Set the carry flag if we need to carry
        if (res < inA) reg.setBit(13, 0, true);

        // Set the overflow flag if we overflow
        if (inA <  0x8000 && inB <  0x8000 && res >= 0x8000 ||
            inA >= 0x8000 && inB >= 0x8000 && res <  0x8000)
        {
            reg.setBit(13, 1, true);
        }
    }
    else if (op == Operation::SUB || op == Operation::SUBi) {
        res = inA - inB;

        setStateFlags = true;

        // If this is going to be a negative result, flag carry (borrow)
        if (inB > inA) reg.setBit(13, 0, true);

        // Set the overflow flag if we overflow
        if (inA <  0x8000 && inB >= 0x8000 && res >= 0x8000 ||
            inA >= 0x8000 && inB <  0x8000 && res <  0x8000)
        {
            reg.setBit(13, 1, true);
        }
    }
    else if (op == Operation::MUL) {
        unsigned long longRes = (unsigned long) inA * inB;

        // Upper byte stored in ARITH1
        reg.arith1 = longRes >> 16;

        // Lower byte is returned
        res = longRes & ((1 << 16) - 1);

        setStateFlags = true;
    }
    else if (op == Operation::ROT || op == Operation::ROTi) {
        inB %= 16;

        res = inA << inB;
        // Make sure it wraps
        res |= inA >> (16 - inB);

        setStateFlags = true;
    }

    //
    // Logic
    //
    else if (op == Operation::OR) {
        res = inA | inB;

        setStateFlags = true;
    }
    else if (op == Operation::AND) {
        res = inA & inB;

        setStateFlags = true;
    }
    else if (op == Operation::XOR) {
        res = inA ^ inB;

        setStateFlags = true;
    }
    else if (op == Operation::NOT) {
        res = ~inA;

        setStateFlags = true;
    }

    //
    // Memory
    //
    else if (op == Operation::STORE || op == Operation::LOAD) {
        res = inA;
    }
    else if (op == Operation::PUSH) {
        res = inA;
        reg.stack += 1;
    }
    else if (op == Operation::POP) {
        res = inA;
        // Stack pointer is decremented before resolving inputs
    }

    //
    // Jump and Flags
    //
    else if (op == Operation::JMPf) {
        res = inB + inA;
    }
    else if (op == Operation::JMPb) {
        res = inB - inA;
    }
    else if (op == Operation::FJMP) {
        if (!reg.getBit(13, inA)) {
            res = inB + 1;
        }
    }
    else if (op == Operation::FSET) {
        reg.setBit(13, inA, true);
    }
    else if (op == Operation::FCLR) {
        reg.setBit(13, inA, false);
    }
    else if (op == Operation::FTOG) {
        reg.togBit(13, inA);
    }

    // Set zero and negative flags
    if (setStateFlags) {
        reg.setBit(13, 2, res == 0);
        reg.setBit(13, 3, res >= (1 << 15));
    }
}

void Processor::push(uint16_t instruction) {
    // Totally possible to do this with actual instructions, but this is cleaner
    mem[reg.stack] = instruction;
    reg.stack += 1;
}

void Processor::tick() {
    uint16_t pc = reg.pc;
    reg.pc += 1;
    run(mem[pc]);
}

uint16_t Processor::inspect(size_t index) {
    return reg[index];
}
