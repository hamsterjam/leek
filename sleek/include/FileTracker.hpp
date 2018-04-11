#ifndef LEEK_SLEEK_FILE_TRACKER_H_DEFINED
#define LEEK_SLEEK_FILE_TRACKER_H_DEFINED

#include <string>
#include <fstream>
#include <istream>
#include <sstream>

class Lexer;

class FileTracker {
    protected:
        // These exist for testing, im aware of how easy it is to mistake this
        // for the public constructor
        friend Lexer;

        FileTracker(std::string&  in);
        FileTracker(std::string&& in);

    public:
        FileTracker(const char* filename);

        ~FileTracker();

        void eatWhitespace();

        int peek();
        int get();
        int getRaw();

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
        std::istream*     fin;
        std::stringstream lin;

        bool buffered;
        std::string buffer;
        unsigned int bufferLine;
        unsigned int bufferColumn;

        unsigned int line;
        unsigned int column;

        void discardComments();
        void newLine();
};

#endif
