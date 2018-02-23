#include "Processor.hpp"

#include <iostream>
#include <cstdlib>
#include <cstdint>

using namespace std;

int main(int argc, char** argv) {
    Processor test(0x10000); // 64k

    {
        // Test HSET and LSET by setting every register to every possible
        // 16 bit value and making sure it actually gets to the register
        cout << "Testing HSET and LSET... \t";

        bool pass = true;

        for (size_t reg = 1; reg <= 10 && pass; ++reg) {
            uint8_t valLo = 0;
            uint8_t valHi = 0;
            // I actually found a use for a do-while loop. These will loop
            // through every value of valLo and valHi
            do {
                do {
                    test.run(0x1000 | valHi << 4 | reg); // HSET valHi reg
                    test.run(0x2000 | valLo << 4 | reg); // LSET valLo reg

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
        // Test OBLIVION permanance by attempting to set register 0 to 0xffff
        cout << "Testing OBLIVION permanence...\t";

        test.run(0x1ff0);
        test.run(0x2ff0);

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
        cout << "Testing MOV... \t\t\t";

        bool pass = true;

        for (size_t src = 1; src <= 10 && pass; ++src) {

            uint8_t valLo = 0;
            uint8_t valHi = 0;

            do {
                do {
                    uint16_t val = valHi << 8 | valLo;
                    for (size_t dest = 1; dest <= 10; ++dest) {
                        test.run(0x1000 | valHi << 4 | src); // HSET valHi src
                        test.run(0x2000 | valLo << 4 | src); // LSET valLo src
                        test.run(0x0100 | src << 4 | dest);  // MOV src dest

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
        cout << "Testing flag operations... \t";

        bool pass = true;

        uint8_t valLo;
        uint8_t valHi;
        do {
            do {
                uint16_t val = valHi << 8 | valLo;
                for (size_t bit = 0; bit < 16; ++bit) {
                    uint16_t corrVal;

                    test.run(0x100d | valHi << 4); // HSET valHi FLAGS
                    test.run(0x200d | valLo << 4); // LSET valLo FLAGS
                    test.run(0x070d | bit << 4);   // FSET bit

                    corrVal = val | (1 << bit);
                    if (test.inspect(13) != corrVal) {
                        pass = false;
                        break;
                    }

                    test.run(0x100d | valHi << 4); // HSET valHi FLAGS
                    test.run(0x200d | valLo << 4); // LSET valLo FLAGS
                    test.run(0x080d | bit << 4);   // FCLR bit

                    corrVal = val & ~(1 << bit);
                    if (test.inspect(13) != corrVal) {
                        pass = false;
                        break;
                    }

                    test.run(0x100d | valHi << 4); // HSET valHi FLAGS
                    test.run(0x200d | valLo << 4); // LSET valLo FLAGS
                    test.run(0x090d | bit << 4);   // FTOG bit

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
        cout << "Testing ADD... \t\t\t";

        bool pass = true;

        uint8_t val1Hi = 0x00;
        uint8_t val2Hi = 0x00;

        do {
            do {
                test.run(0x080d);               // FCLR 0
                test.run(0x081d);               // FCLR 1
                test.run(0x0101);               // MOV 0 1
                test.run(0x1001 | val1Hi << 4); // HSET val1Hi 1
                test.run(0x0102);               // MOV 0 2
                test.run(0x1002 | val2Hi << 4); // HSET val2Hi 2
                test.run(0x3123);               // ADD 1 2 3

                // Make sure the value is right
                uint32_t longRes = ((uint32_t) val1Hi + val2Hi) << 8;
                uint16_t corrRes = (val1Hi + val2Hi) << 8;
                if (test.inspect(3) != corrRes) {
                    pass = false;
                    break;
                }

                // If we carry and don't trigger the carry flag
                if (longRes > 0xffff && !(test.inspect(13) & 1)) {
                    pass = false;
                    break;
                }

                // Check stuff to do with signed arithemetic
                int16_t signVal1 = (* (int16_t*) &val1Hi) << 8;
                int16_t signVal2 = (* (int16_t*) &val2Hi) << 8;

                int32_t signLongRes = signVal1 + signVal2;
                int16_t signCorrRes = signVal1 + signVal2;

                // This confusing condition just means:
                // "If they have different sign and we have no overflow flag"
                if (!(signCorrRes > 0) != !(signLongRes > 0) && !(test.inspect(13) & 2)) {
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
    return 0;
}
