#include "Error.hpp"

#include <iostream>

Error::Error(const char* message, unsigned int line, unsigned int col) {
    this->message = message;
    this->line    = line;
    this->col     = col;
}

void Error::print() {
    std::cerr << message << " ";
    std::cerr << "at (" << line << ", " << col << ")" << std::endl;
}
