#include "Instruction.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <vector>

#include <cstdint>

int main(int argc, char** argv) {
    char* inputFilename = 0;

    // Process arguments
    for (int i = 1; i < argc; ++i) {
        if (argv[i][0] == '-') {
            // Process flag
            switch (argv[i][1]) {
                case 'h':
                    // Print the help file
                    {
                        std::ifstream fin("help.txt");
                        while (fin.peek() != std::ifstream::traits_type::eof()) {
                            std::cout.put(fin.get());
                        }
                    }
                    return 0;
                default:
                    std::cerr << "Unkown option: " << argv[i] << std::endl;
                    return 1;
            }
        }
        else {
            // It is an input file
            if (inputFilename) {
                // Then we already have a filename, this is an error (for now)
                std::cerr << "More than one file provided" << std::endl;
                return 1;
            }
            inputFilename = argv[i];
        }
    }

    if (!inputFilename) {
        // No file provided, this is an error
        std::cerr << "No input file provided" << std::endl;
        return 1;
    }

    std::ifstream in(inputFilename);

    unsigned int logicalLineNumber = 0;
    unsigned int sourceLineNumber  = 1;

    std::map<std::string, unsigned int> symbolTable;
    std::vector<Instruction*> instructions(0);

    // Process line by line
    while (in.peek() != std::ifstream::traits_type::eof()) {
        std::string lineString;
        std::getline(in, lineString);
        std::stringstream line(std::move(lineString));

        Instruction* instr = 0;

        while (line.peek() != std::stringstream::traits_type::eof()) {
            line >> std::ws;

            std::string tok;
            std::string peek;
            bool isLabel = false;

            line >> tok;
            line >> std::ws;

            if (tok[0] == '#') {
                // Comment
                break;
            }

            if (tok.back() == ':') {
                isLabel = true;
                tok.pop_back();
            }

            if (line.peek() == ':') {
                isLabel = true;
                line.get();
            }

            if (isLabel) {
                // Process label
                if (instr) {
                    // Unless we are processing an instruction
                    std::cerr << "Unexpected label on line: " << sourceLineNumber << std::endl;
                    return 1;
                }
                symbolTable[tok] = logicalLineNumber;
            }
            else {
                // Process instruction
                if (!instr) {
                    // If we arent processing an instruction, start
                    instructions.push_back(new Instruction(tok));
                    instr = instructions[logicalLineNumber];

                    ++logicalLineNumber;
                }
                else {
                    // If we are processing an instruction, add an argument
                    if (instr->isSpecified()) {
                        // Unless its already fully specified
                        std::cerr << "Too many arguments provided to instruction on line: " << sourceLineNumber << std::endl;
                        return 1;
                    }
                    instr->addArgument(tok);
                }
            }
        }
        if (instr && !instr->isSpecified()) {
            // If the line had an instruction, and its not fully specified
            std::cerr << "Too few arguments provided to instruction on line: " << sourceLineNumber << std::endl;
            return 1;
        }

        ++sourceLineNumber;
    }

    return 0;
}
