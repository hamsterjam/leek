#include "Lexer.hpp"
#include "ParseUnit.hpp"
#include "SymbolTable.hpp"
#include "Token.hpp"

#include <iostream>

int main(int argc, char** argv) {
    Lexer lex(argv[1]);

    ParseUnit globalPU;
    globalPU.generate();

    // Push a dummy token to the front so we dont discard the opening keyword
    Token next;
    next.type = Token::Type::KEYWORD;
    next.keywordVal = Keyword::NA;
    globalPU.push(next);
    do {
        next = lex.get();
        globalPU.push(next);
    } while (next.type != Token::Type::END_OF_FILE);

    globalPU.join();

    if (lex.errorCount()) {
        std::cout << "Found " << lex.errorCount() << " lex error";
        if (lex.errorCount() == 1) {
            std::cout << "." << std::endl;
        }
        else {
            std::cout << "s." << std::endl;
        }
    }
    else {
        std::cout << "Complete with no lex errors." << std::endl;
    }

    return 0;
}
