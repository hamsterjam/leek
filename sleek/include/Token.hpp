#ifndef LEEK_SLEEK_TOKEN_H_DEFINED
#define LEEK_SLEEK_TOKEN_H_DEFINED

#include <string>
#include <cstdint>

class Symbol;

enum class Keyword {
    NA,

    CLASS,  DEFER,  STATIC, NEW,
    // Visibilities
    PUBLIC, PRIVATE,NOWRITE,
    // Control statements
    RETURN, IF,     ELSE,   ELIF,
    WHILE,  DO,     FOR,
    // Loop flow control
    BREAK,  PASS,
    // Types
    VOID,   INT,    UINT,   TYPE,
    FUNC,
    // Values
    TRUE,   FALSE,  NULL_REF
};

enum class UnaryOperator {
    NA,

    REFERENCE_TO,
    NEGATIVE,
    NOT,
    CONST
};

enum class BinaryOperator {
    NA,

    ADD,      SUB,      MUL,     DIV,       MOD,
    BIT_AND,  BIT_OR,   BIT_XOR, L_SHIFT,   R_SHIFT,
    BOOL_AND, BOOL_OR,  EQUAL,   NOT_EQUAL,
    COMP_LTE, COMP_GTE, COMP_LT, COMP_GT,
    ASSIGN,
    ASSIGN_ADD, ASSIGN_SUB, ASSIGN_MUL, ASSIGN_DIV, ASSIGN_MOD,
    ASSIGN_AND, ASSIGN_OR,  ASSIGN_XOR
};

struct Token {
    enum class Type {
        IDENTIFIER,
        KEYWORD,
        INTEGER,
        OPENING_BLOCK,
        CLOSING_BLOCK,
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
        PROPERTY_ACCESS,
        COMMA,
        DEFINITION,
        OVERLOAD,
        UNARY_OPERATOR,
        BINARY_OPERATOR,
        END_OF_STATEMENT,
        END_OF_FILE
    };

    Type type;
    union {
        UnaryOperator  unaryOpVal;
        BinaryOperator binaryOpVal;
        Keyword        keywordVal;
        Symbol*        symbolVal;
        uint16_t       intVal;
    };
};

#endif
