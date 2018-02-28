#include "Processor.hpp"

#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>
#include <cstdint>
#include <cassert>
#include <cstdlib> // strtol

const char* helpText = R"(
NAME
        leek-vm - The Little Educational Electronic Komputer Virtual Machine

SYNOPSIS
        leek-vm [options] [file]

DESCRIPTION
        leek-vm is a virtual machine that emulates the LEEK15 architecture. It
        reads from stdin or a file, loads the given program into memory and
        runs it.

        This program is typically used to run a LEEK16 program with the command

            leek-vm file

        You can more generally start the virtual machine with

            leek-vm [options] [file]

OPTIONS
        The options may be given in any order, but the file name must be the
        final argument.

        -h              Print this help message.

        -i              Enable interactive mode.

        -m {mode}       Sets the input mode. mode is either hex or bin with the
                        default being bin. In bin mode, the virtual machine
                        will read 2 chars and interpret it as a 16 bit
                        instruction. In hex mode the virtual machine will read
                        4 chars and interpret it as a 16 bit number written in
                        hexadecimal.
)";

enum InputMode {
    BIN,
    HEX
};

int main(int argc, char** argv) {
    // Process args
    bool interactive = false;
    InputMode mode = BIN;
    char* filename = 0;
    for (int i = 1; i < argc; ++i) {
        if (argv[i][0] == '-') {
            // Process flag
            switch (argv[i][1]) {
                case 'h':
                    // Print help text
                    // +1 is there to discard opening newline
                    std::cout << helpText + 1 << std::flush;
                    return 0;
                    break;

                case 'i':
                    // Interactive mode
                    interactive = true;
                    break;

                case 'm':
                    // Sets the input mode
                    ++i;
                    if (i == argc) {
                        std::cout << "No mode provided to -m option" << std::endl;
                        return 1;
                    }
                    if (argv[i][0] == 'b') {
                        mode = BIN;
                    }
                    else if (argv[i][0] == 'h') {
                        mode = HEX;
                    }
                    else {
                        std::cout << "Unkown input mode: " << argv[i] << std::endl;
                        return 1;
                    }
                    break;

                default:
                    std::cout << "Unknown option: " << argv[i] << std::endl;
                    return 1;
            }
        }

        // If it is the last arg, it is a filename
        else if (i + 1 == argc) {
            filename = argv[i];
        }

        // Else it is an error
        else {
            std::cout << "Unknown argument: " << argv[i] << std::endl;
            return 1;
        }
    }

    std::ifstream fin(filename);
    std::istream& in = (filename) ? static_cast<std::istream&>(fin) : std::cin;

    Processor cpu(0x10000); // 64k of memory

    // Initialise the state of STACK and PC

    cpu.exec(0x010e); // MOV  0 STACK
    cpu.exec(0x010f); // MOV  0 PC
    cpu.exec(0x201f); // LSET 1 PC

    while (true) {
        uint16_t instruction;
        if      (mode == BIN) {
            instruction = in.get() << 8 | in.get();
            if (in.gcount() == 0) break;
        }
        else if (mode == HEX) {
            // Discard whitespac
            in >> std::ws;
            // Read 4 characters
            char buff[4];
            in.read(buff, 4);
            if (in.gcount() < 4) break;

            // Interperet it as a base-16 string
            instruction = strtol(buff, NULL, 16);
        }
        else {
            assert(false);
        }

        cpu.push(instruction);
    }

    // Run the processor till it halts
    if (interactive) {
        bool done = false;
        while (!done) {
            std::cout << ">> " << std::flush;
            std::string line;
            std::cin >> line;
            switch (line[0]) {
                case 'e':
                    // exec
                    {
                        std::string arg;
                        std::cin >> arg;
                        uint16_t instr = strtol(arg.c_str(), NULL, 16);
                        cpu.exec(instr);
                    }
                    break;

                case 'p':
                    // print
                    {
                        std::string arg;
                        std::cin >> arg;
                        int reg = atoi(arg.c_str());
                        try {
                            std::cout << cpu.inspect(reg) << std::endl;
                        }
                        catch (std::out_of_range e){
                            std::cout << "Invalid register" << reg << std::endl;
                        }
                    }
                    break;

                case 'q':
                    // quit
                    done = true;
                    break;

                case 'r':
                    // run
                    cpu.run();
                    break;

                case 't':
                    // tick
                    cpu.tick();
                    break;

                default:
                    std::cout << "Unknown command" << std::endl;
            }
        }
    }
    else {
        // Just run untill we halt.
        // This mode doesn't really make sense without IO implemented
        cpu.run();
    }

    return 0;
}
