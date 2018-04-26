#include "ParseUnit.hpp"
#include "Token.hpp"

#include <mutex>
#include <condition_variable>
#include <atomic>
#include <thread>

ParseUnit::ParseUnit() {
    iBuffEmpty = true;
}

void ParseUnit::bufferPush(Token tok) {
    std::lock_guard<std::mutex> lk(iBuffAccess);
    iBuff.push_back(tok);

    iBuffEmpty = false;
    iBuffPush.notify_all();
}

// Gets a token from the buffer, if the buffer is empty, the thread will wait
// till there is something to get
Token ParseUnit::bufferGet() {
    if (iBuffEmpty) {
        std::unique_lock<std::mutex> lk(waiting);
        while (iBuffEmpty) iBuffPush.wait(lk);
    }

    std::lock_guard<std::mutex> lk(iBuffAccess);
    Token ret = iBuff.front();
    iBuff.pop_front();
    if (iBuff.size() == 0) iBuffEmpty = true;

    return ret;
}

void ParseUnit::generate() {
    auto routine = [this]() {
        Token next;
        do {
            next = bufferGet();
        } while (next.type != Token::Type::END_OF_FILE);
    };

    th = std::thread(routine);
}

void ParseUnit::join() {
    th.join();
}
