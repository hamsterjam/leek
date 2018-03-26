#include "Lexer.hpp"
#include "SymbolTable.hpp"
#include "helper.hpp"

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

/*
 * Recursive Lexers
 */

void Lexer::lexExpression() {
    char peek = in.peek();

    // If we have an opening paren here, it is an actual paren, not a function call
    if (peek == '(') {
        Token open;
        open.type = Token::Type::OPENING_PAREN;
        tokQueue.push(open);

        // Discard the opening paren
        in.get();

        lexWhitespace();
        lexExpression();
        lexWhitespace();

        if (in.peek() != ')') {
            // ERROR: no closing paren
            std::cerr << "Missing closing parentheses ";
            std::cerr << "at (" << in.getLine() << ", " << in.getColumn() << ")" << std::endl;
            return;
        }

        Token close;
        close.type = Token::Type::CLOSING_PAREN;
        tokQueue.push(close);

        // Discard the closing paren
        in.get();

        lexWhitespace();
    }

    // If the first character is a letter, treat it as an identifier
    else if (isLetter(peek)) {
        lexIdentifier(false);
        lexWhitespace();

        // Check that it actually was an identifier, it's possible that it was
        // a keyword instead
        Token id = tokQueue.back();
        if (id.type == Token::Type::UNARY_OPERATOR) {
            // Keyword style unary op (such as const)
            lexExpression();
        }
        else if (id.type == Token::Type::CLASS) {
            // class keyword
            //TODO// Lex classes
        }
    }

    // If the first character is a number, treat it as a number
    else if (isNumber(peek)) {
        lexNumber();
        lexWhitespace();
    }
}

/*
 * Terminal Lexers
 */

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
    if (!in.isBuffered()) in.bufferIdentifier();

    unsigned int lineNumber = in.getBufferLine();
    unsigned int colNumber  = in.getBufferColumn();

    std::string id = in.getBufferedIdentifier();
    in.clearBuffer();

    if (id == "op") {
        // Operator identifier
        //TODO//
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
