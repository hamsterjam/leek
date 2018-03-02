#include "devices/NumberDisplay.hpp"
#include "IODevice.hpp"

#include <iostream>

#include <cstdlib>
#include <cstdint>

NumberDisplay::NumberDisplay(): IODevice(1) {
    // Do nothing
}

void NumberDisplay::write(size_t address, uint16_t value) {
    // Bound check with super
    IODevice::write(address, 0);

    // Write to stdout
    std::cout << value << std::endl;

    ready();
}
