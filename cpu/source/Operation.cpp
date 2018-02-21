#include <Operation.hpp>
#include <assert.h>

Operation::Operation(unsigned char opCode, Operation::Mode mode) {
    this->opCode = opCode;
    this->mode   = mode;

    if (mode == IR || mode == RR) {
        Operation::shortOps[opCode] = this;
    }
    else {
        Operation::longOps[opCode] = this;
    }
}

Operation& Operation::fromInstruction(unsigned int instruction) {
    // Get top most 8 bits
    unsigned int top8 = instruction >> 8;

    // Split that into its top and low 4 bits
    unsigned int opLo = top8 & ((1 << 4) - 1);
    unsigned int opHi = top8 >> 4;

    // These should be at most 4 bits long (< 2^4)
    assert(opLo < 16);
    assert(opHi < 16);

    if (opHi == 0) {
        return *Operation::shortOps[opLo];
    }
    else {
        return *Operation::longOps[opHi];
    }
}

Operation::Mode Operation::getMode() {
    return this->mode;
}

bool operator==(Operation& lhs, Operation& rhs) {
    // These are immutable and unique, just compare addresses
    return &lhs == &rhs;
}

bool operator!=(Operation& lhs, Operation& rhs) {
    return !(lhs == rhs);
}

// Move and set operations
Operation Operation::NOP(  0x0, RR);
Operation Operation::MOV(  0x1, RR);
Operation Operation::HSET( 0x1, IIR);
Operation Operation::LSET( 0x2, IIR);
// Arithmetic operations
Operation Operation::ADD(  0x3, RRR);
Operation Operation::ADDi( 0x4, RIR);
Operation Operation::SUB(  0x5, RRR);
Operation Operation::SUBi( 0x6, RIR);
Operation Operation::MUL(  0x7, RRR);
Operation Operation::ROT(  0x8, RRR);
Operation Operation::ROTi( 0x9, RIR);
// Logic operations
Operation Operation::OR(   0xa, RRR);
Operation Operation::AND(  0xb, RRR);
Operation Operation::XOR(  0xc, RRR);
Operation Operation::NOT(  0x2, RR);
// Memory operations
Operation Operation::STORE(0x3, RR);
Operation Operation::LOAD( 0x4, RR);
Operation Operation::PUSH( 0x5, RR);
Operation Operation::POP(  0x6, RR);
// Jump and flag operations
Operation Operation::JMP(  0xd, RIR);
Operation Operation::FJMP( 0xe, RIR);
Operation Operation::FSET( 0x7, IR);
Operation Operation::FCLR( 0x8, IR);
Operation Operation::FTOG( 0x9, IR);

Operation* Operation::shortOps[16];
Operation* Operation::longOps[16];
