#include "Operation.hpp"
#include <assert.h>
#include <cstdint>

Operation::Operation(uint8_t opCode, Operation::Mode mode) {
    this->opCode = opCode;
    this->mode   = mode;

    if (mode == IR || mode == RR) {
        Operation::shortOps[opCode] = this;
    }
    else {
        Operation::longOps[opCode] = this;
    }
}

Operation& Operation::fromInstruction(uint16_t instruction) {
    // Get top most 8 bits
    uint16_t top8 = instruction >> 8;

    // Split that into its top and low 4 bits
    uint16_t opLo = top8 & ((1 << 4) - 1);
    uint16_t opHi = top8 >> 4;

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
Operation Operation::ROT(  0x7, RRR);
Operation Operation::ROTi( 0x8, RIR);
// Logic operations
Operation Operation::OR(   0x9, RRR);
Operation Operation::AND(  0xa, RRR);
Operation Operation::XOR(  0xb, RRR);
Operation Operation::NOT(  0x2, RR);
// Memory operations
Operation Operation::STORE(0x3, RR);
Operation Operation::LOAD( 0x4, RR);
Operation Operation::LDRf( 0xc, IIR);
Operation Operation::LDRb( 0xd, IIR);
Operation Operation::PUSH( 0x5, RR);
Operation Operation::POP(  0x6, RR);
// Jump and flag operations
Operation Operation::JMPf( 0xe, IIR);
Operation Operation::JMPb( 0xf, IIR);
Operation Operation::FJMP( 0x7, IR);
Operation Operation::FSET( 0x8, IR);
Operation Operation::FCLR( 0x9, IR);
Operation Operation::FTOG( 0xa, IR);
// Other operations
Operation Operation::INTER(0xb, RR);
Operation Operation::WFI(  0xc, RR);

Operation* Operation::shortOps[16];
Operation* Operation::longOps[16];
