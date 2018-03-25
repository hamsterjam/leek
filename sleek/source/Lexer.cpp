#include "Lexer.hpp"
#include "SymbolTable.hpp"

#include <queue>
#include <stdexcept>
#include <sstream>
#include <string>
#include <iostream>

#include <cstring>

Lexer::Lexer(const char* filename, SymbolTable& sym) : in(filename) {
    this->sym = &sym;
}

Token Lexer::peek() {
    return tokQueue.front();
}

Token Lexer::get() {
    Token ret = tokQueue.front();
    tokQueue.pop();
    return ret;
}

void Lexer::operator>>(Token& out) {
    out = get();
}

bool isNumber(char test) {
    return test >= '0' && test <= '9';
}

bool isHexNumber(char test) {
    return isNumber(test) || (test >= 'a' && test <= 'f') || (test >= 'A' && test <= 'F');
}

bool isLetter(char test) {
    return (test >= 'a' && test <= 'z') || (test >= 'A' && test <= 'Z');
}

void Lexer::lexWhitespace() {
    // Just discard it, whitespace has no meaning in sleek
    in.eatWhitespace();
}

void Lexer::lexUnaryOperator() {
    unsigned int lineNumber = in.getLine();
    unsigned int colNumber  = in.getColumn();

    Token ret;
    ret.type = Token::Type::UNARY_OPERATOR;
    char peek = in.peek();
    switch (peek) {
        case '&':
        case '-':
        case '!':
            ret.stringVal[0] = peek;
            ret.stringVal[1] = 0;
            break;
        default:
            std::cerr << "Invalid unary operator ";
            std::cerr << "at (" << lineNumber << ", " << colNumber << ")" << std::endl;
            return;
    }

    tokQueue.push(ret);
}

void Lexer::lexBinaryOperator() {
    unsigned int lineNumber = in.getLine();
    unsigned int colNumber  = in.getColumn();

    Token ret;
    ret.type = Token::Type::BINARY_OPERATOR;

    std::string id = "";

    char first = in.peek();
    switch (first) {
        case '+':
        case '-':
        case '*':
        case '/': // May be succeded by an '='
        case '%':
        case '^':
        case '=':
            id += in.get();
            if (in.peek() == '=') {
                id += in.get();
            }
            break;
        case '&':
        case '|': // May be succeded by itself or an '='
        case '<':
        case '>':
            id += in.get();
            if (in.peek() == '=') {
                id += in.get();
            }
            else if (in.peek() == first) {
                id += in.get();
            }
            break;
        case '!': // Must be succesded by an '='
            id += in.get();
            if (in.peek() == '=') {
                id += in.get();
                break;
            }
            // Else fall through to default (error)
        default:
            std::cerr << "Invalid Binary Operator ";
            std::cerr << "at (" << lineNumber << ", " << colNumber << ")" << std::endl;
            return;
    }

    int i = 0;
    for (; i < id.length(); ++i) {
        ret.stringVal[i] = id[i];
    }
    ret.stringVal[i] = 0;

    tokQueue.push(ret);
}

void Lexer::lexIdentifier(bool definition) {
    unsigned int lineNumber = in.getLine();
    unsigned int colNumber  = in.getColumn();

    if (!isLetter(in.peek())) {
        // ERROR: Not a valid identifier
        std::cerr << "Invaild identifier ";
        std::cerr << "at (" << lineNumber << ", " << colNumber << ")" << std::endl;
        return;
    }

    std::stringstream buff;
    char peek = in.peek();
    while (isLetter(peek) || isNumber(peek) || peek == '_' || peek == '-') {
        buff << (char) in.get();
        peek = in.peek();
    }

    std::string id;
    buff >> id;

    if (id == "op") {
        // Operator identifier
    }

    Token ret;

    // Handle keywords
    if (id == "class") {
        ret.type = Token::Type::CLASS;
        tokQueue.push(ret);
        return;
    }
    if (id == "const") {
        ret.type = Token::Type::UNARY_OPERATOR;
        strncpy(ret.stringVal, "const", 8);
        tokQueue.push(ret);
        return;
    }


    ret.type = Token::Type::IDENTIFIER;

    if (definition) {
        try {
            ret.varVal = &sym->define(id);
        }
        catch (std::out_of_range e) {
            // ERROR: Variable already exists
            std::cerr << "Variable \"" << id << "\" redefined ";
            std::cerr << "at (" << lineNumber << ", " << colNumber << ")" << std::endl;
            return;
        }
    }
    else {
        ret.varVal = &sym->get(id);
    }

    tokQueue.push(ret);
}

void Lexer::lexNumber() {
    unsigned int lineNumber = in.getLine();
    unsigned int colNumber  = in.getColumn();

    if (!isNumber(in.peek())) {
        // ERROR: invalid number
        std::cerr << "Invalid number ";
        std::cerr << "at (" << lineNumber << ", " << colNumber << ")" << std::endl;
        return;
    }

    int base = 10;
    if (in.peek() == '0') {
        // Check the next symbol for the base

        // Discard the 0
        in.get();

        // If the next char is a number, its not a base specifier
        if (!isNumber(in.peek())) {
            char specifier = in.get();
            switch (specifier) {
                case 'x':
                case 'X':
                    base = 16;
                    break;
                case 'o':
                case 'O':
                    base = 8;
                    break;
                case 'b':
                case 'B':
                    base = 2;
                    break;
                default:
                    std::cerr << "Invalid number: unrecognised base specifier \"" << specifier << "\" ";
                    std::cerr << "at (" << lineNumber << ", " << colNumber << ")" << std::endl;
                    return;
            }
        }
    }

    std::stringstream buff;
    while (isHexNumber(in.peek())) {
        buff << (char) in.get();
    }

    std::string numString;
    buff >> numString;

    unsigned long retVal = std::stoul(numString, 0, base);

    if (retVal > 0xffff) {
        std::cerr << "Number is bigger than 16 bits ";
        std::cerr << "at (" << lineNumber << ", " << colNumber << ")" << std::endl;
        return;
    }

    Token ret;
    ret.type = Token::Type::INTEGER;
    ret.intVal = retVal;

    tokQueue.push(ret);
}
