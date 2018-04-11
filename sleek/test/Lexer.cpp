#include "Lexer.hpp"
#include "SymbolTable.hpp"

#include <iostream>
#include <ostream>
#include <string>

class LexerTest : public Lexer {
    public:
        LexerTest(std::string&& in, SymbolTable& sym, std::ostream& err):
            Lexer(std::forward<std::string&&>(in), sym, err)
        {
        }
};

const char* test1 = R"(
Vector := type<T : type> {
    return class {
        this :: (x : T, y : T) {
            this.x = x;
            this.y = y;
        }

        op+= :: (rhs : &Vector<T>) {
            this.x = this.x + rhs.x;
            this.y = this.y + rhs.y;
        }

        x : T;
        y : T;
    }
}

p := new Vector<int>(1, 2);
q := new Vector<int>(3, 4);

p += q; // p contains a Vector<int> with x = 4, y = 6
)";

int main(int argc, char** argv) {
    SymbolTable sym;

    std::string src(test1);
    LexerTest lex(std::move(src), sym, std::cerr);
    lex.lexAll();

    return 0;
}
