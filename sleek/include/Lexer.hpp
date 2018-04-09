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

        void lexAll();

    private:
        std::queue<Token> tokQueue;
        FileTracker in;
        SymbolTable* sym;

        bool lexingArgList;
        bool lexingParamList;

        unsigned int scopeLevel;
        bool inClassBlock;

        unsigned int errorCount;

        // Recursive Lexers
        void lexStatement();
        void lexRawStatement();
        void lexRegularStatement();
        void lexClassStatement();
        void lexPostStatement();

        void lexAccessSpecifier();

        void lexExpression();
        void lexPostExpression();

        void lexDefinition();
        void lexArgList();
        void lexParamList();

        // Function Expression Lexers
        void lexVoidFunctionExpression(bool compileTime);
        void lexFunctionExpression(bool compileTime);
        void lexFunctionExpressionFromList(bool compileTime);
        void lexFunctionExpressionFromBlock(bool compileTime);

        // Terminal Lexers
        void lexWhitespace();
        void lexUnaryOperator();
        void lexBinaryOperator();
        void lexIdentifier(bool definition);
        void lexKeyword();
        void lexKeywordOperator();
        void lexNumber();
};

#endif
