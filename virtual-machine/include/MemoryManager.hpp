/*
 * MemoryManager.hpp
 *
 * For now this just allocates a "big" 64k array to store data. Note that the
 * byte size of this machine is 16 bits though, so it's 128k of physical system
 * memory. This is a class so I can make some of the memory map to peripherals
 * later on.
 *
 * -- Callum Nicholson
 */
#ifndef LEEK_VM_MEMORY_H_DEFINED
#define LEEK_VM_MEMORY_H_DEFINED

#include <cstdlib> // for size_t
#include <cstdint>

class MemoryManager {
    public:
        MemoryManager(std::size_t words);
        ~MemoryManager();

        void setRange(std::size_t index, uint16_t* values, std::size_t length);

        uint16_t& operator[](std::size_t index);
    private:
        std::size_t words;
        uint16_t* data;
};

#endif
