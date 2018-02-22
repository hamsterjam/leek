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
