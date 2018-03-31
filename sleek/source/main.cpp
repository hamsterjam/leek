#include "Lexer.hpp"
#include "SymbolTable.hpp"
#include "Token.hpp"

#include <iostream>

int main(int argc, char** argv) {
    SymbolTable sym;
    Lexer lex("test", sym);

    return 0;
}
