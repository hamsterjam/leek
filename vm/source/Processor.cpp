#include <Processor.hpp>
#include <Operation.hpp>
#include <cstdlib>
#include <stdexcept>

Processor::Processor(size_t memWords): mem(memWords) {
    // Do nothing
}

void Processor::run(unsigned int instruction) {
    Operation& op = Operation::fromInstruction(instruction);

    unsigned char mask = (1 << 4) - 1;

    // Break the instruction into 4 bit chunks
    unsigned char litA = (instruction >> 8) & mask;
    unsigned char litB = (instruction >> 4) & mask;
    unsigned char litC = (instruction >> 0) & mask;

    // Figure out the inputs based on the mode
    unsigned int inA;
    unsigned int inB;

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
            inA = 0;
            inB = litB;
            break;
        case (Operation::RR):
            inA = 0;
            inB = reg[litB];
            break;
    }

    // For some operations inB is an address in memory
    if (op == Operation::LOAD || op == Operation::POP) {
        inB = mem[inB];
    }

    // Compute the result
    // Some operations write to memory, most write to a register
    // Yes, I do have to do this with the ?: operator
    unsigned int& res = (op == Operation::STORE || op == Operation::PUSH) ? mem[reg[litC]] : reg[litC];

    //
    // Move and Set
    //
    if      (op == Operation::NOP) {
        res = 0;
    }
    else if (op == Operation::MOV) {
        res = inB;
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

        // Set the carry flag if we overflow
        if (res < inA) reg.setBit(13, 0, true);

        //TODO// Figure out overflow flag
    }
    else if (op == Operation::SUB || op == Operation::SUBi) {
        res = inA - inB;

        //TODO// Figure out overflow flag
    }
    else if (op == Operation::MUL) {
        unsigned long longRes = (unsigned long) inA * inB;

        // Upper byte stored in ARITH1
        reg.arith1 = longRes >> 16;

        // Lower byte is returned
        res = longRes & ((1 << 16) - 1);
    }
    else if (op == Operation::ROT || op == Operation::ROTi) {
        inB %= 16;

        res = inA << inB;
        // Make sure it wraps
        res |= inA >> (16 - inB);
    }

    //
    // Logic
    //
    else if (op == Operation::OR) {
        res = inA | inB;
    }
    else if (op == Operation::AND) {
        res = inA & inB;
    }
    else if (op == Operation::XOR) {
        res = inA ^ inB;
    }
    else if (op == Operation::NOT) {
        res = ~inB;
    }

    //
    // Memory
    //
    else if (op == Operation::STORE || op == Operation::LOAD) {
        res = inB;
    }
    else if (op == Operation::PUSH) {
        res = inB;
        reg.stack += 1;
    }
    else if (op == Operation::POP) {
        res = inB;
        reg.stack -= 1;
    }

    //
    // Jump and Flags
    //
    else if (op == Operation::JMP) {
        res = inA + inB;
    }
    else if (op == Operation::FJMP) {
        if (reg.getBit(13, inB)) {
            res = inA;
        }
    }
    else if (op == Operation::FSET) {
        reg.setBit(13, inB, true);
    }
    else if (op == Operation::FCLR) {
        reg.setBit(13, inB, false);
    }
    else if (op == Operation::FTOG) {
        reg.togBit(13, inB);
    }

    // Set zero and negative flags
    reg.setBit(13, 2, res == 0);
    reg.setBit(13, 3, res >= (1 << 15));
}

void Processor::tick() {
    unsigned int& pc = this->reg.pc;
    ++pc;
    run(this->mem[pc]);
}
