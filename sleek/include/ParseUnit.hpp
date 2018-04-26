#ifndef LEEK_SLEEK_PARSE_UNIT_H_DEFINED
#define LEEK_SLEEK_PARSE_UNIT_H_DEFINED

#include "Token.hpp"
#include "AsyncQueue.hpp"

#include <deque>
#include <set>

#include <mutex>
#include <condition_variable>
#include <atomic>
#include <thread>

class ParseUnit {
    public:
        void push(Token val);

        // Async actions
        void generate();
        void join();

    private:
        AsyncQueue<Token> iBuff;

        std::thread th;
};

#endif
