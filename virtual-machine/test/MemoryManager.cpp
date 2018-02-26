#include "MemoryManager.hpp"

#include <iostream>
#include <stdexcept>
#include <cstdint>

using namespace std;

int main(int argc, char** argv) {

    size_t words = 1L << 16;
    MemoryManager testMem(words);

    {
        // I'm just going to store the numbers 0 ~ 2^16-1 in the array as a test.
        cout << "Testing store and recall... \t\t" << flush;
        for (size_t i = 0; i < words; ++i) {
            testMem[i] = i;
        }

        bool pass = true;
        for (size_t i = 0; i < words; ++i) {
            if (testMem[i] != i) {
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


    {
        // Test setRange
        cout << "Testing setRange... \t\t\t" << flush;
        uint16_t data[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 26};
        testMem.setRange(0x1337, data, 10);

        bool pass = true;
        for (size_t i = 0; i < 10; ++i) {
            if (testMem[0x1337 + i] != data[i]) {
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

    {
        // Test out of range indexing
        cout << "Testing out of range indexes... \t" << flush;

        bool pass1 = false;
        try {
            testMem[1L << 16] = 0;
        }
        catch(std::out_of_range e) {
            pass1 = true;
        }

        bool pass2 = false;
        try {
            uint16_t data[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
            testMem.setRange(0xfff9, data, 10);
        }
        catch(std::out_of_range e) {
            pass2 = true;
        }

        if (pass1 && pass2) {
            cout << "OK!" << endl;
        }
        else {
            cout << "Fail" << endl;
        }
    }
}
