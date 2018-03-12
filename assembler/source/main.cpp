#include "Instruction.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <vector>

#include <cstdint>

void addStandardReferences(std::map<std::string, unsigned int>& symbolTable) {
    // Numeric register references
    symbolTable["r0"]  = 0;
    symbolTable["r1"]  = 1;
    symbolTable["r2"]  = 2;
    symbolTable["r3"]  = 3;
    symbolTable["r4"]  = 4;
    symbolTable["r5"]  = 5;
    symbolTable["r6"]  = 6;
    symbolTable["r7"]  = 7;
    symbolTable["r8"]  = 8;
    symbolTable["r9"]  = 9;
    symbolTable["r10"] = 10;
    symbolTable["r11"] = 11;
    symbolTable["r12"] = 12;
    symbolTable["r13"] = 13;
    symbolTable["r14"] = 14;
    symbolTable["r15"] = 15;

    // Symbolic register references
    symbolTable["rMBZ"]   = 0;
    symbolTable["rAUX1"]  = 10;
    symbolTable["rAUX2"]  = 11;
    symbolTable["rIHP"]   = 12;
    symbolTable["rFLAGS"] = 13;
    symbolTable["rSTACK"] = 14;
    symbolTable["rPC"]    = 15;

    // Symbolic flag references
    symbolTable["fZERO"]  = 0;
    symbolTable["fNEG"]   = 1;
    symbolTable["fCARRY"] = 2;
    symbolTable["fOVER"]  = 3;
    symbolTable["fICF"]   = 4;
    symbolTable["fISFs"]  = 7;
    symbolTable["fISF0"]  = 8;
    symbolTable["fISF1"]  = 9;
    symbolTable["fISF2"]  = 10;
    symbolTable["fISF3"]  = 11;
    symbolTable["fISF4"]  = 12;
    symbolTable["fISF5"]  = 13;
    symbolTable["fISF6"]  = 14;
    symbolTable["fISF7"]  = 15;
}

int main(int argc, char** argv) {

    /* * * * * * * * * * *
     * Process arguments *
     * * * * * * * * * * */

    char* inputFilename  = 0;
    char* outputFilename = 0;

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

                case 'o':
                    // Output file
                    if (outputFilename) {
                        std::cerr << "More than one output file provided." << std::endl;
                        return 1;
                    }
                    outputFilename = argv[i+1];
                    ++i;
                    break;

                default:
                    std::cerr << "Unkown option: " << argv[i] << std::endl;
                    return 1;
            }
        }
        else {
            // It is an input file
            if (inputFilename) {
                // Then we already have a filename, this is an error (for now)
                std::cerr << "More than one input file provided." << std::endl;
                return 1;
            }
            inputFilename = argv[i];
        }
    }

    if (!inputFilename) {
        // No file provided, this is an error
        std::cerr << "No input file provided." << std::endl;
        return 1;
    }

    if (!outputFilename) {
        // No output provided, this is an error
        std::cerr << "No output file provided." << std::endl;
        return 1;
    }

    std::ifstream in(inputFilename);

    /* * * * *
     * Parse *
     * * * * */

    unsigned int logicalLineNumber = 0;
    unsigned int sourceLineNumber  = 1;

    std::map<std::string, unsigned int> symbolTable;
    std::vector<Instruction*> instructions(0);

    addStandardReferences(symbolTable);

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
                    instructions.push_back(new Instruction(tok, sourceLineNumber));
                    instr = instructions[logicalLineNumber];

                    ++logicalLineNumber;
                }
                else {
                    // If we are processing an instruction, add an argument
                    if (instr->isSpecified()) {
                        // Unless its already fully specified
                        std::cerr << "Too many arguments provided to instruction. (line " << sourceLineNumber << ")" << std::endl;
                        return 1;
                    }
                    instr->addArgument(tok);
                }
            }
        }
        if (instr && !instr->isSpecified()) {
            // If the line had an instruction, and its not fully specified
            std::cerr << "Too few arguments provided to instruction. (line " << sourceLineNumber << ")" << std::endl;
            return 1;
        }

        ++sourceLineNumber;
    }

    /* * * * * * * * * *
     * Link References *
     * * * * * * * * * */

    for (int i = 0; i < instructions.size(); ++i) {
        instructions[i]->linkReferences(symbolTable, i);
        if (!instructions[i]->isReady()) {
            // Error message provided by Instruction::linkReferences
            return 1;
        }
    }

    /* * * * * *
     * Output  *
     * * * * * */

    std::ofstream out(outputFilename);

    for (int i = 0; i < instructions.size(); ++i) {
        uint16_t instr = instructions[i]->toBin();
        uint8_t instrHi = instr >> 8;
        uint8_t instrLo = instr & ((1 << 8) - 1);

        out.put(instrHi);
        out.put(instrLo);
    }

    return 0;
}
