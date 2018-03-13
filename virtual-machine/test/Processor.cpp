#include "Processor.hpp"
#include "RegisterManager.hpp"

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

            uint16_t val = 0;

            do {
                for (size_t dest = 1; dest <= 10; ++dest) {
                    test.set(src, val);
                    test.exec(0x0100 | src << 4 | dest);  // MOV src dest

                    if (test.inspect(dest) != val) {
                        pass = false;
                        break;
                    }
                }
            } while (++val != 0 && pass);
        }

        if (pass) {
            cout << "OK!" << endl;
        }
        else {
            cout << "Fail" << endl;
        }
    }

    {
        // Test JMP by attempting to jump from every address with every offset
        cout << "Testing REL+/REL-... \t\t" << flush;

        bool pass = true;

        uint16_t  addr = 0;
        uint8_t offset = 0;
        do {
            do {
                uint16_t corrAddr;
                test.set(RegisterManager::PC, addr);
                test.exec(0x100f | offset << 4); // REL+ offset rPC

                corrAddr = addr + offset;
                if (test.inspect(15) != corrAddr) {
                    pass = false;
                    break;
                }

                test.set(RegisterManager::PC, addr);
                test.exec(0x200f | offset << 4); // REL- offset rPC

                corrAddr = addr - offset;
                if (test.inspect(15) != corrAddr) {
                    pass = false;
                    break;
                }
            } while (++offset != 0);
        } while (++addr != 0 && pass);

        if (pass) {
            cout << "OK!" << endl;
        }
        else {
            cout << "Fail" << endl;
        }
    }

    {
        // Test FSET, FCLR, and FTOG by calling each on every bit of every
        // possible value
        cout << "Testing flag operations... \t" << flush;

        bool pass = true;

        uint16_t val = 0;
        do {
            for (size_t bit = 0; bit < 16; ++bit) {
                uint16_t corrVal;

                test.set(RegisterManager::FLAGS, val);
                test.exec(0x080d | bit << 4);   // FSET bit

                corrVal = val | (1 << bit);
                if (test.inspect(13) != corrVal) {
                    pass = false;
                    break;
                }

                test.set(RegisterManager::FLAGS, val);
                test.exec(0x090d | bit << 4);   // FCLR bit

                corrVal = val & ~(1 << bit);
                if (test.inspect(13) != corrVal) {
                    pass = false;
                    break;
                }

                test.set(RegisterManager::FLAGS, val);
                test.exec(0x0a0d | bit << 4);   // FTOG bit

                corrVal = val ^ (1 << bit);
                if (test.inspect(13) != corrVal) {
                    pass = false;
                    break;
                }
            }
        } while (++val != 0 && pass);

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
                test.set(1, val1Hi << 8);
                test.set(2, val2Hi << 8);
                test.exec(0x080d);               // FCLR 0
                test.exec(0x081d);               // FCLR 1
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
                test.set(1, val1Hi << 8);
                test.set(2, val2Hi << 8);
                test.exec(0x080d);               // FCLR 0
                test.exec(0x081d);               // FCLR 1
                test.exec(0x6123);               // SUB 1 2 3

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
        // Testing ROT by testing all rotations of all possible values
        cout << "Testing ROT... \t\t\t" << flush;

        bool pass = true;

        uint16_t val = 0;
        do {
            for (uint8_t rot = 0; rot < 16; ++rot) {
                test.set(1, val);
                test.set(2, rot);
                test.exec(0xb123);              // ROT 1 2 3

                uint16_t corrRes = val << rot | val >> (16 - rot);

                if (test.inspect(3) != corrRes) {
                    pass = false;
                    break;
                }
            }
        } while (++val != 0 && pass);

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
            test.set(1, val1);
            test.exec(0x0216);             // NOT 1 6

            uint16_t corrNot = ~((uint16_t) val1);
            if (test.inspect(6) != corrNot) {
                pass = false;
                break;
            }

            do {
                test.set(2, val2);
                test.exec(0xd123);             // OR  1 2 3
                test.exec(0xe124);             // AND 1 2 4
                test.exec(0xf125);             // XOR 1 2 5

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

        uint16_t addr = 0;
        do {
            test.set(1, addr);
            test.exec(0x0311);               // STORE 1 1
        } while (++addr != 0);

        addr = 0;
        do {
            test.set(1, addr);
            test.exec(0x0412);               // LOAD 1 2

            if (test.inspect(2) != addr) {
                pass = false;
                break;
            }
        } while (++addr != 0);

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

        uint16_t count = 0;
        do {
                test.set(1, count);
                test.exec(0x051e);                // PUSH 1
        } while (++count != 0);

        count = 0xffff;
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
        // Just make sure it works with every flag in both positions
        cout << "Testing FJMP... \t\t" << flush;

        bool pass = true;

        uint16_t addr = 0;

        // Need to precalculate this as the NOT command will set some flags
        test.exec(0x0201); // NOT 0 1
        do {
            for (uint8_t flag = 0; flag < 16; ++flag) {
                test.set(RegisterManager::PC, addr);
                test.exec(0x010d);               // MOV 0 FLAGS
                test.exec(0x070f | flag << 4);   // FJMP flag

                uint16_t corrAddr = addr + 1;
                if (test.inspect(15) != corrAddr) {
                    pass = false;
                    break;
                }

                test.set(RegisterManager::PC, addr);
                test.exec(0x011d);               // MOV 1 FLAGS
                test.exec(0x070f | flag << 4);   // FJMP flag

                if (test.inspect(15) != addr) {
                    pass = false;
                    break;
                }
            }
        } while (++addr != 0 && pass);

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
        test.set(RegisterManager::PC, 1);
        test.set(RegisterManager::STACK, 0);

        test.push(0x0101); // 1: MOV 0 1
        test.push(0x0102); // 2: MOV 0 2
        test.push(0x5012); // 3: ADDi 0 1 2
        test.push(0x3121); // 4: ADD 1 2 1
        test.push(0x3122); // 5: ADD 1 2 2
        test.push(0x203f); // 6: REL- $3 rPC    # line 4

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
        test.set(RegisterManager::PC, 1);
        test.set(RegisterManager::STACK, 0x1337); // This is just a random address
        test.exec(0x5e1a); // ADDi STACK 1 10

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
        test.push(0x5112); //  3: ADDi 1 1 2
        test.push(0x0413); //  4: LOAD 1 3
        test.push(0x0424); //  5: LOAD 2 4
        test.push(0x3400); //  6: ADD 4 0 0
        test.push(0x070f); //  7: FJMP ZERO
        test.push(0x108f); //  8: REL+ 8 rPC    # line 17
        test.push(0x6340); //  9: SUB 3 4 0
        test.push(0x071f); // 10: FJMP NEG
        test.push(0x103f); // 11: REL+ 3 rPC    # line 15
        test.push(0x0341); // 12: STORE 4 1
        test.push(0x0332); // 13: STORE 3 2
        test.push(0x095d); // 14: FCLR 5
        test.push(0x0121); // 15: MOV 2 1
        test.push(0x20ef); // 16: REL- 14 rPC   # line 3
        test.push(0x075f); // 17: FJMP 5
        test.push(0x201f); // 18: REL- 1 rPC    # halt
        test.push(0x213f); // 19: REL- 19 rPC   # line 1

        // Keep going till the PC gets stuck
        uint16_t prevPC = test.inspect(15);
        while (true) {
            test.tick();
            if (test.inspect(15) == prevPC) break;
            prevPC = test.inspect(15);
        }

        bool pass = true;

        test.exec(0x01ae); // MOV 10 STACK
        test.exec(0x5e9e); // ADDi STACK 9 STACK
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
