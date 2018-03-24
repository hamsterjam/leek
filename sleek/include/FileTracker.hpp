#ifndef LEEK_SLEEK_FILE_TRACKER_H_DEFINED
#define LEEK_SLEEK_FILE_TRACKER_H_DEFINED

#include <fstream>
#include <sstream>

//TODO// This doesn't deal with eof very well....

class FileTracker {
    public:
        FileTracker(const char* filename);

        void eatWhitespace();

        int peek();
        int get();
        int eof();

        unsigned int getLine();
        unsigned int getColumn();

    private:
        std::ifstream fin;
        std::stringstream  lin;

        unsigned int line;
        unsigned int column;

        void newLine();
};

#endif
