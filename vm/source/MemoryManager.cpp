#include "MemoryManager.hpp"

#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <stdexcept>

MemoryManager::MemoryManager(std::size_t words) {
    this->data = (uint16_t*) malloc(sizeof(uint16_t) * words);
    this->words = words;
}

MemoryManager::~MemoryManager() {
    free(this->data);
}

void MemoryManager::setRange(std::size_t index, uint16_t* values, std::size_t length) {
    if (index + length >= this->words) {
        throw std::out_of_range("Memory::setRange");
    }

    memcpy(this->data + index, values, sizeof(uint16_t) * length);
}

uint16_t& MemoryManager::operator[](std::size_t index) {
    if (index >= this->words) {
        throw std::out_of_range("Memory::operator[]");
    }

    return this->data[index];
}
