#include "Lexer.hpp"
#include "SymbolTable.hpp"

#include <iostream>

int main(int argc, char** argv) {
    SymbolTable sym;
    Lexer lex("test", sym);

    for (int i = 0; i < 2; ++i)
        lex.lexIdentifier(true);

    return 0;
}
