#include "Type.hpp"

#include <iostream>

int ERR_TEST;
int ERR_TOTAL = 0;
#define startTest(msg) ERR_TEST = 0; std::cout << (msg) << std::flush
#define endTest() ERR_TOTAL += ERR_TEST; std::cout << (ERR_TEST ? "Fail" : "OK!") << std::endl
#define assert(x) ERR_TEST += (x) ? 0 : 1;
#define errors() ERR_TOTAL

int main(int argc, char** argv) {
    {
        startTest("Primitive Types...\t");
        Type test;

        test = Type::VOID;
        assert(test == Type::VOID);

        test = Type::INT;
        assert(test == Type::INT);

        endTest();
    }

    {
        startTest("Reference Types...\t");
        Type test1 = Type::referenceType(Type::VOID);
        Type test2 = Type::referenceType(Type::INT);

        assert(test1 != Type::VOID);
        assert(test2 != Type::INT);
        assert(test1 != test2);

        assert(test1 == test1);
        assert(test2 == test2);

        endTest();
    }

    {
        startTest("Function Types...\t");

        Type test1 = Type::functionType(true,  {Type::VOID, Type::INT});
        Type test2 = Type::functionType(false, {Type::VOID, Type::INT});
        Type test3 = Type::functionType(true, {Type::VOID});

        assert(test1 != test2);
        assert(test2 != test3);
        assert(test3 != test1);

        assert(test1 == test1);
        assert(test2 == test2);
        assert(test3 == test3);

        endTest();
    }

    return errors();
}
