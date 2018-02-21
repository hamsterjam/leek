#include <Operation.hpp>
#include <iostream>

using namespace std;

void testShortOp(unsigned int opCode, Operation& op) {
    bool succ = true;

    unsigned int instrHi = opCode << 8;
    for (unsigned int instrLo = 0; instrLo <= 0xff; ++instrLo) {
        unsigned int instr = instrHi | instrLo;
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

void testLongOp(unsigned int opCode, Operation& op) {
    bool succ = true;

    unsigned int instrHi = opCode << 12;
    for (unsigned int instrLo = 0; instrLo <= 0xfff; ++instrLo) {
        unsigned int instr = instrHi | instrLo;
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

    cout << "Move and set opCodes:" << endl;

    cout << "Testing NOP...   \t";
    testShortOp(0x0, Operation::NOP);

    cout << "Testing MOV...   \t";
    testShortOp(0x1, Operation::MOV);

    cout << "Testing HSET...  \t";
    testLongOp(0x1, Operation::HSET);

    cout << "Testing LSET...  \t";
    testLongOp(0x2, Operation::LSET);


    cout << endl;
    cout << "Arithmetic opCodes:" << endl;

    cout << "Testing ADD...   \t";
    testLongOp(0x3, Operation::ADD);

    cout << "Testing ADDi...  \t";
    testLongOp(0x4, Operation::ADDi);

    cout << "Testing SUB...   \t";
    testLongOp(0x5, Operation::SUB);

    cout << "Testing SUBi...  \t";
    testLongOp(0x6, Operation::SUBi);

    cout << "Testing MUL...   \t";
    testLongOp(0x7, Operation::MUL);

    cout << "Testing ROT...   \t";
    testLongOp(0x8, Operation::ROT);

    cout << "Testing ROTi...  \t";
    testLongOp(0x9, Operation::ROTi);


    cout << endl;
    cout << "Logic opCodes:" << endl;

    cout << "Testing OR...    \t";
    testLongOp(0xa, Operation::OR);

    cout << "Testing AND...   \t";
    testLongOp(0xb, Operation::AND);

    cout << "Testing XOR...   \t";
    testLongOp(0xc, Operation::XOR);

    cout << "Testing NOT...   \t";
    testShortOp(0x2, Operation::NOT);


    cout << endl;
    cout << "Memory opCodes:" << endl;

    cout << "Testing STORE... \t";
    testShortOp(0x3, Operation::STORE);

    cout << "Testing LOAD...  \t";
    testShortOp(0x4, Operation::LOAD);

    cout << "Testing PUSH...  \t";
    testShortOp(0x5, Operation::PUSH);

    cout << "Testing POP...   \t";
    testShortOp(0x6, Operation::POP);


    cout << endl;
    cout << "Jump and flag opCodes:" << endl;

    cout << "Testing JMP...   \t";
    testLongOp(0xd, Operation::JMP);

    cout << "Testing FJMP...  \t";
    testLongOp(0xe, Operation::FJMP);

    cout << "Testing FSET...  \t";
    testShortOp(0x7, Operation::FSET);

    cout << "Testing FCLR...  \t";
    testShortOp(0x8, Operation::FCLR);

    cout << "Testing FTOG...  \t";
    testShortOp(0x9, Operation::FTOG);


    cout << endl;
    cout << "Other tests:" << endl;

    cout << "Testing uniqueness... \t";
    {
        // This is a big chain of comparisons, this is sufficent to ensure that
        // these variables are pairwise unique
        bool unique = true;
        unique = unique && Operation::NOP   != Operation::MOV;
        unique = unique && Operation::MOV   != Operation::HSET;
        unique = unique && Operation::HSET  != Operation::LSET;
        unique = unique && Operation::LSET  != Operation::ADD;
        unique = unique && Operation::ADD   != Operation::ADDi;
        unique = unique && Operation::ADDi  != Operation::SUB;
        unique = unique && Operation::SUB   != Operation::SUBi;
        unique = unique && Operation::MUL   != Operation::ROT;
        unique = unique && Operation::ROT   != Operation::ROTi;
        unique = unique && Operation::ROTi  != Operation::OR;
        unique = unique && Operation::OR    != Operation::AND;
        unique = unique && Operation::AND   != Operation::XOR;
        unique = unique && Operation::XOR   != Operation::NOT;
        unique = unique && Operation::NOT   != Operation::STORE;
        unique = unique && Operation::STORE != Operation::LOAD;
        unique = unique && Operation::LOAD  != Operation::PUSH;
        unique = unique && Operation::PUSH  != Operation::POP;
        unique = unique && Operation::POP   != Operation::JMP;
        unique = unique && Operation::JMP   != Operation::FJMP;
        unique = unique && Operation::FJMP  != Operation::FSET;
        unique = unique && Operation::FSET  != Operation::FCLR;
        unique = unique && Operation::FCLR  != Operation::FTOG;

        if (unique) {
            cout << "OK!" << endl;
        }
        else {
            cout << "Failed" << endl;
        }
    }
    return 0;
}
