#ifndef LEEK_SLEEK_TOKEN_H_DEFINED
#define LEEK_SLEEK_TOKEN_H_DEFINED

#include <string>
#include <cstdint>

class Variable;

struct Token {
    enum class Type {
        IDENTIFIER,
        INTEGER,
        CLASS,
        OPENING_PAREN,
        CLOSING_PAREN,
        UNARY_OPERATOR,
        BINARY_OPERATOR,
        END_OF_FILE
    };

    Type type;
    union {
        char stringVal[8];
        Variable* varVal;
        uint16_t intVal;
    };
};

#endif
