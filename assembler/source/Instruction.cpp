#include "Instruction.hpp"

#include <string>
#include <iostream>

#include <cstdint>

Instruction::Instruction(std::string& opCode, unsigned int sourceLineNumber) {
    this->sourceLineNumber = sourceLineNumber;
    nextFree = 3;

    // At least clear the bools
    for (int i = 0; i < 4; ++i) {
        args[i].ready    = false;
        args[i].relative = false;
        args[i].reserved = false;
    }

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
    else if (opCode == "REL+") {
        setNextValue(0x1);
        reserveNext();
        args[1].relative = true;
    }
    else if (opCode == "REL-" || opCode == "REL") {
        setNextValue(0x2);
        reserveNext();
        args[1].relative = true;
    }
    else if (opCode == "JMP+") {
        setNextValue(0x1);
        reserveNext();
        setValue(0, 0xF);
        args[1].relative = true;
    }
    else if (opCode == "JMP-" || opCode == "JMP") {
        setNextValue(0x2);
        reserveNext();
        setValue(0, 0xF);
        args[1].relative = true;
    }
    else if (opCode == "ADD") {
        setNextValue(0x3);
    }
    else if (opCode == "ADDC") {
        setNextValue(0x4);
    }
    else if (opCode == "ADDi") {
        setNextValue(0x5);
    }
    else if (opCode == "SUB") {
        setNextValue(0x6);
    }
    else if (opCode == "SUBB") {
        setNextValue(0x7);
    }
    else if (opCode == "SUBi") {
        setNextValue(0x8);
    }
    else if (opCode == "MUL") {
        setNextValue(0x9);
    }
    else if (opCode == "DIV") {
        setNextValue(0xA);
    }
    else if (opCode == "ROT") {
        setNextValue(0xB);
    }
    else if (opCode == "ROTi") {
        setNextValue(0xC);
    }
    else if (opCode == "OR") {
        setNextValue(0xD);
    }
    else if (opCode == "AND") {
        setNextValue(0xE);
    }
    else if (opCode == "XOR") {
        setNextValue(0xF);
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
    else if (opCode == "HALT") {
        setNextValue(0x2);
        setNextValue(0x0);
        setNextValue(0x1);
        setNextValue(0xF);
    }
    else if (opCode == "LIT") {
        reserveNext();
        reserveNext();
        reserveNext();
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
                std::cerr << "Undefined reference to \"" << args[i].reference
                    << "\" (line " << sourceLineNumber << ")" << std::endl;
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

            setValue(i, refValue);
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
    // Make sure the value isn't too big
    if ((pos >= 3 || !args[pos + 1].reserved) && value > 0xF) {
        // Max of 4 bits, got something longer
        std::cerr << "Argument longer than expected 4 bits."
            << "(line " << sourceLineNumber
            << ", arg " << 4 - pos - 1
            << ")" << std::endl;
        return;
    }
    if ((pos >= 2 || !args[pos + 2].reserved) && value > 0xFF) {
        // Max of 8 bits, got something longer
        std::cerr << "Argument longer than expected 8 bits."
            << "(line " << sourceLineNumber
            << ", arg " << 4 - pos - 2
            << ")" << std::endl;
        return;
    }
    if ((pos >= 1 || !args[pos + 3].reserved) && value > 0xFFF) {
        // Max of 12 bits, got something longer
        std::cerr << "Argument longer than expected 12 bits."
            << "(line " << sourceLineNumber
            << ", arg " << 4 - pos - 2
            << ")" << std::endl;
        return;
    }

    args[pos].value = value;
    args[pos].ready = true;
}

void Instruction::setNextValue(uint16_t value) {
    setValue(nextFree, value);

    if (!args[nextFree].ready) return;
    while (args[--nextFree].ready);
}

void Instruction::reserveNext() {
    args[nextFree].reserved = true;
    args[nextFree].ready    = true;
    while (args[--nextFree].ready);
}

void Instruction::toggleRefDir() {
    args[3].value ^= 0x3;
}
