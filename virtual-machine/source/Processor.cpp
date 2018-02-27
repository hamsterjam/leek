#include "Processor.hpp"
#include "Operation.hpp"
#include <cstdlib>
#include <cstdint>
#include <stdexcept>
#include <atomic_ops.h>

Processor::Processor(size_t memWords): mem(memWords) {
    AO_test_and_set(&shouldInterrupt);
    AO_test_and_set(&softISF);
    for (int i = 0; i < 8; ++i) {
        AO_test_and_set(&hardISF[i]);
    }
}

void Processor::run(uint16_t instruction) {
    const uint8_t ZERO_FLAG  = 0;
    const uint8_t NEG_FLAG   = 1;
    const uint8_t CARRY_FLAG = 2;
    const uint8_t OVER_FLAG  = 3;

    Operation& op = Operation::fromInstruction(instruction);

    // We need to increase the stack pointer before resolving inputs if the
    // operation is PUSH
    if (op == Operation::PUSH) reg[RegisterManager::STACK] += 1;

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
        bool carry = res < inA;
        reg.setBit(RegisterManager::FLAGS, CARRY_FLAG, carry);

        // Set the overflow flag if we overflow
        bool over = inA <  0x8000 && inB <  0x8000 && res >= 0x8000 ||
                    inA >= 0x8000 && inB >= 0x8000 && res <  0x8000;

        reg.setBit(RegisterManager::FLAGS, OVER_FLAG, over);
    }
    else if (op == Operation::SUB || op == Operation::SUBi) {
        res = inA - inB;

        setStateFlags = true;

        // If this is going to be a negative result, flag carry (borrow)
        bool carry = inB > inA;
        reg.setBit(RegisterManager::FLAGS, CARRY_FLAG, carry);

        // Set the overflow flag if we overflow
        bool over = inA <  0x8000 && inB >= 0x8000 && res >= 0x8000 ||
                    inA >= 0x8000 && inB <  0x8000 && res <  0x8000;
        reg.setBit(RegisterManager::FLAGS, OVER_FLAG, over);
    }
    else if (op == Operation::MUL) {
        unsigned long longRes = (unsigned long) inA * inB;

        // Upper byte stored in AUX
        reg[RegisterManager::AUX] = longRes >> 16;

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
        // Stack pointer is already incremeneted
    }
    else if (op == Operation::POP) {
        res = inA;
        reg[RegisterManager::STACK] -= 1;
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
        if (!reg.getBit(RegisterManager::FLAGS, inA)) {
            res = inB + 1;
        }
    }
    else if (op == Operation::FSET) {
        reg.setBit(RegisterManager::FLAGS, inA, true);
    }
    else if (op == Operation::FCLR) {
        reg.setBit(RegisterManager::FLAGS, inA, false);
    }
    else if (op == Operation::FTOG) {
        reg.togBit(RegisterManager::FLAGS, inA);
    }

    // Set zero and negative flags
    if (setStateFlags) {
        reg.setBit(RegisterManager::FLAGS, ZERO_FLAG, res == 0);
        reg.setBit(RegisterManager::FLAGS, NEG_FLAG,  res & (1 << 15));
    }
}

void Processor::push(uint16_t instruction) {
    // Totally possible to do this with actual instructions, but this is cleaner
    reg[RegisterManager::STACK] += 1;
    mem[reg[RegisterManager::STACK]] = instruction;
}

void Processor::tick() {
    const size_t ISFs = 7;
    const size_t ISF0 = 8;
    const size_t ICF  = 4;

    bool softISF = !AO_test_and_set(&this->softISF);
    reg.setBit(RegisterManager::FLAGS, ISFs, softISF);

    for (int i = 0; i < 8; ++i) {
        bool hardISF = !AO_test_and_set(&this->hardISF[i]);
        reg.setBit(RegisterManager::FLAGS, ISF0 + i, hardISF);
    }

    bool shouldInterrupt = !AO_test_and_set(&this->shouldInterrupt);

    if (shouldInterrupt && reg.getBit(RegisterManager::FLAGS, ICF)) {

        reg.setBit(RegisterManager::FLAGS, ICF, false);
        // Reset shouldInterrupt as it might of been cleared between reading it
        // and setting the ICF in the FLAGS register
        AO_test_and_set(&this->shouldInterrupt);
        push(reg[RegisterManager::PC]);
        reg[RegisterManager::PC] = reg[RegisterManager::IHP];
    }
    else {
        uint16_t pc = reg[RegisterManager::PC];
        reg[RegisterManager::PC] += 1;
        run(mem[pc]);
    }
}

void Processor::interrupt(int line) {
    if (line > 7) {
        throw std::out_of_range("Processor::interrupt");
    }
    AO_CLEAR(&shouldInterrupt);

    if (line < 0) {
        AO_CLEAR(&softISF);
    }
    else {
        AO_CLEAR(&hardISF[line]);
    }
}

uint16_t Processor::inspect(size_t index) {
    return reg[index];
}
