#ifndef LEEK_SLEEK_TOKEN_H_DEFINED
#define LEEK_SLEEK_TOKEN_H_DEFINED

#include <string>
#include <cstdint>

class Variable;

struct Token {
    enum class Type {
        IDENTIFIER,
        INTEGER,
        KEYWORD,
        END_OF_FILE
    };

    Type type;
    union {
        char keywordVal[8];
        Variable* varVal;
        uint16_t intVal;
    };
};

#endif
