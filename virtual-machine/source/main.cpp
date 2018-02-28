#include "Processor.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <limits>
#include <stdexcept>

#include <cstdint>
#include <cassert>
#include <cstdio>

#include <unistd.h>

const char* helpText = R"(
NAME
        leek-vm - The Little Educational Electronic Komputer Virtual Machine

SYNOPSIS
        leek-vm [options] [file]

DESCRIPTION
        leek-vm is a virtual machine that emulates the LEEK16 architecture. It
        reads data from file, loads the given program into memory and runs it.

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
    bool interactive = false;
    InputMode mode = BIN;
    char* filename = 0;

    // Process args
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

    if (!filename) {
        // If we don't provide data, then the only thing that makes sense is to
        // run in interactive mode.
        interactive = true;
    }

    Processor cpu(0x10000); // 64k of memory

    // Initialise the state of STACK and PC

    cpu.exec(0x010e); // MOV  0 STACK
    cpu.exec(0x010f); // MOV  0 PC
    cpu.exec(0x201f); // LSET 1 PC

    // Push any data (either as a file or a pipe) to memory
    if (filename) {
        std::ifstream in(filename);

        while (in.peek() != std::istream::traits_type::eof()) {
            uint16_t instruction;
            switch (mode) {
                case BIN:
                    instruction = in.get() << 8 | in.get();
                    break;

                case HEX:
                    in >> std::ws;
                    char buff[4];
                    in.read(buff, 4);
                    instruction = std::stoul(buff, NULL, 16);
                    break;

                default:
                    assert(false);
            }
            cpu.push(instruction);
        }
    }

    if (interactive) {
        bool done = false;
        while (!done) {
            const std::streamsize maxSize = std::numeric_limits<std::streamsize>::max();

            std::cout << ">> " << std::flush;

            // stringstreams are easy to deal with when lexing, but it does
            // make this kind of verbose.
            std::string lineString;
            std::getline(std::cin, lineString);
            std::stringstream line(std::move(lineString));
            switch (line.peek()) {
                case 'e':
                    // exec
                    line.ignore(maxSize, ' ');
                    if (line.eof()) {
                        std::cout << "No argument" << std::endl;
                    }
                    else {
                        uint16_t instr;
                        line >> std::hex >> instr;
                        cpu.exec(instr);
                    }
                    break;

                case 'p':
                    // print
                    line.ignore(maxSize, ' ');
                    if (line.eof()) {
                        std::cout << "No argument" << std::endl;
                    }
                    else {
                        int reg;
                        line >> std::dec >> reg;
                        try {
                            std::cout << cpu.inspect(reg) << std::endl;
                        }
                        catch (std::out_of_range e) {
                            std::cout << "Invalid register" << std::endl;
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
