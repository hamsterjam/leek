#ifndef LEEK_SLEEK_FILE_TRACKER_H_DEFINED
#define LEEK_SLEEK_FILE_TRACKER_H_DEFINED

#include <string>
#include <fstream>
#include <sstream>

class FileTracker {
    public:
        FileTracker(const char* filename);

        void eatWhitespace();

        int peek();
        int get();

        void bufferIdentifier();
        void clearBuffer();
        bool isBuffered();
        std::string getBufferedIdentifier();

        unsigned int getBufferLine();
        unsigned int getBufferColumn();

        unsigned int getLine();
        unsigned int getColumn();

        static int eof();

    private:
        std::ifstream     fin;
        std::stringstream lin;

        bool buffered;
        std::string buffer;
        unsigned int bufferLine;
        unsigned int bufferColumn;

        unsigned int line;
        unsigned int column;

        void newLine();
};

#endif
