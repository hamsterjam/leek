#include "Lexer.hpp"
#include "Token.hpp"
#include "SymbolTable.hpp"
#include "Variable.hpp"

#include <iostream>
#include <ostream>
#include <sstream>
#include <string>

#include <cstring>
#include <cstdint>

#define SS_EOF (std::stringstream::traits_type::eof())

int ERR_TEST;
int ERR_TOTAL = 0;
#define startTest(msg) ERR_TEST = 0; std::cout << (msg) << std::flush
#define endTest() ERR_TOTAL += ERR_TEST; std::cout << (ERR_TEST ? "Fail" : "OK!") << std::endl
#define assert(x) ERR_TEST += (x) ? 0 : 1;
#define errors() ERR_TOTAL

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
    std::cout << "Testing things that should not result in lex errors:" << std::endl;

    {   // Number lexing
        startTest("Number lexing...\t\t\t");

        std::string source(R"(
            10;
            0010;
            0b10;
            0B10;
            0c10;
            0C10;
            0x10;
            0X10;
        )");
        SymbolTable sym;
        std::stringstream err;

        LexerTest lex(std::move(source), sym, err);

        auto nextIs = [&lex](uint16_t val) {
            assert(lex.peek().type == INT);
            assert(lex.get().intVal == val);
            lex.get();
        };

        nextIs(10);
        nextIs(10);
        nextIs(2);
        nextIs(2);
        nextIs(8);
        nextIs(8);
        nextIs(16);
        nextIs(16);

        // Assert no errors
        assert(err.peek() == SS_EOF);
        assert(lex.errorCount() == 0);

        endTest();
    }
    {   // Variablish keywords
        startTest("Variablish keywords...\t\t\t");

        std::string source(R"(
            void;
            int;
            uint;
            type;
            func;
            true;
            false;
            null;
        )");
        SymbolTable sym;
        std::stringstream err;

        LexerTest lex(std::move(source), sym, err);

        auto nextIs = [&lex](const char* val) {
            assert(lex.peek().type == KEY);
            assert(!strncmp(lex.get().stringVal, val, 8));
            lex.get();
        };

        // Make sure the tokens are right
        nextIs("void");
        nextIs("int");
        nextIs("uint");
        nextIs("type");
        nextIs("func");
        nextIs("true");
        nextIs("false");
        nextIs("null");

        // Assert no errors
        assert(err.peek() == SS_EOF);
        assert(lex.errorCount() == 0);

        endTest();
    }
    {   // Conditional control
        startTest("Conditional control statements...\t");

        std::string source(R"(
            if cond {
                foo();
            }
            elif a + b == c - d {
                bar();
            }
            else
                baz();
        )");
        SymbolTable sym;
        std::stringstream err;

        LexerTest lex(std::move(source), sym, err);

        // Assert the tokens are correct
        assert(lex.matches({
            KEY, ID, OB,
            ID, OAL, CAL, EOS,
            CB,
            KEY, ID, OP2, ID, OP2, ID, OP2, ID, OB,
            ID, OAL, CAL, EOS,
            CB,
            KEY,
            ID, OAL, CAL, EOS,
            END
        }));

        // Assert no errors
        assert(err.peek() == SS_EOF);
        assert(lex.errorCount() == 0);

        endTest();
    }
    {   // Loop control
        startTest("Loop control statements...\t\t");

        std::string source(R"(
            i := 0;
            while i < 5 {
                foo(i);
                i += 1;
            }

            i = 0;
            do {
                foo(i);
                i += 1;
            } while i <= 5;

            for (i := 0; i < 5; i += 1)
                foo(i);
        )");
        SymbolTable sym;
        std::stringstream err;

        LexerTest lex(std::move(source), sym, err);

        // Get a reference to the first i so we can check scoping rules
        Variable* globalI = &lex.peek().varVal->getValue();

        // Check some tokens
        assert(lex.matches({
            ID, DEF, OP2, INT, EOS,
            KEY, ID, OP2, INT, OB,
            ID, OAL, ID, CAL, EOS
        }));

        // This i (in the while loop) should be the global i
        assert(&lex.peek().varVal->getValue() == globalI);

        // Check some more tokens
        assert(lex.matches({
            ID, OP2, INT, EOS,
            CB,
            ID, OP2, INT, EOS,
            KEY, OB,
            ID, OAL, ID, CAL, EOS
        }));

        // This i (in the do-while loop) should be the global i
        assert(&lex.peek().varVal->getValue() == globalI);

        // Check some more tokens
        assert(lex.matches({
            ID, OP2, INT, EOS,
            CB, KEY, ID, OP2, INT, EOS,
            KEY, ID, DEF, OP2, INT, EOS, ID, OP2, INT, EOS, ID, OP2, INT,
            ID, OAL
        }));

        // This i should be localy scoped to the definition in the head
        assert(&lex.peek().varVal->getValue() != globalI);

        // Lex the rest of the tokens
        assert(lex.matches({ID, CAL, EOS, END}));

        // Assert we have no errors
        assert(err.peek() == SS_EOF);
        assert(lex.errorCount() == 0);

        endTest();
    }
    {   // Unary operators
        startTest("Unary operators...\t\t\t");

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
        startTest("Binary operators...\t\t\t");

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
    {   // Ambiguous operator resolution
        startTest("Ambiguous operator resolution...\t");

        std::string source(R"(
            a - - - (a - - - a);
        )");
        SymbolTable sym;
        std::stringstream err;

        LexerTest lex(std::move(source), sym, err);

        // Assert the tokens are correct
        assert(lex.matches({
            ID, OP2, OP1, OP1, OP, ID, OP2, OP1, OP1, ID, CP, EOS,
            END
        }));

        // Assert that there are no errors
        assert(err.peek() == SS_EOF);
        assert(lex.errorCount() == 0);

        endTest();
    }
    {   // Basic definitions
        startTest("Basic definition lexing...\t\t");

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
        startTest("Reference aliasing...\t\t\t");

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
        startTest("Parenthesis pairing...\t\t\t");

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
        startTest("Function definitions...\t\t\t");

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
        startTest("Function calls...\t\t\t");

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

    return errors();
}
