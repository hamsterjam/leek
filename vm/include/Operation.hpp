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
#ifndef KOTO_VM_OPERATION_H_DEFINED
#define KOTO_VM_OPERATION_H_DEFINED

class Operation {
    public:
        enum Mode {
            IIR,
            RIR,
            RRR,
            IR,
            RR,
        };

        static Operation& fromInstruction(unsigned int instruction);
        Mode getMode();

        // Move and set operations
        static Operation NOP, MOV, HSET, LSET;
        // Arithmetic operations
        static Operation ADD, ADDi, SUB, SUBi, MUL, ROT, ROTi;
        // Logic operations
        static Operation OR, AND, XOR, NOT;
        // Memory operations
        static Operation STORE, LOAD, PUSH, POP;
        // Jump and flag operations
        static Operation JMP, FJMP, FSET, FCLR, FTOG;

    private:
        unsigned char opCode;
        Mode          mode;

        Operation(Operation const&)      = delete;
        void operator=(Operation const&) = delete;

        static Operation* shortOps[16];
        static Operation* longOps[16];

        Operation(unsigned char opCode, Mode mode);
};

bool operator==(Operation& lhs, Operation& rhs);
bool operator!=(Operation& lhs, Operation& rhs);

#endif
