#ifndef LEEK_VM_DEVICES_INCREMENTER_H_DEFINED
#define LEEK_VM_DEVICES_INCREMENTER_H_DEFINED

#include "IODevice.hpp"

#include <cstdlib>
#include <cstdint>

class Processor;

class Incrementer: public IODevice {
    public:
        Incrementer();

        void     write(size_t address, uint16_t value);
        uint16_t  read(size_t address);

    private:
        uint16_t value;
};

#endif
