#include "Lexer.hpp"
#include "SymbolTable.hpp"
#include "Token.hpp"

#include <iostream>

int main(int argc, char** argv) {
    SymbolTable sym;
    Lexer lex("test", sym);

    lex.lexAll();

    if (lex.errorCount()) {
        std::cout << "Aborting with " << lex.errorCount() << " lex errors" << std::endl;
    }
    else {
        std::cout << "Lexing complete with no errors." << std::endl;
    }

    return 0;
}
