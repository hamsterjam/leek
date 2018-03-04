#ifndef LEEK_VM_MULTIPLIER_H_DEFINED
#define LEEK_VM_MULTIPLIER_H_DEFINED

#include "IODevice.hpp"

#include <mutex>
#include <utility>

#include <cstdlib>
#include <cstdint>

class Multiplier: public IODevice {
    public:
        Multiplier();

        void write(size_t address, uint16_t val);
        uint16_t read(size_t address);

    private:
        std::mutex mt;

        std::pair<uint16_t, bool> vals[2];
};

#endif
