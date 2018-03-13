#include "Operation.hpp"

#include <iostream>
#include <cstdint>

using namespace std;

void testShortOp(uint16_t opCode, Operation& op) {
    bool succ = true;

    uint16_t instrHi = opCode << 8;
    for (uint16_t instrLo = 0; instrLo <= 0xff; ++instrLo) {
        uint16_t instr = instrHi | instrLo;
        if (Operation::fromInstruction(instr) != op) {
            succ = false;
            break;
        }
    }

    if (succ) {
        cout << "OK!" << endl;
    }
    else {
        cout << "Failed" << endl;
    }
}

void testLongOp(uint16_t opCode, Operation& op) {
    bool succ = true;

    uint16_t instrHi = opCode << 12;
    for (uint16_t instrLo = 0; instrLo <= 0xfff; ++instrLo) {
        uint16_t instr = instrHi | instrLo;
        if (Operation::fromInstruction(instr) != op) {
            succ = false;
            break;
        }
    }

    if (succ) {
        cout << "OK!" << endl;
    }
    else {
        cout << "Failed" << endl;
    }
}

int main(int argc, char** argv) {
    // Just going to run every valid instruction to see if it gives the right
    // operation. Note that as some operations require some parts of the
    // instruction to be 0 (such as the destination for flag operations), this
    // actually tests some isntructions that are technically undefined.

    cout << "Move opCodes:" << endl;

    cout << "Testing NOP...   \t" << flush;
    testShortOp(0x0, Operation::NOP);

    cout << "Testing MOV...   \t" << flush;
    testShortOp(0x1, Operation::MOV);

    cout << "Testing REL+...  \t" << flush;
    testLongOp(0x1, Operation::RELp);

    cout << "Testing REL-...  \t" << flush;
    testLongOp(0x2, Operation::RELm);


    cout << endl;
    cout << "Arithmetic opCodes:" << endl;

    cout << "Testing ADD...   \t" << flush;
    testLongOp(0x3, Operation::ADD);

    cout << "Testing ADDC...  \t" << flush;
    testLongOp(0x4, Operation::ADDC);

    cout << "Testing ADDi...  \t" << flush;
    testLongOp(0x5, Operation::ADDi);

    cout << "Testing SUB...   \t" << flush;
    testLongOp(0x6, Operation::SUB);

    cout << "Testing SUBB...  \t" << flush;
    testLongOp(0x7, Operation::SUBB);

    cout << "Testing SUBi...  \t" << flush;
    testLongOp(0x8, Operation::SUBi);

    cout << "Testing MUL...   \t" << flush;
    testLongOp(0x9, Operation::MUL);

    cout << "Testing DIV...   \t" << flush;
    testLongOp(0xa, Operation::DIV);

    cout << "Testing ROT...   \t" << flush;
    testLongOp(0xb, Operation::ROT);

    cout << "Testing ROTi...  \t" << flush;
    testLongOp(0xc, Operation::ROTi);


    cout << endl;
    cout << "Logic opCodes:" << endl;

    cout << "Testing OR...    \t" << flush;
    testLongOp(0xd, Operation::OR);

    cout << "Testing AND...   \t" << flush;
    testLongOp(0xe, Operation::AND);

    cout << "Testing XOR...   \t" << flush;
    testLongOp(0xf, Operation::XOR);

    cout << "Testing NOT...   \t" << flush;
    testShortOp(0x2, Operation::NOT);


    cout << endl;
    cout << "Memory opCodes:" << endl;

    cout << "Testing STORE... \t" << flush;
    testShortOp(0x3, Operation::STORE);

    cout << "Testing LOAD...  \t" << flush;
    testShortOp(0x4, Operation::LOAD);

    cout << "Testing PUSH...  \t" << flush;
    testShortOp(0x5, Operation::PUSH);

    cout << "Testing POP...   \t" << flush;
    testShortOp(0x6, Operation::POP);


    cout << endl;
    cout << "Jump and flag opCodes:" << endl;

    cout << "Testing FJMP...  \t" << flush;
    testShortOp(0x7, Operation::FJMP);

    cout << "Testing FSET...  \t" << flush;
    testShortOp(0x8, Operation::FSET);

    cout << "Testing FCLR...  \t" << flush;
    testShortOp(0x9, Operation::FCLR);


    cout << "Testing FTOG...  \t" << flush;
    testShortOp(0xa, Operation::FTOG);


    cout << endl;
    cout << "Interrupt opCodes:" << endl;

    cout << "Testing INTER... \t" << flush;
    testShortOp(0xb, Operation::INTER);

    cout << "Testing WFI...   \t" << flush;
    testShortOp(0xc, Operation::WFI);


    cout << endl;
    cout << "Other tests:" << endl;

    cout << "Testing uniqueness... \t" << flush;
    {
        // This is a big chain of comparisons, this is sufficent to ensure that
        // these variables are pairwise unique
        bool unique = true;
        unique = unique && Operation::NOP   != Operation::MOV;
        unique = unique && Operation::MOV   != Operation::RELp;
        unique = unique && Operation::RELp  != Operation::RELm;
        unique = unique && Operation::RELm  != Operation::ADD;
        unique = unique && Operation::ADD   != Operation::ADDC;
        unique = unique && Operation::ADDC  != Operation::ADDi;
        unique = unique && Operation::ADDi  != Operation::SUB;
        unique = unique && Operation::SUB   != Operation::SUBB;
        unique = unique && Operation::SUBB  != Operation::SUBi;
        unique = unique && Operation::SUBi  != Operation::MUL;
        unique = unique && Operation::MUL   != Operation::DIV;
        unique = unique && Operation::DIV   != Operation::ROT;
        unique = unique && Operation::ROT   != Operation::ROTi;
        unique = unique && Operation::ROTi  != Operation::OR;
        unique = unique && Operation::OR    != Operation::AND;
        unique = unique && Operation::AND   != Operation::XOR;
        unique = unique && Operation::XOR   != Operation::NOT;
        unique = unique && Operation::NOT   != Operation::STORE;
        unique = unique && Operation::STORE != Operation::LOAD;
        unique = unique && Operation::LOAD  != Operation::PUSH;
        unique = unique && Operation::PUSH  != Operation::POP;
        unique = unique && Operation::POP   != Operation::FJMP;
        unique = unique && Operation::FJMP  != Operation::FSET;
        unique = unique && Operation::FSET  != Operation::FCLR;
        unique = unique && Operation::FCLR  != Operation::FTOG;
        unique = unique && Operation::FTOG  != Operation::INTER;

        if (unique) {
            cout << "OK!" << endl;
        }
        else {
            cout << "Failed" << endl;
        }
    }
    return 0;
}
