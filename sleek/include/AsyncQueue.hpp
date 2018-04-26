#ifndef LEEK_SLEEK_ASYNC_QUEUE_H_DEFINED
#define LEEK_SLEEK_ASYNC_QUEUE_H_DEFINED

#include <deque>

#include <mutex>
#include <atomic>
#include <condition_variable>

template<typename T>
class AsyncQueue {
    public:
        AsyncQueue() {
            empty = true;
        }

        void push(T val) {
            std::lock_guard<std::mutex> lk(access);
            q.push_back(val);
            empty = false;
            pushed.notify_all();
        }

        T pop() {
            if (empty) {
                std::unique_lock<std::mutex> lk(waiting);
                while (empty) pushed.wait(lk);
            }

            std::lock_guard<std::mutex> lk(access);
            T ret = q.front();
            q.pop_front();
            if (q.size() == 0) empty = true;

            return ret;
        }

    private:
        std::deque<T> q;

        std::mutex              access;
        std::atomic<bool>       empty;
        std::condition_variable pushed;

        std::mutex waiting;
};

#endif
