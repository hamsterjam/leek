#ifndef KOTO_REGISTER_MANAGER_H_DEFINED
#define KOTO_REGISTER_MANAGER_H_DEFINED

#include <cstdlib>

class RegisterManager {
    public:
        unsigned int& operator[](size_t index);

        unsigned int& arith1 = registers[11];
        unsigned int& arith2 = registers[12];
        unsigned int& flags  = registers[13];
        unsigned int& stack  = registers[14];
        unsigned int& pc     = registers[15];
    private:
        unsigned int registers[16];
};

#endif
