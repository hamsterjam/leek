#ifndef KOTO_PROCESSOR_H_DEFINED
#define KOTO_PROCESSOR_H_DEFINED

#include <Memory.hpp>
#include <RegisterManager.hpp>
#include <cstdlib>

class Processor {
    public:
        Memory mem;
        RegisterManager reg;
        Processor(size_t memWords);
    private:
};

#endif
