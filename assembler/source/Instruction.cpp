#include "Instruction.hpp"

#include <string>
#include <iostream>

#include <cstdint>

Instruction::Instruction(std::string& opCode) {
    nextFree = 3;

    // There is probably a much nicer way to do this...
    if      (opCode == "NOP") {
        setNextValue(0x0);
        setNextValue(0x0);
        setNextValue(0x0);
        setNextValue(0x0);
    }
    else if (opCode == "MOV") {
        setNextValue(0x0);
        setNextValue(0x1);
    }
    else if (opCode == "ADD") {
        setNextValue(0x1);
    }
    else if (opCode == "ADDC") {
        setNextValue(0x2);
    }
    else if (opCode == "ADDi") {
        setNextValue(0x3);
    }
    else if (opCode == "SUB") {
        setNextValue(0x4);
    }
    else if (opCode == "SUBB") {
        setNextValue(0x5);
    }
    else if (opCode == "SUBi") {
        setNextValue(0x6);
    }
    else if (opCode == "ROT") {
        setNextValue(0x7);
    }
    else if (opCode == "ROTi") {
        setNextValue(0x8);
    }
    else if (opCode == "OR") {
        setNextValue(0x9);
    }
    else if (opCode == "AND") {
        setNextValue(0xA);
    }
    else if (opCode == "XOR") {
        setNextValue(0xB);
    }
    else if (opCode == "NOT") {
        setNextValue(0x0);
        setNextValue(0x2);
    }
    else if (opCode == "STORE") {
        setNextValue(0x0);
        setNextValue(0x3);
    }
    else if (opCode == "LOAD") {
        setNextValue(0x0);
        setNextValue(0x4);
    }
    else if (opCode == "LDR+") {
        setNextValue(0xC);
        setNextValue(0x0);
        args[1].relative = true;
    }
    else if (opCode == "LDR-" || opCode == "LDR") {
        setNextValue(0xD);
        setNextValue(0x0);
        args[1].relative = true;
    }
    else if (opCode == "PUSH") {
        setNextValue(0x0);
        setNextValue(0x5);
        setValue(0, 0xE);
    }
    else if (opCode == "POP") {
        setNextValue(0x0);
        setNextValue(0x6);
        setNextValue(0xE);
    }
    else if (opCode == "JMP+") {
        setNextValue(0xE);
        setNextValue(0x0);
        setValue(0, 0xF);
        args[1].relative = true;
    }
    else if (opCode == "JMP-" || opCode == "JMP") {
        setNextValue(0xF);
        setNextValue(0x0);
        setValue(0, 0xF);
        args[1].relative = true;
    }
    else if (opCode == "FJMP") {
        setNextValue(0x0);
        setNextValue(0x7);
        setValue(0, 0xF);
    }
    else if (opCode == "FSET") {
        setNextValue(0x0);
        setNextValue(0x8);
        setValue(0, 0xD);
    }
    else if (opCode == "FCLR") {
        setNextValue(0x0);
        setNextValue(0x9);
        setValue(0, 0xD);
    }
    else if (opCode == "FTOG") {
        setNextValue(0x0);
        setNextValue(0xA);
        setValue(0, 0xD);
    }
    else if (opCode == "INTER") {
        setNextValue(0x0);
        setNextValue(0xB);
        setNextValue(0xC);
        setNextValue(0xF);
    }
    else if (opCode == "WFI") {
        setNextValue(0x0);
        setNextValue(0xC);
        setNextValue(0x0);
        setNextValue(0xF);
    }
    else if (opCode == "LIT") {
        setNextValue(0x0);
        setNextValue(0x0);
        setNextValue(0x0);
    }
}

void Instruction::addArgument(std::string& arg) {
    // If the first character is a number
    if (arg[0] >= '0' && arg[0] <= '9') {
        uint16_t val = 0;
        // If it begins with 0x interperet it as hex, otherwise as dec
        if (arg[0] == '0' && arg[1] == 'x') {
            val = std::stoul(arg.substr(2, -1), NULL, 16);
        }
        else {
            val = std::stoul(arg, NULL, 10);
        }

        setNextValue(val);
    }
    else {
        // Otherwise it is a string referencing something
        args[nextFree].reference = arg;
        while (args[--nextFree].ready);
    }
}

void Instruction::linkReferences(std::map<std::string, unsigned int>& sym, unsigned int num) {
    // Relative values are relative to the *next* instruction, not this one
    ++num;
    for (int i = 0; i < 4; ++i) {
        if (!args[i].ready) {
            if (!sym.count(args[i].reference)) {
                std::cerr << "Undefined reference to \"" << args[i].reference << "\"" << std::endl;
                return;
            }

            unsigned int refValue = sym[args[i].reference];

            if (args[i].relative) {
                // If it is a relative reference
                if (refValue > num) {
                    refValue = refValue - num;
                    // Recalling that backwards jumps are the default
                    toggleRefDir();
                }
                else {
                    refValue = num - refValue;
                }
            }

            args[i].value = refValue;
            args[i].ready = true;
        }
    }
}

bool Instruction::isReady() {
    bool ret = true;
    for (int i = 0; i < 4; ++i) {
        ret = ret && args[i].ready;
    }

    return ret;
}

bool Instruction::isSpecified() {
    return nextFree < 0;
}

uint16_t Instruction::toBin() {
    uint16_t ret = 0;
    for (int i = 0; i < 4; ++i) {
        ret |= args[i].value << (i*4);
    }
    return ret;
}

void Instruction::setValue(int pos, uint16_t value) {
    args[pos].value = value;
    args[pos].ready = true;
}

void Instruction::setNextValue(uint16_t value) {
    setValue(nextFree, value);
    while (args[--nextFree].ready);
}

void Instruction::toggleRefDir() {
    args[3].value ^= 0x1;
}
