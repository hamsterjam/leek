/*
 * Memory.hpp
 *
 * For now this just allocates a "big" 64k array to store data. Note that the
 * byte size of this machine is 16 bits though, so it's 128k of physical system
 * memory. This is a class so I can make some of the memory map to peripherals
 * later on.
 *
 * -- Callum Nicholson
 */
#ifndef KOTO_MEMORY_H_DEFINED
#define KOTO_MEMORY_H_DEFINED

#include <cstdlib> // for size_t

class Memory {
    public:
        Memory(std::size_t words);
        ~Memory();

        void setRange(std::size_t index, unsigned int* values, std::size_t length);

        unsigned int& operator[](std::size_t index);
    private:
        std::size_t words;
        unsigned int* data;
};

#endif
