/*
 * Operation.hpp
 *
 * This is just a fancy enum that lets me store some extra information
 * alongside the keys. In this case we store an opcode and an operation type.
 * Note that the default constructor and asignment operator are deleted meaning
 * that the only valid Operation variables are the ones defined inside the
 * class itself
 *
 * -- Callum Nicholson
 */
#ifndef LEEK_VM_OPERATION_H_DEFINED
#define LEEK_VM_OPERATION_H_DEFINED

#include <cstdint>

class Operation {
    public:
        enum Mode {
            IIR,
            RIR,
            RRR,
            IR,
            RR,
        };

        static Operation& fromInstruction(uint16_t instruction);
        Mode getMode();

        // Move operations
        static Operation NOP, MOV;
        // Arithmetic operations
        static Operation ADD, ADDC, ADDi, SUB, SUBB, SUBi, ROT, ROTi;
        // Logic operations
        static Operation OR, AND, XOR, NOT;
        // Memory operations
        static Operation STORE, LOAD, LDRf, LDRb, PUSH, POP;
        // Jump and flag operations
        static Operation JMPf, JMPb, FJMP, FSET, FCLR, FTOG;
        // Interrupt operations
        static Operation INTER, WFI;

    private:
        uint8_t opCode;
        Mode    mode;

        Operation(Operation const&)      = delete;
        void operator=(Operation const&) = delete;

        static Operation* shortOps[16];
        static Operation* longOps[16];

        Operation(uint8_t opCode, Mode mode);
};

bool operator==(Operation& lhs, Operation& rhs);
bool operator!=(Operation& lhs, Operation& rhs);

#endif
