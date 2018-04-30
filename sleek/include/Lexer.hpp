#ifndef LEEK_SLEEK_LEXER_H_DEFINED
#define LEEK_SLEEK_LEXER_H_DEFINED

#include "Token.hpp"
#include "FileTracker.hpp"
#include "SymbolTable.hpp"

#include <deque>
#include <ostream>
#include <string>

class FileTracker;

class Lexer {
    protected:
        // These are testing constructors, you probably don't want to call them
        Lexer(std::string&  in, std::ostream& err);
        Lexer(std::string&& in, std::ostream& err);

    public:
        Lexer(const char* filename);

        Token peek();
        Token get();
        void operator>>(Token& out);

        unsigned int errorCount();

        void lexAll();

    private:
        std::deque<Token> tokQueue;
        FileTracker in;
        SymbolTable  symRoot;
        SymbolTable* sym;

        std::ostream& out;

        bool lexingArgList;
        bool lexingParamList;

        unsigned int scopeLevel;
        bool inClassBlock;

        unsigned int errors;

        void lexSomeTokens();

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
