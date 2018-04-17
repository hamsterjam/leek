#include "Lexer.hpp"
#include "Token.hpp"
#include "SymbolTable.hpp"
#include "Variable.hpp"

#include <iostream>
#include <ostream>
#include <sstream>
#include <string>

#include <cstring>

#define SS_EOF (std::stringstream::traits_type::eof())

int err_test;
int err_total = 0;
#define startTest(msg) err_test = 0; std::cout << (msg) << std::flush
#define endTest() err_total += err_test; std::cout << (err_test ? "Fail" : "OK!") << std::endl
#define assert(x) err_test += (x) ? 0 : 1;
#define errors() err_total

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
    std::cout << "Things that should not result in lex errors:" << std::endl;

    {   // Basic definitions
        startTest("Testing definition lexing...\t");

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
        startTest("Testing reference aliasing...\t");

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
    {   // Function definitions
        startTest("Testing function definitions...\t");

        std::string source(R"(
            foo :: () {}
            inc :: &int (x : &int) {
                return x + 1;
            }
            foo :: <> {}
            sub :: int <x : int, y : int = 1> {
                return x - y;
            }
        )");
        SymbolTable sym;
        std::stringstream err;

        LexerTest lex(std::move(source), sym, err);

        // Assert tokens are right
        assert(lex.matches({
            ID, OVER, KEY, OPL, CPL, OB, CB,
            ID, OVER, OP1, KEY, OPL, ID, DEF, OP1, KEY, CPL, OB,
                KEY, ID, OP2, INT, EOS,
            CB,
            ID, OVER, KEY, OPL_CT, CPL_CT, OB, CB,
            ID, OVER, KEY, OPL_CT, ID, DEF, KEY, COM, ID, DEF, KEY, OP2, INT, CPL_CT, OB,
                KEY, ID, OP2, ID, EOS,
            CB,
            END
        }));

        // Assert no errors
        assert(err.peek() == SS_EOF);
        assert(lex.errorCount() == 0);

        endTest();
    }
    {   // Function calls
        startTest("Testing function calls...\t");

        std::string source(R"(
            foo();
            foo(1, 2, 3);
            foo<1,  , 3>;
            <>{}<>;
            (){}();
            foo(1 + 2, bar(20) - 6);
        )");
        SymbolTable sym;
        std::stringstream err;

        LexerTest lex(std::move(source), sym, std::cerr);

        // Assert we have correct tokens
        assert(lex.matches({
            ID, OAL, CAL, EOS,
            ID, OAL, INT, COM, INT, COM, INT, CAL, EOS,
            ID, OAL_CT, INT, COM, COM, INT, CAL_CT, EOS,
            KEY, OPL_CT, CPL_CT, OB, CB, OAL_CT, CAL_CT, EOS,
            KEY, OPL, CPL, OB, CB, OAL, CAL, EOS,
            ID, OAL, INT, OP2, INT, COM, ID, OAL, INT, CAL, OP2, INT, CAL, EOS,
            END
        }));

        // Assert there were no errors
        assert(err.peek() == SS_EOF);
        assert(lex.errorCount() == 0);

        endTest();
    }
    {   // Unary operators
        startTest("Testing unary operators...\t");

        std::string source(R"(
            &a;
            -a;
            !a;
            const a;
        )");
        SymbolTable sym;
        std::stringstream err;

        LexerTest lex(std::move(source), sym, err);

        auto nextIs = [&lex](const char* val) {
            assert(lex.peek().type == OP1);
            assert(!strncmp(lex.peek().stringVal, val, 8));
            for (int i = 0; i < 3; ++i) lex.get();
        };

        nextIs("&");
        nextIs("-");
        nextIs("!");
        nextIs("const");

        // Assert no errors
        assert(err.peek() == SS_EOF);
        assert(lex.errorCount() == 0);

        endTest();
    }
    {   // Binary operators
        startTest("Testing binary operators...\t");

        std::string source(R"(
            A +  A -  A *  A /  A %  A;
            A &  A |  A ^  A << A >> A;
            A && A || A == A != A;
            A <= A >= A >  A <  A;
            A =  A;
            A += A -= A *= A /= A %= A;
            A &= A |= A ^= A;
        )");
        SymbolTable sym;
        std::stringstream err;

        LexerTest lex(std::move(source), sym, err);

        auto nextIs = [&lex](const char* val){
            lex.get();
            assert(lex.peek().type == OP2);
            assert(!strncmp(lex.get().stringVal, val, 8));
        };

        // Arithmetic
        nextIs("+");
        nextIs("-");
        nextIs("*");
        nextIs("/");
        nextIs("%");
        lex.get(); lex.get();
        // Bitwise logic
        nextIs("&");
        nextIs("|");
        nextIs("^");
        nextIs("<<");
        nextIs(">>");
        lex.get(); lex.get();
        // Bool logic
        nextIs("&&");
        nextIs("||");
        nextIs("==");
        nextIs("!=");
        lex.get(); lex.get();
        // Comparison
        nextIs("<=");
        nextIs(">=");
        nextIs(">");
        nextIs("<");
        lex.get(); lex.get();
        // Assignment
        nextIs("=");
        lex.get(); lex.get();
        // Arithmetic assignment
        nextIs("+=");
        nextIs("-=");
        nextIs("*=");
        nextIs("/=");
        nextIs("%=");
        lex.get(); lex.get();
        // Logic assignment
        nextIs("&=");
        nextIs("|=");
        nextIs("^=");

        // Assert no errors
        assert(err.peek() == SS_EOF);
        assert(lex.errorCount() == 0);

        endTest();
    }

    return errors();
}
