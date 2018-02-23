#include <RegisterManager.hpp>

#include <stdexcept>
#include <cstdlib>

unsigned int& RegisterManager::operator[](size_t index) {
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

    unsigned int regVal = (*this)[index];
    return regVal & (1 << bit);
}

void RegisterManager::setBit(size_t index, size_t bit, bool value) {
    if (bit >= 16) {
        throw std::out_of_range("RegisterManager::setBit");
    }

    unsigned int& regVal = (*this)[index];

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

    unsigned int& regVal = (*this)[index];

    regVal ^= 1 << bit;
}
