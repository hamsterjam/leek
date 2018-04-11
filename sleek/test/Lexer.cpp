#include "Lexer.hpp"
#include "Token.hpp"
#include "SymbolTable.hpp"

#include <iostream>
#include <ostream>
#include <sstream>
#include <string>

#define SS_EOF (std::stringstream::traits_type::eof())

#define t(x) (Token::Type::x)

class LexerTest : public Lexer {
    public:
        LexerTest(std::string&& in, SymbolTable& sym, std::ostream& err):
            Lexer(std::forward<std::string&&>(in), sym, err)
        {
        }

        bool matches(std::initializer_list<Token::Type> list) {
            for (auto i : list) {
                if (i != get().type) return false;
            }
            return true;
        }
};

int main(int argc, char** argv) {
    SymbolTable sym;

    // First test things that should lex
    {
        // Basic definitions
        std::cout << "Testing definition lexing...\t" << std::flush;

        bool pass = true;

        std::string source(R"(
            foo : int = 2;
            bar := 3;
            baz : uint;
        )");
        SymbolTable sym;
        std::stringstream err;

        // Assert the token stream is correct
        LexerTest lex(std::move(source), sym, err);
        pass = pass && lex.matches({
            t(IDENTIFIER), t(DEFINITION), t(KEYWORD), t(BINARY_OPERATOR), t(INTEGER), t(END_OF_STATEMENT),
            t(IDENTIFIER), t(DEFINITION), t(BINARY_OPERATOR), t(INTEGER), t(END_OF_STATEMENT),
            t(IDENTIFIER), t(DEFINITION), t(KEYWORD), t(END_OF_STATEMENT),
            t(END_OF_FILE)
        });

        // Assert that there were no errors
        pass = pass && err.peek() == SS_EOF;

        if (pass) std::cout << "OK!"  << std::endl;
        else      std::cout << "Fail" << std::endl;
    }

    return 0;
}
