#ifndef LEEK_SLEEK_LEXER_H_DEFINED
#define LEEK_SLEEK_LEXER_H_DEFINED

#include "Token.hpp"
#include "FileTracker.hpp"

#include <queue>

class SymbolTable;

class Lexer {
    public:
        Lexer(const char* filename, SymbolTable& sym);
        Token peek();
        Token get();
        void operator>>(Token& out);

        // Recursive Lexers
        void lexExpression();
        void lexDefinition();
        void lexParamList();

        // Terminal Lexers
        void lexWhitespace();
        void lexUnaryOperator();
        void lexBinaryOperator();
        void lexIdentifier(bool definition);
        void lexNumber();
    private:
        std::queue<Token> tokQueue;
        FileTracker in;
        SymbolTable* sym;

};

#endif
