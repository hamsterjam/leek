/*
 * RegisterManager.hpp
 *
 * This basically only exists to bound check the array of registers
 *
 * -- Callum Nicholson
 */
#ifndef KOTO_VM_REGISTER_MANAGER_H_DEFINED
#define KOTO_VM_REGISTER_MANAGER_H_DEFINED

#include <cstdlib>
#include <cstdint>

class RegisterManager {
    public:
        uint16_t& operator[](size_t index);

        bool getBit(size_t index, size_t bit);
        void setBit(size_t index, size_t bit, bool value);
        void togBit(size_t index, size_t bit);

        uint16_t& arith1 = registers[11];
        uint16_t& arith2 = registers[12];
        uint16_t& flags  = registers[13];
        uint16_t& stack  = registers[14];
        uint16_t& pc     = registers[15];
    private:
        uint16_t registers[16];
};

#endif
