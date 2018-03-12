#ifndef LEEK_ASSEMBLER_INSTRUCTION_H_DEFINED
#define LEEK_ASSEMBLER_INSTRUCTION_H_DEFINED

#include <string>
#include <map>
#include <cstdint>

class Instruction {
    public:
        Instruction(std::string& opCode, unsigned int sourceLineNumber);

        void addArgument(std::string& arg);
        void linkReferences(std::map<std::string, unsigned int>& symbolTable, unsigned int logicalLineNumber);
        bool isReady();
        bool isSpecified();
        uint16_t toBin();

    private:
        struct Arg {
            uint16_t     value;
            std::string reference;

            bool ready;
            bool relative;
            bool reserved;
        };

        void setValue(int pos, uint16_t value);
        void setNextValue(uint16_t value);
        void reserveNext();

        void toggleRefDir();

        Arg args[4]; // little-endian
        unsigned int sourceLineNumber;
        int nextFree;
};

#endif
