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

#include <set>
#include <utility>

#include <cstdlib>
#include <cstdint>

class IODevice;

class MemoryManager {
    public:
        MemoryManager(size_t words);
        ~MemoryManager();

        uint16_t& operator[](size_t index);
        void setRange(size_t index, uint16_t* values, size_t length);

        void useDevice(IODevice& dev, size_t pos);
        void removeDevice(IODevice& dev);
        void writeIfDevice(uint16_t* data);

    private:
        size_t    words;
        uint16_t* data;

        std::set<std::pair<IODevice*, size_t>> devices;
};

#endif
