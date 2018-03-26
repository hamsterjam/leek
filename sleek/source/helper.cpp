#include "helper.hpp"

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
