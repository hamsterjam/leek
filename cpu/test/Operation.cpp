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

    cout << "Testing NOP...   \t";
    testShortOp(0x0, Operation::NOP);

    cout << "Testing MOV...   \t";
    testShortOp(0x1, Operation::MOV);

    cout << "Testing HSET...  \t";
    testLongOp(0x1, Operation::HSET);

    cout << "Testing LSET...  \t";
    testLongOp(0x2, Operation::LSET);

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

    cout << "Testing OR...    \t";
    testLongOp(0xa, Operation::OR);

    cout << "Testing AND...   \t";
    testLongOp(0xb, Operation::AND);

    cout << "Testing XOR...   \t";
    testLongOp(0xc, Operation::XOR);

    cout << "Testing NOT...   \t";
    testShortOp(0x2, Operation::NOT);

    cout << "Testing STORE... \t";
    testShortOp(0x3, Operation::STORE);

    cout << "Testing LOAD...  \t";
    testShortOp(0x4, Operation::LOAD);

    cout << "Testing PUSH...  \t";
    testShortOp(0x5, Operation::PUSH);

    cout << "Testing POP...   \t";
    testShortOp(0x6, Operation::POP);

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
    return 0;
}
