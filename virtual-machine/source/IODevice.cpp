#include "IODevice.hpp"
#include "Processor.hpp"

#include <stdexcept>

#include <cstdlib>
#include <cstdint>

IODevice::IODevice(uint16_t words) {
    this->words = words;
}

void IODevice::write(size_t address, uint16_t value) {
    // Default behaviour is to do nothing (and not interrupt)
    if (address >= words) {
        throw std::out_of_range("IODevice::write");
    }
}

uint16_t IODevice::read(size_t address) {
    // Default behaviour is to do nothing (and not interrupt)
    if (address >= words) {
        throw std::out_of_range("IODevice::read");
    }
    return 0;
}

uint16_t IODevice::length() {
    return words;
}

void IODevice::ready() {
    cpu->interrupt(line);
}
