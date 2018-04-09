#ifndef LEEK_SLEEK_ERROR_H_DEFINED
#define LEEK_SLEEK_ERROR_H_DEFINED

class Error {
    public:
        Error(const char* message, unsigned int line, unsigned int col);
        void print();

    private:
        const char* message;
        unsigned int line;
        unsigned int col;
};

#endif
