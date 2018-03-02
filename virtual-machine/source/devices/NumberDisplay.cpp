#include "devices/NumberDisplay.hpp"
#include "IODevice.hpp"

#include <vector>
#include <utility>
#include <iostream>

#include <cstdlib>
#include <cstdint>

NumberDisplay::NumberDisplay(size_t bufferSize):
    IODevice(bufferSize + 1),
    buffer(bufferSize + 1)
{
    for (int i = 0; i <= bufferSize; ++i) {
        buffer[i] = std::pair<uint16_t, bool>(0, false);
    }
    length = bufferSize + 1;
}

void NumberDisplay::write(size_t address, uint16_t value) {
    // Bound check with super
    IODevice::write(address, 0);

    if (address != 0) {
        // Just write to the buffer
        buffer[address] = std::pair<uint16_t, bool>(value, true);
    }
    else {
        // Write to stdout
        for(int i = 1; i < length; ++i) {
            uint16_t value = buffer[i].first;
            bool& modified = buffer[i].second;

            if (!modified) break;
            modified = false;

            std::cout << value << std::endl;
        }

        ready();
    }
}
