#include "IODevice.hpp"
#include "Processor.hpp"

#include <stdexcept>

#include <cstdlib>
#include <cstdint>

IODevice::IODevice(Processor& cpu, uint8_t line, uint16_t words) {
    if (line >= 16) {
        throw std::out_of_range("IODevice::IODevice");
    }

    this->cpu   = &cpu;
    this->line  = line;
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
