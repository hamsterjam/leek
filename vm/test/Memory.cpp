#include <Memory.hpp>
#include <iostream>

using namespace std;

int main(int argc, char** argv) {

    size_t words = 1L << 16;
    Memory testMem(words);

    {
        // I'm just going to store the numbers 0 ~ 2^16-1 in the array as a test.
        cout << "Testing store and recall... \t";
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
        cout << "Testing setRange... \t\t";
        unsigned int data[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 26};
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
}
