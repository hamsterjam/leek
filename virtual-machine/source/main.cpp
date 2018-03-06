#include "Processor.hpp"
#include "IODevice.hpp"
#include "devices/NumberDisplay.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <set>
#include <tuple>
#include <limits>
#include <stdexcept>

#include <cstdint>
#include <cstring>
#include <cassert>
#include <cstdio>

#include <unistd.h>

const std::streamsize maxStreamSize = std::numeric_limits<std::streamsize>::max();

int main(int argc, char** argv) {
    std::set<std::tuple<IODevice*, size_t, uint8_t>> devices;
    bool standardDevices = false;

    bool interactive = false;
    bool hexMode     = false;
    char* filename = 0;

    // Process args
    for (int i = 1; i < argc; ++i) {
        if (argv[i][0] == '-') {
            // Process flag
            switch (argv[i][1]) {
                case 'd':
                    // Add a device
                    if (!strcmp(argv[i+1], "numdisp")) {
                        IODevice* dev = new NumberDisplay();
                        size_t    pos = strtoul(argv[i+2], NULL, 16);
                        uint8_t  line = atoi(argv[i+3]);

                        devices.insert(std::make_tuple(dev, pos, line));
                    }
                    // Eat 3 words
                    i += 3;
                    break;

                case 'h':
                    // Print help text
                    {
                        std::fstream fin("help.txt");
                        while (fin.peek() != std::ifstream::traits_type::eof()) {
                            std::cout.put(fin.get());
                        }
                        return 0;
                    }
                    break;

                case 'i':
                    // Interactive mode
                    interactive = true;
                    break;

                case 's':
                    // Standard devices
                    standardDevices = true;
                    break;

                case 'x':
                    // Sets the input mode
                    hexMode = true;
                    break;

                default:
                    std::cerr << "Unknown option: " << argv[i] << std::endl;
                    return 1;
            }
        }
        else {
            // If it is not a flag, interpret it as the filename
            if (filename) {
                // If we already have a filename, this is an error.
                std::cerr << "More than one filename provided" << std::endl;
                return 1;
            }
            filename = argv[i];
        }
    }

    if (standardDevices) {
        {
            // Number Display
            IODevice* dev = new NumberDisplay();
            size_t    pos = 0xc100;
            uint8_t  line = 0;

            devices.insert(std::make_tuple(dev, pos, line));
        }
    }

    if (!filename) {
        // If we don't provide data, then the only thing that makes sense is to
        // run in interactive mode.
        interactive = true;
    }

    Processor cpu(0x10000); // 64k of memory


    for (auto t : devices) {
        cpu.useDevice(*std::get<0>(t), std::get<1>(t), std::get<2>(t));
    }

    // Initialise the state of the processor

    cpu.exec(0x010d); // MOV  r0    rFLAGS
    cpu.exec(0x010e); // MOV  r0    rSTACK
    cpu.exec(0x401f); // ADDi r0 $1 rPC

    // Push any data in the file to memory
    if (filename) {
        std::ifstream in(filename);

        while (in.peek() != std::ifstream::traits_type::eof()) {
            uint16_t instruction;
            if (hexMode) {
                in >> std::ws;
                if (in.peek() == '\n') {
                    in.get();
                    continue;
                }
                if (in.peek() == '#') {
                    in.ignore(maxStreamSize, '\n');
                    continue;
                }
                char buff[5];
                in.read(buff, 4);
                buff[4] = 0;
                instruction = std::stoul(buff, NULL, 16);
            }
            else {
                instruction = in.get() << 8 | in.get();
            }
            cpu.push(instruction);
        }
    }

    if (interactive) {
        bool done = false;
        while (!done) {

            std::cout << ">> " << std::flush;

            // stringstreams are easy to deal with when lexing, but it does
            // make this kind of verbose.
            std::string lineString;
            std::getline(std::cin, lineString);
            std::stringstream line(std::move(lineString));
            switch (line.peek()) {
                case 'e':
                    // exec
                    line.ignore(maxStreamSize, ' ');
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
                    line.ignore(maxStreamSize, ' ');
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
        cpu.run();
    }

    for (auto t : devices) {
        delete std::get<0>(t);
    }

    return 0;
}
