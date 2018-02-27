/*
 * RegisterManager.hpp
 *
 * This is just a wrapper on a uint16_t[16] that does bounds checking and also
 * provides some other convenience functions
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

        static size_t MBZ;
        static size_t AUX;
        static size_t IHP;
        static size_t FLAGS;
        static size_t STACK;
        static size_t PC;
    private:
        uint16_t registers[16];
};

#endif
