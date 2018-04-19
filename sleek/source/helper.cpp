#include "helper.hpp"
#include <string>

bool isNumber(char test) {
    return test >= '0' && test <= '9';
}

bool isHexNumber(char test) {
    return isNumber(test) || (test >= 'a' && test <= 'f') || (test >= 'A' && test <= 'F');
}

bool isLetter(char test) {
    return (test >= 'a' && test <= 'z') || (test >= 'A' && test <= 'Z');
}

bool isOperatorChar(char test) {
    return test == '+' || test == '-' || test == '*' || test == '/'
        || test == '%' || test == '&' || test == '|' || test == '^'
        || test == '=' || test == '!' || test == '<' || test == '>';
}

bool isUnaryOperatorChar(char test) {
    return test == '&' || test == '-' || test == '!';
}

bool isKeyword(std::string& test) {
    if      (test == "class")  return true;
    else if (test == "defer")  return true;
    else if (test == "new")    return true;
    // Control statements
    else if (test == "return") return true;
    else if (test == "if")     return true;
    else if (test == "else")   return true;
    else if (test == "elif")   return true;
    else if (test == "while")  return true;
    else if (test == "do")     return true;
    else if (test == "for")    return true;
    // Loop flow control
    else if (test == "break")  return true;
    else if (test == "pass")   return true;
    // Types
    else if (test == "void")   return true;
    else if (test == "int")    return true;
    else if (test == "uint")   return true;
    else if (test == "type")   return true;
    else if (test == "func")   return true;
    // Values
    else if (test == "true")   return true;
    else if (test == "false")  return true;
    else if (test == "null")   return true;

    return false;
}

bool isKeywordOperator(std::string& test) {
    if      (test == "const") return true;

    return false;
}
