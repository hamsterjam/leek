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

class RegisterManager {
    public:
        unsigned int& operator[](size_t index);

        bool getBit(size_t index, size_t bit);
        void setBit(size_t index, size_t bit, bool value);

        unsigned int& arith1 = registers[11];
        unsigned int& arith2 = registers[12];
        unsigned int& flags  = registers[13];
        unsigned int& stack  = registers[14];
        unsigned int& pc     = registers[15];
    private:
        unsigned int registers[16];
};

#endif
