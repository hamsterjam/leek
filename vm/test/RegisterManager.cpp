#include "RegisterManager.hpp"

#include <iostream>
#include <cstdlib>
#include <cstdint>

using namespace std;

int main(int argc, char** argv) {
    RegisterManager test;
    {
        // Set every register to every possible value
        cout << "Testing store and recall... \t" << flush;

        bool pass = true;

        for (size_t reg = 1; reg < 16; ++reg) {
            uint16_t val = 0;
            do {
                test[reg] = val;
                if (test[reg] != val) {
                    pass = false;
                    break;
                }
            } while (val++ != 0xffff && pass);
        }

        if (pass) {
            cout << "OK!" << endl;
        }
        else {
            cout << "Fail" << endl;
        }
    }

    {
        // Just try setting it to 0xffff
        cout << "Testing OBLIVION permanence... \t" << flush;

        test[0] = 0xffff;

        if (test[0] == 0) {
            cout << "OK!" << endl;
        }
        else {
            cout << "Fail" << endl;
        }
    }

    {
        // Attempt to set, unset, and toggle every bit in every possible state
        cout << "Testing bitwise operations... \t" << flush;

        bool pass = true;

        uint16_t val = 0;
        do {
            for (size_t bit = 0; bit < 16; ++bit) {
                uint16_t corrVal;

                test[1] = val;
                test.setBit(1, bit, true);
                corrVal = val | (1 << bit);
                // Test both getBit and just that we have the right value
                if (!test.getBit(1, bit) || test[1] != corrVal) {
                    pass = false;
                    break;
                }

                test[1] = val;
                test.setBit(1, bit, false);
                corrVal = val & ~(1 << bit);
                if (test.getBit(1, bit) || test[1] != corrVal) {
                    pass = false;
                    break;
                }

                test[1] = val;
                test.togBit(1, bit);
                corrVal = val ^ (1 << bit);
                if (test[1] != corrVal) {
                    pass = false;
                    break;
                }
            }
        } while (val++ != 0xffff);

        if (pass) {
            cout << "OK!" << endl;
        }
        else {
            cout << "Fail" << endl;
        }
    }
    return 0;
}
