#include "Lexer.hpp"
#include "SymbolTable.hpp"
#include "Token.hpp"

#include <iostream>

int main(int argc, char** argv) {
    SymbolTable sym;
    Lexer lex("test", sym);

    lex.lexWhitespace();
    lex.lexStatement();
    lex.lexStatement();
    lex.lexStatement();

    return 0;
}
