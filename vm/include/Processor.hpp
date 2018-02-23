/*
 * Processor.hpp
 *
 * This is where all the actuall computation happens
 *
 * -- Callum Nicholson
 */
#ifndef KOTO_VM_PROCESSOR_H_DEFINED
#define KOTO_VM_PROCESSOR_H_DEFINED

#include <MemoryManager.hpp>
#include <RegisterManager.hpp>
#include <cstdlib>

class Processor {
    public:
        Processor(size_t memWords);

        void run(unsigned int instruction);
        void tick();
    private:
        MemoryManager mem;
        RegisterManager reg;
};

#endif
