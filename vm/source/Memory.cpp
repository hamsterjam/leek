#include <Memory.hpp>

#include <cstdlib>
#include <cstring>
#include <stdexcept>

Memory::Memory(std::size_t words) {
    this->data = (unsigned int*) malloc(sizeof(unsigned int) * words);
    this->words = words;
}

Memory::~Memory() {
    free(this->data);
}

void Memory::setRange(std::size_t index, unsigned int* values, std::size_t length) {
    if (index + length >= this->words) {
        throw std::out_of_range("Memory::setRange");
    }

    memcpy(this->data + index, values, sizeof(unsigned int) * length);
}

unsigned int& Memory::operator[](std::size_t index) {
    if (index >= this->words) {
        throw std::out_of_range("Memory::operator[]");
    }

    return this->data[index];
}
