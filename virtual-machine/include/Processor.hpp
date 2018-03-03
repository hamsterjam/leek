/*
 * Processor.hpp
 *
 * This is where all the actuall computation happens
 *
 * -- Callum Nicholson
 */
#ifndef LEEK_VM_PROCESSOR_H_DEFINED
#define LEEK_VM_PROCESSOR_H_DEFINED

#include "MemoryManager.hpp"
#include "RegisterManager.hpp"

#include <mutex>
#include <condition_variable>
#include <atomic>

#include <cstdlib>
#include <cstdint>

class IODevice;

class Processor {
    public:
        Processor(size_t memWords);

        void exec(uint16_t instruction);
        void tick();
        void run();
        void interrupt(int line); /* thread safe */

        void useDevice(IODevice& dev, size_t pos, uint8_t line);
        void removeDevice(IODevice& dev);

        void push(uint16_t instruction);
        uint16_t inspect(size_t index);
    private:
        std::mutex sleepM;
        std::condition_variable sleepCV;

        std::atomic<bool> anyISF;
        std::atomic<bool> softISF;
        std::atomic<bool> hardISF[8];

        MemoryManager mem;
        RegisterManager reg;
};

#endif
