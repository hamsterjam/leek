/*
 * RegisterManager.hpp
 *
 * This basically only exists to bound check the array of registers
 *
 * -- Callum Nicholson
 */
#ifndef LEEK_VM_REGISTER_MANAGER_H_DEFINED
#define LEEK_VM_REGISTER_MANAGER_H_DEFINED

#include <cstdlib>
#include <cstdint>

class RegisterManager {
    public:
        uint16_t& operator[](size_t index);

        bool getBit(size_t index, size_t bit);
        void setBit(size_t index, size_t bit, bool value);
        void togBit(size_t index, size_t bit);

        uint16_t& AUX   = registers[11];
        uint16_t& IHP   = registers[12];
        uint16_t& FLAGS = registers[13];
        uint16_t& STACK = registers[14];
        uint16_t& PC    = registers[15];
    private:
        uint16_t registers[16];
};

#endif
