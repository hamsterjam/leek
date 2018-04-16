#include "Lexer.hpp"
#include "Token.hpp"
#include "SymbolTable.hpp"
#include "Variable.hpp"

#include <iostream>
#include <ostream>
#include <sstream>
#include <string>

#define SS_EOF (std::stringstream::traits_type::eof())

bool pass;
#define startTest(msg) pass = true;std::cout<<(msg)<<std::flush
#define endTest() std::cout<<(pass?"OK!":"Fail")<<std::endl
#define assert(x) pass = pass && (x)

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

    {   // Basic Definitions
        startTest("Testing definition lexing\t");

        std::string source(R"(
            foo : int = 2;
            bar := 3;
            baz : uint;
        )");
        SymbolTable sym;
        std::stringstream err;

        LexerTest lex(std::move(source), sym, err);

        // Assert the token stream is correct
        assert(lex.matches({
            ID, DEF, KEY, OP2, INT, EOS,
            ID, DEF, OP2, INT, EOS,
            ID, DEF, KEY, EOS,
            END
        }));

        // Assert that there were no errors
        assert(err.peek() == SS_EOF);
        assert(lex.errorCount() == 0);

        endTest();
    }
    {   // Reference aliasing
        startTest("Testing reference aliasing\t");

        std::string source(R"(
            foo;
            foo : int;
            foo;
            { foo : int; }
            { foo; }
        )");
        SymbolTable sym;
        std::stringstream err;

        LexerTest lex(std::move(source), sym, err);
        lex.lexAll();

        // Assert that there were no errors
        assert(err.peek() == SS_EOF);
        assert(lex.errorCount() == 0);

        // Not interested in token types now, only that the references match
        Variable* globalFoo = &lex.get().varVal->getValue();

        // Discard the EOS
        lex.get();
        // Assert that the next ID has the same value object
        assert(&lex.get().varVal->getValue() == globalFoo);
        // Discard DEF, KEY, EOS
        for (int i = 0; i < 3; ++i) lex.get();
        // Assert that the next ID has the same value object
        assert(&lex.get().varVal->getValue() == globalFoo);
        // Discard EOS, OB
        for (int i = 0; i < 2; ++i) lex.get();
        // Assert that the next ID does NOT have the same value object
        assert(&lex.get().varVal->getValue() != globalFoo);
        // Discard DEF, KEY, EOS, CB, OB
        for (int i = 0; i < 5; ++i) lex.get();
        // Assert that the next ID has the same value object
        assert(&lex.get().varVal->getValue() == globalFoo);

        endTest();
    }
    {   // Parens
        startTest("Testing parentheses...\t\t");

        std::string source(R"(
            (a + b);
            a + (b);
            ((a + b));
            a + ((b));
        )");
        SymbolTable sym;
        std::stringstream err;

        LexerTest lex(std::move(source), sym, err);

        // Assert the tokens are correct
        assert(lex.matches({
            OP, ID, OP2, ID, CP, EOS,
            ID, OP2, OP, ID, CP, EOS,
            OP, OP, ID, OP2, ID, CP, CP, EOS,
            ID, OP2, OP, OP, ID, CP, CP, EOS,
            END
        }));

        // Assert no errors
        assert(err.peek() == SS_EOF);
        assert(lex.errorCount() == 0);

        endTest();
    }

    return 0;
}
