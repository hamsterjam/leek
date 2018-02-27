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
#include <atomic>

class Processor {
    public:
        Processor(size_t memWords);

        void run(uint16_t instruction);
        void tick();
        void push(uint16_t instruction);

        // This can be called asynchronously
        void interrupt(int line);

        uint16_t inspect(size_t index);
    private:
        std::atomic<bool> anyISF;
        std::atomic<bool> softISF;
        std::atomic<bool> hardISF[8];

        MemoryManager mem;
        RegisterManager reg;
};

#endif
