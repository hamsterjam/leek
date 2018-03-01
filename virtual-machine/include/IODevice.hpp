#ifndef LEEK_VM_IODEVICE_H_DEFINED
#define LEEK_VM_IODEVICE_H_DEFINED

#include <cstdlib>
#include <cstdint>

class Processor;


class IODevice {
    public:
        IODevice(Processor& cpu, uint8_t line, uint16_t words);

        virtual void     write(size_t address, uint16_t value);
        virtual uint16_t  read(size_t address);

        uint16_t length();

    protected:
        void ready();

    private:
        Processor* cpu;
        uint8_t line;
        uint16_t words;
};

#endif
