#include "devices/Incrementer.hpp"
#include "IODevice.hpp"

#include <cstdlib>
#include <cstdint>

#include <unistd.h>

Incrementer::Incrementer(): IODevice(1) {
    // Do nothing
}

void Incrementer::write(size_t address, uint16_t value) {
    // Call super to validate address
    IODevice::write(address, value);
    this->value = value + 1;

    // Wait a second, this is a slow peripheral...
    sleep(1);

    ready();
}

uint16_t Incrementer::read(size_t address) {
    // Call super to validate address
    IODevice::read(address);
    return value;
}
