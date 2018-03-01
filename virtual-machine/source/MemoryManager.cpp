#include "MemoryManager.hpp"
#include "IODevice.hpp"

#include <vector>
#include <utility>
#include <stdexcept>

#include <cstdlib>
#include <cstdint>
#include <cstring>

MemoryManager::MemoryManager(size_t words) {
    this->data  = (uint16_t*) malloc(sizeof(uint16_t) * words);
    this->words = words;
}

MemoryManager::~MemoryManager() {
    free(data);
}

void MemoryManager::setRange(size_t index, uint16_t* values, size_t length) {
    if (index + length >= words) {
        throw std::out_of_range("MemoryManager::setRange");
    }

    memcpy(data + index, values, sizeof(uint16_t) * length);
}

void MemoryManager::useDevice(IODevice& dev, size_t pos) {
    // Check the deivce range doesn't overlap the memory boundaries
    if (pos + dev.length() >= words) {
        throw std::out_of_range("MemoryManager::useDevice: Memory boundary collision");
    }

    // Check it doesn't overlap any other device either
    for (auto p : devices) {
        size_t    posCheck =  p.first;
        IODevice& devCheck = *p.second;

        size_t a1, a2, b1, b2;
        a1 = pos;
        a2 = pos + dev.length();
        b1 = posCheck;
        b2 = posCheck + devCheck.length();

        if ((a1>=b1 || a2>=b1) && (a1<=b2 || a2<=b2)) {
            throw std::out_of_range("MemoryManager::useDevice: Device collision");
        }
    }

    devices.push_back(std::make_pair(pos, &dev));
}

uint16_t& MemoryManager::operator[](size_t index) {
    if (index >= words) {
        throw std::out_of_range("MemoryManager::operator[]");
    }

    uint16_t& ret = data[index];

    for (auto p : devices) {
        size_t    pos =  p.first;
        IODevice& dev = *p.second;

        if (pos <= index && index <= pos + dev.length()) {
            ret = dev.read(index - pos);
            break;
        }
    }

    return this->data[index];
}
