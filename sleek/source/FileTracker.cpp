#include "FileTracker.hpp"
#include "helper.hpp"

#include <string>
#include <fstream>
#include <sstream>

#define SS_EOF (std::stringstream::traits_type::eof())
#define FS_EOF (std::ifstream::traits_type::eof())

FileTracker::FileTracker(const char* filename): fin(filename) {
    line = 0;
    newLine();
}

void FileTracker::eatWhitespace() {
    while (std::isspace(peek())) {
        get();
    }
}

int FileTracker::peek() {
    if (lin.peek() == SS_EOF) {
        if (fin.peek() == FS_EOF) {
            return eof();
        }
        return '\n';
    }

    return lin.peek();
}

int FileTracker::get() {
    if (lin.peek() == SS_EOF) {
        if (fin.peek() == FS_EOF) {
            return eof();
        }
        newLine();
        return '\n';
    }

    column += 1;
    return lin.get();
}

int FileTracker::eof() {
    return -1;
}

/*
 * Buffer
 */

void FileTracker::bufferIdentifier() {
    bufferLine    = line;
    bufferColumn  = column;
    buffered    = true;

    buffer = "";

    char peek = this->peek();
    while (isLetter(peek) || isNumber(peek) || peek == '_' || peek == '-') {
        buffer += (char) get();
        peek = this->peek();
    }
}

void FileTracker::clearBuffer() {
    buffered = false;
}

bool FileTracker::isBuffered() {
    return buffered;
}

std::string FileTracker::getBufferedIdentifier() {
    return buffer;
}

unsigned int FileTracker::getBufferLine() {
    return bufferLine;
}

unsigned int FileTracker::getBufferColumn() {
    return bufferColumn;
}

unsigned int FileTracker::getLine() {
    return line;
}

unsigned int FileTracker::getColumn() {
    return column;
}

void FileTracker::newLine() {
    column = 1;
    line += 1;

    std::string linestring;
    std::getline(fin, linestring);
    std::stringstream newLine(std::move(linestring));

    lin = std::move(newLine);
}
