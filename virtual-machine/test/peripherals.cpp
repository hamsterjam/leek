#include "Processor.hpp"
#include "devices/Incrementer.hpp"

#include <iostream>

int main(int argc, char** argv) {
    // Doing a simple read/write cycle with a simple IODevice.
    std::cout << "Testing Incrementer... \t" << std::flush;
    Processor test(0x10000); // 64k
    Incrementer inc;

    test.useDevice(inc, 0xc100, 0);

    test.exec(0x010e); // MOV  r0  rSTACK
    test.exec(0x010f); // MOV  r0  rPC
    test.exec(0x201f); // LSET rPC 1

    test.exec(0x0101); // MOV  r0 r1
    test.exec(0x2641); // LSET r1 100

    // Push the program
    test.push(0x084d); // FSET  fICF
    test.push(0x4f5c); // ADDi  rPC 5 rIHP

    test.push(0x1c12); // HSET  r2 0xc1
    test.push(0x2002); // LSET  r2 0x00
    test.push(0x0312); // STORE r1 r2

    test.push(0x0000); // NOP
    test.push(0xe02f); // JMP-  2

    test.push(0x0421); // LOAD  r2 r1
    test.push(0xe01f); // JMP-  1

    test.run();

    if (test.inspect(1) == 101) {
        std::cout << "OK!" << std::endl;
    }
    else {
        std::cout << "Fail" << std::endl;
    }
}
