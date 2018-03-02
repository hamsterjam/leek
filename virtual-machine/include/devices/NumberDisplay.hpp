#ifndef LEEK_VM_NUMBER_DISPLAY_H_DEFINED
#define LEEK_VM_NUMBER_DISPLAY_H_DEFINED

#include "IODevice.hpp"
#include <vector>
#include <utility>
#include <cstdlib>
#include <cstdint>

class NumberDisplay: public IODevice {
    public:
        NumberDisplay(size_t bufferSize);
        void write(size_t address, uint16_t value);

    private:
        std::vector<std::pair<uint16_t, bool>> buffer;
}

#endif
