#ifndef LEEK_VM_IODEVICE_H_DEFINED
#define LEEK_VM_IODEVICE_H_DEFINED

#include "Processor.hpp"

#include <cstdlib>
#include <cstdint>

class IODevice {
    public:
        IODevice(uint16_t words);

        virtual void     write(size_t address, uint16_t value);
        virtual uint16_t  read(size_t address);

        uint16_t length();

    protected:
        void ready();

    private:
        Processor* cpu;
        uint8_t line;
        uint16_t words;

        friend Processor;
};

#endif
