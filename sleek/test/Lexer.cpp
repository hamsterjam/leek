#include "Lexer.hpp"
#include "Token.hpp"
#include "SymbolTable.hpp"

#include <iostream>
#include <ostream>
#include <sstream>
#include <string>

#define SS_EOF (std::stringstream::traits_type::eof())

#define assert(x) (pass = pass && (x))

// Short token names
const Token::Type ID     = Token::Type::IDENTIFIER;
const Token::Type KEY    = Token::Type::KEYWORD;
const Token::Type INT    = Token::Type::INTEGER;
const Token::Type OB     = Token::Type::OPENING_BLOCK;
const Token::Type CB     = Token::Type::CLOSING_BLOCK;
const Token::Type OP     = Token::Type::OPENING_PAREN;
const Token::Type CP     = Token::Type::CLOSING_PAREN;
const Token::Type OAS    = Token::Type::OPENING_ACCESS_SPECIFIER;
const Token::Type CAS    = Token::Type::CLOSING_ACCESS_SPECIFIER;
const Token::Type OIB    = Token::Type::OPENING_INDEX_BRACKET;
const Token::Type CIB    = Token::Type::CLOSING_INDEX_BRACKET;
const Token::Type OAL    = Token::Type::OPENING_ARG_LIST;
const Token::Type CAL    = Token::Type::CLOSING_ARG_LIST;
const Token::Type OAL_CT = Token::Type::OPENING_ARG_LIST_CT;
const Token::Type CAL_CT = Token::Type::CLOSING_ARG_LIST_CT;
const Token::Type OPL    = Token::Type::OPENING_PARAM_LIST;
const Token::Type CPL    = Token::Type::CLOSING_PARAM_LIST;
const Token::Type OPL_CT = Token::Type::OPENING_PARAM_LIST_CT;
const Token::Type CPL_CT = Token::Type::CLOSING_PARAM_LIST_CT;
const Token::Type PA     = Token::Type::PROPERTY_ACCESS;
const Token::Type COM    = Token::Type::COMMA;
const Token::Type DEF    = Token::Type::DEFINITION;
const Token::Type OVER   = Token::Type::OVERLOAD;
const Token::Type OP1    = Token::Type::UNARY_OPERATOR;
const Token::Type OP2    = Token::Type::BINARY_OPERATOR;
const Token::Type EOS    = Token::Type::END_OF_STATEMENT;
const Token::Type END    = Token::Type::END_OF_FILE;

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
        assert(lex.matches({
            ID, DEF, KEY, OP2, INT, EOS,
            ID, DEF, OP2, INT, EOS,
            ID, DEF, KEY, EOS,
            END
        }));

        // Assert that there were no errors
        assert(err.peek() == SS_EOF);
        assert(lex.errorCount() == 0);

        if (pass) std::cout << "OK!"  << std::endl;
        else      std::cout << "Fail" << std::endl;
    }

    return 0;
}
