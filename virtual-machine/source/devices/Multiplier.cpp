#include "devices/Multiplier.hpp"
#include "IODevice.hpp"

#include <mutex>

#include <cstdlib>
#include <cstdint>

Multiplier::Multiplier(): IODevice(2) {
    vals[0].second = false;
    vals[1].second = false;
}

const uint32_t mask = (1 << 16) - 1;

void Multiplier::write(size_t address, uint16_t value) {
    // Bound check with super
    IODevice::write(address, value);

    std::lock_guard<std::mutex> lk(mt);
    vals[address] = std::make_pair(value, true);

    if (vals[0].second && vals[1].second) {
        uint32_t res = vals[0].first * vals[1].first;
        vals[0] = std::make_pair(res &  mask, false);
        vals[1] = std::make_pair((res & ~mask) >> 16, false);

        ready();
    }
}

uint16_t Multiplier::read(size_t address) {
    return vals[address].first;
}
