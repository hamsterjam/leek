#ifndef LEEK_SLEEK_PARSE_UNIT_H_DEFINED
#define LEEK_SLEEK_PARSE_UNIT_H_DEFINED

#include "Token.hpp"
#include "AsyncQueue.hpp"

#include <deque>
#include <list>

#include <thread>

class ParseUnit {
    public:
        void push(Token val);

        // Async actions
        void generate();
        void join();

    private:
        AsyncQueue<Token> iBuff;
        std::list<ParseUnit> children;

        std::thread th;
};

#endif
