#include "MemoryManager.hpp"
#include "IODevice.hpp"

#include <set>
#include <utility>
#include <thread>
#include <stdexcept>

#include <cstdlib>
#include <cstdint>
#include <cstring> // memcpy

MemoryManager::MemoryManager(size_t words) {
    this->data  = (uint16_t*) malloc(sizeof(uint16_t) * words);
    this->words = words;
}

MemoryManager::~MemoryManager() {
    free(data);
}

uint16_t& MemoryManager::operator[](size_t index) {
    if (index >= words) {
        throw std::out_of_range("MemoryManager::operator[]");
    }

    uint16_t& ret = data[index];

    for (auto p : devices) {
        IODevice& dev = *p.first;
        size_t    pos =  p.second;

        if (pos <= index && index <= pos + dev.length()) {
            ret = dev.read(index - pos);
            break;
        }
    }

    return this->data[index];
}

void MemoryManager::setRange(size_t index, uint16_t* values, size_t length) {
    if (index + length >= words) {
        throw std::out_of_range("MemoryManager::setRange");
    }

    memcpy(data + index, values, sizeof(uint16_t) * length);
}

void MemoryManager::useDevice(IODevice& dev, size_t pos) {
    // Check the device range doesn't overlap the memory boundaries
    if (pos + dev.length() >= words) {
        throw std::out_of_range("MemoryManager::useDevice: Memory boundary collision");
    }

    // Check it doesn't overlap any other device either
    for (auto p : devices) {
        IODevice& devCheck = *p.first;
        size_t    posCheck =  p.second;

        size_t a1, a2, b1, b2;
        a1 = pos;
        a2 = pos + dev.length();
        b1 = posCheck;
        b2 = posCheck + devCheck.length();

        if ((a1>=b1 || a2>=b1) && (a1<=b2 || a2<=b2)) {
            throw std::out_of_range("MemoryManager::useDevice: Device collision");
        }
    }

    devices.insert(std::pair<IODevice*, size_t>(&dev, pos));
}

void MemoryManager::removeDevice(IODevice& dev) {
    for (auto it = devices.begin(); it != devices.end(); ++it) {
        IODevice& cand = *it->first;

        if (&cand == &dev) {
            devices.erase(it);
            break;
        }
    }
}

void callWrite(IODevice* dev, size_t pos, uint16_t val) {
    dev->write(pos, val);
}

void MemoryManager::writeIfDevice(uint16_t* val) {
    // Make sure it's in the memory chunk
    if (val < data || val > data + words) return;

    // Find if it is in the memory mapped to a device
    size_t index = val - data;
    for (auto p : devices) {
        IODevice& dev = *p.first;
        size_t    pos =  p.second;

        if (index >= pos && index <= pos + dev.length()) {
            std::thread(callWrite, &dev, index - pos, *val).detach();
            *val = 0;
            break;
        }
    }
}
