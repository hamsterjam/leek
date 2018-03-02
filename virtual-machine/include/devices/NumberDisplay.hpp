#ifndef LEEK_VM_NUMBER_DISPLAY_H_DEFINED
#define LEEK_VM_NUMBER_DISPLAY_H_DEFINED

#include "IODevice.hpp"

#include <cstdlib>
#include <cstdint>


class NumberDisplay: public IODevice {
    public:
        NumberDisplay();
        void write(size_t address, uint16_t value);
};

#endif
