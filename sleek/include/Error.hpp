#ifndef LEEK_SLEEK_ERROR_H_DEFINED
#define LEEK_SLEEK_ERROR_H_DEFINED

#include <string>
#include <ostream>

class Error {
    public:
        Error(const char*   msg, unsigned int line, unsigned int col);
        Error(std::string&  msg, unsigned int line, unsigned int col);
        Error(std::string&& msg, unsigned int line, unsigned int col);

        // Copy constructor
        Error(Error& e) = default;
        // Move constructor
        Error(Error&& e);

        void print();
        void print(std::ostream& out);

    private:
        std::string message;
        unsigned int line;
        unsigned int col;
};

#endif
