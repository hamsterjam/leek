#ifndef LEEK_SLEEK_TOKEN_H_DEFINED
#define LEEK_SLEEK_TOKEN_H_DEFINED

#include <string>
#include <cstdint>

class Variable;

struct Token {
    enum class Type {
        IDENTIFIER,
        KEYWORD,
        INTEGER,
        CLASS,
        OPENING_PAREN,
        CLOSING_PAREN,
        OPENING_INDEX_BRACKET,
        CLOSING_INDEX_BRACKET,
        OPENING_ARG_LIST,
        CLOSING_ARG_LIST,
        OPENING_ARG_LIST_CT,
        CLOSING_ARG_LIST_CT,
        OPENING_PARAM_LIST,
        CLOSING_PARAM_LIST,
        OPENING_PARAM_LIST_CT,
        CLOSING_PARAM_LIST_CT,
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
