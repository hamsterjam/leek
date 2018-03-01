#include "devices/Incrementer.hpp"
#include "IODevice.hpp"

#include <cstdlib>
#include <cstdint>

class Processor;

Incrementer::Incrementer(Processor& cpu, uint8_t line):
    IODevice(cpu, line, 1)
{
    // Do nothing
}

void Incrementer::write(size_t address, uint16_t value) {
    // Call super to validate address
    IODevice::write(address, value);
    this->value = value + 1;

    ready();
}

uint16_t Incrementer::read(size_t address) {
    // Call super to validate address
    IODevice::read(address);
    return value;
}
