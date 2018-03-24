#include "Lexer.hpp"
#include "SymbolTable.hpp"
#include "Token.hpp"

#include <iostream>

int main(int argc, char** argv) {
    SymbolTable sym;
    Lexer lex("test", sym);

    for (int i = 0; i < 4; ++i) {
        lex.lexWhitespace();
        lex.lexNumber();
    }

    for (int i = 0; i < 4; ++i) {
        std::cout << lex.get().intVal << std::endl;
    }

    return 0;
}
