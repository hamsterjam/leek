#include "RegisterManager.hpp"

#include <stdexcept>
#include <cstdlib>
#include <cstdint>

// Static members
size_t RegisterManager::MBZ   = 0;
size_t RegisterManager::AUX   = 11;
size_t RegisterManager::IHP   = 12;
size_t RegisterManager::FLAGS = 13;
size_t RegisterManager::STACK = 14;
size_t RegisterManager::PC    = 15;

uint16_t& RegisterManager::operator[](size_t index) {
    if (index >= 16) {
        throw std::out_of_range("RegisterManager::operator[]");
    }

    // Enforce that register 0 is always 0
    registers[0] = 0;
    return registers[index];
}

bool RegisterManager::getBit(size_t index, size_t bit) {
    if (bit >= 16) {
        throw std::out_of_range("RegisterManager::getBit");
    }

    uint16_t regVal = (*this)[index];
    return regVal & (1 << bit);
}

void RegisterManager::setBit(size_t index, size_t bit, bool value) {
    if (bit >= 16) {
        throw std::out_of_range("RegisterManager::setBit");
    }

    uint16_t& regVal = (*this)[index];

    if (value) {
        regVal |= 1 << bit;
    }
    else {
        regVal &= ~(1 << bit);
    }
}

void RegisterManager::togBit(size_t index, size_t bit) {
    if (bit >= 16) {
        throw std::out_of_range("RegisterManager::togBit");
    }

    uint16_t& regVal = (*this)[index];

    regVal ^= 1 << bit;
}
