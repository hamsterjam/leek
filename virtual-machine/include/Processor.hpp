/*
 * Processor.hpp
 *
 * This is where all the actuall computation happens
 *
 * -- Callum Nicholson
 */
#ifndef LEEK_VM_PROCESSOR_H_DEFINED
#define LEEK_VM_PROCESSOR_H_DEFINED

#include "MemoryManager.hpp"
#include "RegisterManager.hpp"
#include <cstdlib>
#include <cstdint>

class Processor {
    public:
        Processor(size_t memWords);

        void run(uint16_t instruction);
        void tick();
        void push(uint16_t instruction);

        uint16_t inspect(size_t index);
    private:
        MemoryManager mem;
        RegisterManager reg;
};

#endif
