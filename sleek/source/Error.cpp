#include "Error.hpp"

#include <string>
#include <iostream>

Error::Error(const char* msg, unsigned int line, unsigned int col) :
    message(msg)
{
    this->line = line;
    this->col  = col;
}

Error::Error(std::string& msg, unsigned int line, unsigned int col) :
    message(msg)
{
    this->line = line;
    this->col  = col;
}

Error::Error(std::string&& msg, unsigned int line, unsigned int col) :
    message(std::forward<std::string&&>(msg))
{
    this->line = line;
    this->col  = col;
}

Error::Error(Error&& e) :
    Error(std::move(e.message), e.line, e.col)
{
}

void Error::print() {
    std::cerr << message << " ";
    std::cerr << "at (" << line << ", " << col << ")" << std::endl;
}
