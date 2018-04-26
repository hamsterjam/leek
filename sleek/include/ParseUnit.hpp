#ifndef LEEK_SLEEK_PARSE_UNIT_H_DEFINED
#define LEEK_SLEEK_PARSE_UNIT_H_DEFINED

#include "Token.hpp"

#include <deque>
#include <set>

#include <mutex>
#include <condition_variable>
#include <atomic>
#include <thread>

class ParseUnit {
    public:
        ParseUnit();

        void bufferPush(Token tok);
        Token bufferGet();

        // Async actions
        void generate();
        void join();

    private:
        std::deque<Token>       iBuff;
        std::mutex              iBuffAccess;
        std::atomic<bool>       iBuffEmpty;
        std::condition_variable iBuffPush;

        std::mutex waiting;
        std::thread th;
};

#endif
