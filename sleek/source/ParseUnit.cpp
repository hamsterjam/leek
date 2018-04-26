#include "ParseUnit.hpp"
#include "Token.hpp"

#include <mutex>
#include <condition_variable>
#include <atomic>
#include <thread>

void ParseUnit::push(Token val) {
    iBuff.push(val);
}

void ParseUnit::generate() {
    auto routine = [this]() {
        Token next;
        do {
            next = iBuff.pop();
        } while (next.type != Token::Type::END_OF_FILE);
    };

    th = std::thread(routine);
}

void ParseUnit::join() {
    th.join();
}
