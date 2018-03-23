#ifndef LEEK_SLEEK_LEXER_H_DEFINED
#define LEEK_SLEEK_LEXER_H_DEFINED

#include "Token.hpp"

#include <queue>
#include <fstream>

class SymbolTable;

class Lexer {
    public:
        Lexer(const char* filename, SymbolTable& sym);
        Token peek();
        Token get();
        void operator>>(Token& out);

        void lexIdentifier(bool definition);
    private:
        std::queue<Token> tokQueue;
        std::ifstream in;
        SymbolTable* sym;

};

#endif
