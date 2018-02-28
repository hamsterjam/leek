#include "Processor.hpp"

#include <iostream>
#include <cstdlib>
#include <cstdint>

using namespace std;

const uint8_t ZERO_FLAG  = 0;
const uint8_t NEG_FLAG   = 1;
const uint8_t CARRY_FLAG = 2;
const uint8_t OVER_FLAG  = 3;

const uint8_t ZERO_MASK  = 1 << ZERO_FLAG;
const uint8_t NEG_MASK   = 1 << NEG_FLAG;
const uint8_t CARRY_MASK = 1 << CARRY_FLAG;
const uint8_t OVER_MASK  = 1 << OVER_FLAG;

int main(int argc, char** argv) {
    Processor test(0x10000); // 64k

    {
        // Test HSET and LSET by setting every register to every possible
        // 16 bit value and making sure it actually gets to the register
        cout << "Testing HSET and LSET... \t" << flush;

        bool pass = true;

        for (size_t reg = 1; reg <= 10 && pass; ++reg) {
            uint8_t valLo = 0;
            uint8_t valHi = 0;
            // I actually found a use for a do-while loop. These will loop
            // through every value of valLo and valHi
            do {
                do {
                    test.exec(0x1000 | valHi << 4 | reg); // HSET valHi reg
                    test.exec(0x2000 | valLo << 4 | reg); // LSET valLo reg

                    uint16_t val = valHi << 8 | valLo;
                    if (test.inspect(reg) != val) {
                        pass = false;
                        break;
                    }

                } while (++valLo != 0);
            } while (++valHi != 0 && pass);
        }

        if (pass) {
            cout << "OK!" << endl;
        }
        else {
            cout << "Fail" << endl;
        }
    }

    {
        // Test MBZ permanance by attempting to set register 0 to 0xffff
        cout << "Testing MBZ permanence...\t" << flush;

        test.exec(0x1ff0);
        test.exec(0x2ff0);

        if (test.inspect(0) == 0) {
            cout << "OK!" << endl;
        }
        else {
            cout << "Fail" << endl;
        }
    }

    {
        // Test MOV by moving every possible value from every general purpose
        // register to every general purpose register (including itself)
        cout << "Testing MOV... \t\t\t" << flush;

        bool pass = true;

        for (size_t src = 1; src <= 10 && pass; ++src) {

            uint8_t valLo = 0;
            uint8_t valHi = 0;

            do {
                do {
                    uint16_t val = valHi << 8 | valLo;
                    for (size_t dest = 1; dest <= 10; ++dest) {
                        test.exec(0x1000 | valHi << 4 | src); // HSET valHi src
                        test.exec(0x2000 | valLo << 4 | src); // LSET valLo src
                        test.exec(0x0100 | src << 4 | dest);  // MOV src dest

                        if (test.inspect(dest) != val) {
                            pass = false;
                            break;
                        }
                    }
                } while (++valLo != 0 && pass);
            } while (++valHi != 0 && pass);
        }

        if (pass) {
            cout << "OK!" << endl;
        }
        else {
            cout << "Failed" << endl;
        }
    }

    {
        // Test FSET, FCLR, and FTOG by calling each on every bit of every
        // possible value
        cout << "Testing flag operations... \t" << flush;

        bool pass = true;

        uint8_t valLo;
        uint8_t valHi;
        do {
            do {
                uint16_t val = valHi << 8 | valLo;
                for (size_t bit = 0; bit < 16; ++bit) {
                    uint16_t corrVal;

                    test.exec(0x100d | valHi << 4); // HSET valHi FLAGS
                    test.exec(0x200d | valLo << 4); // LSET valLo FLAGS
                    test.exec(0x080d | bit << 4);   // FSET bit

                    corrVal = val | (1 << bit);
                    if (test.inspect(13) != corrVal) {
                        pass = false;
                        break;
                    }

                    test.exec(0x100d | valHi << 4); // HSET valHi FLAGS
                    test.exec(0x200d | valLo << 4); // LSET valLo FLAGS
                    test.exec(0x090d | bit << 4);   // FCLR bit

                    corrVal = val & ~(1 << bit);
                    if (test.inspect(13) != corrVal) {
                        pass = false;
                        break;
                    }

                    test.exec(0x100d | valHi << 4); // HSET valHi FLAGS
                    test.exec(0x200d | valLo << 4); // LSET valLo FLAGS
                    test.exec(0x0a0d | bit << 4);   // FTOG bit

                    corrVal = val ^ (1 << bit);
                    if (test.inspect(13) != corrVal) {
                        pass = false;
                        break;
                    }
                }
            } while (++valHi != 0 && pass);
        } while (++valLo != 0 && pass);

        if (pass) {
            cout << "OK!" << endl;
        }
        else {
            cout << "Fail" << endl;
        }
    }

    {
        // Testing ADD by testing every value of the upper 8 bits
        cout << "Testing ADD... \t\t\t" << flush;

        bool pass = true;

        uint8_t val1Hi = 0x00;
        uint8_t val2Hi = 0x00;
        do {
            do {
                test.exec(0x080d);               // FCLR 0
                test.exec(0x081d);               // FCLR 1
                test.exec(0x0101);               // MOV 0 1
                test.exec(0x1001 | val1Hi << 4); // HSET val1Hi 1
                test.exec(0x0102);               // MOV 0 2
                test.exec(0x1002 | val2Hi << 4); // HSET val2Hi 2
                test.exec(0x3123);               // ADD 1 2 3

                // Make sure the value is right
                uint16_t corrRes = (val1Hi + val2Hi) << 8;
                uint32_t longRes = ((uint32_t) val1Hi + val2Hi) << 8;
                if (test.inspect(3) != corrRes) {
                    pass = false;
                    break;
                }

                // Check if the CARRY flag is set correctly
                if ((longRes != corrRes) == !(test.inspect(13) & CARRY_MASK)) {
                    pass = false;
                    break;
                }

                // Check if the OVER flag is set correctly
                int16_t sVal1 = (* (int16_t*) &val1Hi) << 8;
                int16_t sVal2 = (* (int16_t*) &val2Hi) << 8;

                int16_t sCorrRes = sVal1 + sVal2;
                int32_t sLongRes = sVal1 + sVal2;

                if ((sCorrRes != sLongRes) == !(test.inspect(13) & OVER_MASK)) {
                    pass = false;
                    break;
                }
            } while (++val2Hi != 0);
        } while (++val1Hi != 0 && pass);

        if (pass) {
            cout << "OK!" << endl;
        }
        else {
            cout << "Fail" << endl;
        }
    }

    {
        // Test SUB in the same way we test ADD
        cout << "Testing SUB... \t\t\t" << flush;

        bool pass = true;

        uint8_t val1Hi = 0;
        uint8_t val2Hi = 0;
        do {
            do {
                test.exec(0x080d);               // FCLR 0
                test.exec(0x081d);               // FCLR 1
                test.exec(0x0101);               // MOV 0 1
                test.exec(0x1001 | val1Hi << 4); // HSET val1Hi 1
                test.exec(0x0102);               // MOV 0 2
                test.exec(0x1002 | val2Hi << 4); // HSET val2Hi 2
                test.exec(0x5123);               // SUB 1 2 3

                // Check we have the correct value
                uint16_t corrRes = (val1Hi - val2Hi) << 8;
                uint32_t longRes = ((uint32_t) val1Hi - val2Hi) << 8;
                if (test.inspect(3) != corrRes) {
                    pass = false;
                    break;
                }

                // Check if the CARRY flag is set correctly
                if ((longRes != corrRes) == !(test.inspect(13) & CARRY_MASK)) {
                    pass = false;
                    break;
                }

                // Check if the OVER flag is set correctly
                int16_t sVal1 = (* (int16_t*) &val1Hi) << 8;
                int16_t sVal2 = (* (int16_t*) &val2Hi) << 8;

                int16_t sCorrRes = sVal1 - sVal2;
                int32_t sLongRes = (int32_t) sVal1 - sVal2;

                if ((sLongRes != sCorrRes) == !(test.inspect(13) & OVER_MASK)) {
                    pass = false;
                    break;
                }

            } while (++val2Hi != 0);
        } while (++val1Hi != 0 && pass);

        if (pass) {
            cout << "OK!" << endl;
        }
        else {
            cout << "Fail" << endl;
        }
    }

    {
        // Testing only the high or low portion of a number will yield
        // unintersting results, instead im going to shift my 8 bit numbers 4
        // bits so they straddle the center of the 16 bit number
        cout << "Testing MUL... \t\t\t" << flush;

        bool pass = true;

        uint8_t val1 = 0;
        uint8_t val2 = 0;
        do {
            uint8_t val1Lo = val1 << 4;
            uint8_t val1Hi = val1 >> 4;
            do {
                uint8_t val2Lo = val2 << 4;
                uint8_t val2Hi = val2 >> 4;

                test.exec(0x1001 | val1Hi << 4); // HSET val1Hi 1
                test.exec(0x2001 | val1Lo << 4); // LSET val1Lo 1
                test.exec(0x1002 | val2Hi << 4); // HSET val2Hi 2
                test.exec(0x2002 | val2Lo << 4); // LSET val2Lo 2
                test.exec(0x7123);               // MUL 1 2 3

                uint32_t corrRes = ((uint32_t) val1 * val2) << 8;
                uint32_t testRes = ((uint32_t) test.inspect(11) << 16) | test.inspect(3);

                if (corrRes != testRes) {
                    pass = false;
                    break;
                }
            } while (++val2 != 0);
        } while (++val1 != 0 && pass);

        if (pass) {
            cout << "OK!" << endl;
        }
        else {
            cout << "Fail" << endl;
        }
    }

    {
        // Testing ROT by testing all rotations of all possible values
        cout << "Testing ROT... \t\t\t" << flush;

        bool pass = true;

        uint8_t valLo = 0;
        uint8_t valHi = 0;
        do {
            do {
                uint16_t val = valHi << 8 | valLo;
                for (uint8_t rot = 0; rot < 16; ++rot) {
                    test.exec(0x1001 | valHi << 4); // HSET valHi 1
                    test.exec(0x2001 | valLo << 4); // LSET valLo 1
                    test.exec(0x0102);              // MOV 0 2
                    test.exec(0x2002 | rot << 4);   // LSET rot 2
                    test.exec(0x8123);              // ROT 1 2 3

                    uint16_t corrRes = val << rot | val >> (16 - rot);

                    if (test.inspect(3) != corrRes) {
                        pass = false;
                        break;
                    }
                }
            } while (++valLo != 0 && pass);
        } while (++valHi != 0 && pass);

        if (pass) {
            cout << "OK!" << endl;
        }
        else {
            cout << "Fail" << endl;
        }
    }

    {
        // Testing Logic operations by testing them on all possible 8 bit values
        cout << "Testing logic operations... \t" << flush;

        bool pass = true;

        uint8_t val1 = 0;
        uint8_t val2 = 0;
        do {
            test.exec(0x0101);             // MOV 0 1
            test.exec(0x2001 | val1 << 4); // LSET val1 1
            test.exec(0x0216);             // NOT 1 6

            uint16_t corrNot = ~((uint16_t) val1);
            if (test.inspect(6) != corrNot) {
                pass = false;
                break;
            }

            do {
                test.exec(0x0102);             // MOV 0 2
                test.exec(0x2002 | val2 << 4); // LSET val2 2
                test.exec(0xa123);             // OR  1 2 3
                test.exec(0xb124);             // AND 1 2 4
                test.exec(0xc125);             // XOR 1 2 5

                uint16_t corrOr  = val1 | val2;
                uint16_t corrAnd = val1 & val2;
                uint16_t corrXor = val1 ^ val2;

                if (test.inspect(3) != corrOr  ||
                    test.inspect(4) != corrAnd ||
                    test.inspect(5) != corrXor)
                {
                    pass = false;
                    break;
                }
            } while (++val2 != 0);
        } while (++val1 != 0 && pass);

        if (pass) {
            cout << "OK!" << endl;
        }
        else {
            cout << "Fail" << endl;
        }
    }

    {
        // I'm just going to store the address in the place in memory. There is
        // no doubt better tests but this is simple
        cout << "Testing STORE/LOAD... \t\t" << flush;

        bool pass = true;

        uint8_t addrLo = 0;
        uint8_t addrHi = 0;
        do {
            do {
                test.exec(0x1001 | addrHi << 4); // HSET addrHi 1
                test.exec(0x2001 | addrLo << 4); // LSET addrLo 1
                test.exec(0x0311);               // STORE 1 1
            } while (++addrLo != 0);
        } while (++addrHi != 0);

        addrLo = 0;
        addrHi = 0;
        do {
            do {
                test.exec(0x1001 | addrHi << 4); // HSET addrHi 1
                test.exec(0x2001 | addrLo << 4); // LSET addrLo 1
                test.exec(0x0412);               // LOAD 1 2

                uint16_t addr = addrHi << 8 | addrLo;
                if (test.inspect(2) != addr) {
                    pass = false;
                    break;
                }
            } while (++addrLo != 0);
        } while (++addrHi != 0 && pass);

        if (pass) {
            cout << "OK!" << endl;
        }
        else {
            cout << "Fail" << endl;
        }
    }

    {
        // Just going to push the numbers 0x0000 to 0xffff and pop them back.
        cout << "Testing PUSH/POP... \t\t" << flush;

        bool pass = true;

        test.exec(0x010e); // MOV 0 STACK

        uint8_t countLo = 0;
        uint8_t countHi = 0;
        do {
            do {
                test.exec(0x1001 | countHi << 4); // HSET countHi 1
                test.exec(0x2001 | countLo << 4); // LSET countLo 1
                test.exec(0x051e);                // PUSH 1
            } while (++countLo != 0);
        } while (++countHi != 0);

        uint16_t count = 0xffff;
        do {
            test.exec(0x06e1); // POP 1
            if (test.inspect(1) != count) {
                pass = false;
                break;
            }
        } while (--count != 0xffff);

        if (pass) {
            cout << "OK!" << endl;
        }
        else {
            cout << "Fail" << endl;
        }
    }

    {
        // Test JMP by attempting to jump from every address with every offset
        cout << "Testing JMP+/JMP-... \t\t" << flush;

        bool pass = true;

        uint8_t addrHi = 0;
        uint8_t addrLo = 0;
        uint8_t offset = 0;
        do {
            do {
                uint16_t addr = addrHi << 8 | addrLo;
                do {
                    uint16_t corrAddr;
                    test.exec(0x100f | addrHi << 4); // HSET addrHi PC
                    test.exec(0x200f | addrLo << 4); // LSET addrLo PC
                    test.exec(0xd00f | offset << 4); // JMP+ offset

                    corrAddr = addr + offset;
                    if (test.inspect(15) != corrAddr) {
                        pass = false;
                        break;
                    }

                    test.exec(0x100f | addrHi << 4); // HSET addrHi PC
                    test.exec(0x200f | addrLo << 4); // LSET addrLo PC
                    test.exec(0xe00f | offset << 4); // JMP- offset

                    corrAddr = addr - offset;
                    if (test.inspect(15) != corrAddr) {
                        pass = false;
                        break;
                    }
                } while (++offset != 0);
            } while (++addrLo != 0 && pass);
        } while (++addrHi != 0 && pass);

        if (pass) {
            cout << "OK!" << endl;
        }
        else {
            cout << "Fail" << endl;
        }
    }

    {
        // Just make sure it works with every flag in both positions
        cout << "Testing FJMP... \t\t" << flush;

        bool pass = true;

        uint8_t addrLo = 0;
        uint8_t addrHi = 0;

        // Need to precalculate this as the NOT command will set some flags
        test.exec(0x0201); // NOT 0 1
        do {
            do {
                uint16_t addr = addrHi << 8 | addrLo;
                for (uint8_t flag = 0; flag < 16; ++flag) {
                    test.exec(0x100f | addrHi << 4); // HSET addrHi PC
                    test.exec(0x200f | addrLo << 4); // LSET addrLo PC
                    test.exec(0x010d);               // MOV 0 FLAGS
                    test.exec(0x070f | flag << 4);   // FJMP flag

                    uint16_t corrAddr = addr + 1;
                    if (test.inspect(15) != corrAddr) {
                        pass = false;
                        break;
                    }

                    test.exec(0x100f | addrHi << 4); // HSET addrHi PC
                    test.exec(0x200f | addrLo << 4); // LSET addrLo PC
                    test.exec(0x011d);               // MOV 1 FLAGS
                    test.exec(0x070f | flag << 4);   // FJMP flag

                    if (test.inspect(15) != addr) {
                        pass = false;
                        break;
                    }
                }
            } while (++addrLo != 0 && pass);
        } while (++addrHi != 0 && pass);

        if (pass) {
            cout << "OK!" << endl;
        }
        else {
            cout << "Fail" << endl;
        }
    }

    {
        // Test it running an actual program (calculating fibonacci numbers)
        cout << "Fibonacci test... \t\t" << flush;
        test.exec(0x010f); // MOV 0 PC
        test.exec(0x201f); // LSET 1 PC
        test.exec(0x010e); // MOV 0 STACK

        test.push(0x0101); // 1: MOV 0 1
        test.push(0x0102); // 2: MOV 0 2
        test.push(0x2012); // 3: LSET $1 2
        test.push(0x3121); // 4: ADD 1 2 1
        test.push(0x3122); // 5: ADD 1 2 2
        test.push(0xe03f); // 6: JMP- $3    # line 4

        // tick 3 times to do the initial data set up, every 3 ticks after that
        // will calculate the next 2 fibonacci numbers. After n loops we should
        // have reg[1] = F_{2n} and reg[2] = F={2n + 1}. The largest fibonacci
        // number that fits into 16 bits is F_24 = 46368. This will require 12
        // loops plus the initial 3 ticks = 39 ticks.

        for (int i = 0; i < 39; ++i) test.tick();

        if (test.inspect(1) == 46368) {
            cout << "OK!" << endl;
        }
        else {
            cout << "Fail" << endl;
        }
    }

    {
        // Test a slightly more complicated program (bubble sort)
        cout << "Bubble sort test... \t\t" << flush;
        test.exec(0x010f); // MOV 0 PC
        test.exec(0x201f); // LSET 1 PC
        test.exec(0x113e); // HSET 0x13 STACK
        test.exec(0x237e); // LSET 0x37 STACK
        test.exec(0x4e1a); // ADDi STACK 1 10

        // Push the data
        test.push(5);
        test.push(6);
        test.push(4);
        test.push(7);
        test.push(3);
        test.push(8);
        test.push(2);
        test.push(9);
        test.push(1);
        test.push(10);
        test.push(0);

        test.exec(0x010e); // MOV 0 STACK

        // Push the program
        test.push(0x01a1); //  1: MOV 10 1
        test.push(0x085d); //  2: FSET 5
        test.push(0x4112); //  3: ADDi 1 1 2
        test.push(0x0413); //  4: LOAD 1 3
        test.push(0x0424); //  5: LOAD 2 4
        test.push(0x3400); //  6: ADD 4 0 0
        test.push(0x070f); //  7: FJMP ZERO
        test.push(0xd08f); //  8: JMP+ 8        # line 17
        test.push(0x5340); //  9: SUB 3 4 0
        test.push(0x071f); // 10: FJMP NEG
        test.push(0xd03f); // 11: JMP+ 3        # line 15
        test.push(0x0341); // 12: STORE 4 1
        test.push(0x0332); // 13: STORE 3 2
        test.push(0x095d); // 14: FCLR 5
        test.push(0x0121); // 15: MOV 2 1
        test.push(0xe0ef); // 16: JMP- 14       # line 3
        test.push(0x075f); // 17: FJMP 5
        test.push(0xe01f); // 18: JMP- 1        # halt
        test.push(0xe13f); // 19: JMP- 19       # line 1

        // Keep going till the PC gets stuck
        uint16_t prevPC = test.inspect(15);
        while (true) {
            test.tick();
            if (test.inspect(15) == prevPC) break;
            prevPC = test.inspect(15);
        }

        bool pass = true;

        test.exec(0x01ae); // MOV 10 STACK
        test.exec(0x4e9e); // ADDi STACK 9 STACK
        for (int i = 10; i >= 1; --i) {
            test.exec(0x06e1); // POP 1
            if (test.inspect(1) != i) {
                pass = false;
                break;
            }
        }

        if (pass) {
            cout << "OK!" << endl;
        }
        else {
            cout << "Fail" << endl;
        }
    }
    return 0;
}
