#ifndef LEEK_SLEEK_HELPER_H_DEFINED
#define LEEK_SLEEK_HELPER_H_DEFINED

#include <string>

bool isNumber(char test);
bool isHexNumber(char test);
bool isLetter(char test);
bool isOperatorChar(char test);
bool isUnaryOperatorChar(char test);
bool isKeyword(std::string& test);
bool isKeywordOperator(std::string& test);

#endif
