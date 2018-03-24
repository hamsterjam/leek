#include "Lexer.hpp"
#include "SymbolTable.hpp"

#include <queue>
#include <stdexcept>
#include <sstream>
#include <string>
#include <iostream>

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

bool isLetter(char test) {
    return (test >= 'a' && test <= 'z') || (test >= 'A' && test <= 'Z');
}

void Lexer::lexIdentifier(bool definition) {
    if (!isLetter(in.peek())) {
        // ERROR: Not a valid identifier
        std::cerr << "Invaild identifier" << std::endl;
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
    ret.type = Token::Type::IDENTIFIER;

    if (definition) {
        try {
            ret.varVal = &sym->define(id);
        }
        catch (std::out_of_range e) {
            // ERROR: Variable already exists
            std::cerr << "Variable \"" << id << "\" redefined" << std::endl;
        }
    }
    else {
        ret.varVal = &sym->get(id);
    }

    tokQueue.push(ret);

    // Discard any optional whitespace
    in.eatWhitespace();
}
