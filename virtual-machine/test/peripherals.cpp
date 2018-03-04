#include "Processor.hpp"
#include "devices/Incrementer.hpp"
#include "devices/Multiplier.hpp"

#include <iostream>

int main(int argc, char** argv) {
    {
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
        test.push(0x4f4c); // ADDi  rPC 4 rIHP
        test.push(0x1c12); // HSET  r2 0xc1
        test.push(0x2002); // LSET  r2 0x00
        test.push(0x0312); // STORE r1 r2
        test.push(0x0C0F); // WFI
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

    {
        // The multiplier is a slightly more complicated i/o device
        std::cout << "Testing Multiplier... \t" << std::flush;
        Processor test(0x10000);
        Multiplier mul;

        test.useDevice(mul, 0xc100, 0);

        test.exec(0x010d); // MOV   r0    rFLAGS
        test.exec(0x010e); // MOV   r0    rSTACK
        test.exec(0x401f); // ADDi  r0 $1 rPC

        test.push(0x1c19); // HSET  0xc1  r9
        test.push(0x2009); // LSET  0x00  r9
        test.push(0x491a); // ADDi  r9 $1 r10
        test.push(0x4051); // ADDi  r0 $5 r1
        test.push(0x4072); // ADDi  r0 $7 r2
        test.push(0x0319); // STORE r1    r9
        test.push(0x032a); // STORE r2    r10
        test.push(0x0c0f); // WFI
        test.push(0x0491); // LOAD  r9    r1
        test.push(0x04a2); // LOAD  r10   r2
        test.push(0xe01f); // JMP-  $1

        test.run();

        if (test.inspect(1) == 35 && test.inspect(2) == 0) {
            std::cout << "OK!" << std::endl;
        }
        else {
            std::cout << "Fail" << std::endl;
        }
    }

    return 0;
}
