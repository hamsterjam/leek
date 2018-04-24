#include "helper.hpp"
#include "Token.hpp"
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
    return stringToKeyword(test) != Keyword::NA;
}

bool isKeywordOperator(std::string& test) {
    return stringToKeywordOperator(test) != UnaryOperator::NA;
}

Keyword stringToKeyword(std::string& val) {
    if (val == "class")   return Keyword::CLASS;
    if (val == "defer")   return Keyword::DEFER;
    if (val == "static")  return Keyword::STATIC;
    if (val == "new")     return Keyword::NEW;
    // Visibilities
    if (val == "public")  return Keyword::PUBLIC;
    if (val == "private") return Keyword::PRIVATE;
    if (val == "nowrite") return Keyword::NOWRITE;
    // Control statements
    if (val == "return")  return Keyword::RETURN;
    if (val == "if")      return Keyword::IF;
    if (val == "else")    return Keyword::ELSE;
    if (val == "elif")    return Keyword::ELIF;
    if (val == "while")   return Keyword::WHILE;
    if (val == "do")      return Keyword::DO;
    if (val == "for")     return Keyword::FOR;
    // Loop flow control
    if (val == "break")   return Keyword::BREAK;
    if (val == "pass")    return Keyword::PASS;
    // Types
    if (val == "void")    return Keyword::VOID;
    if (val == "int")     return Keyword::INT;
    if (val == "uint")    return Keyword::UINT;
    if (val == "type")    return Keyword::TYPE;
    if (val == "func")    return Keyword::FUNC;
    // Values
    if (val == "true")    return Keyword::TRUE;
    if (val == "false")   return Keyword::FALSE;
    if (val == "null")    return Keyword::NULL_REF;

    return Keyword::NA;
}

UnaryOperator stringToKeywordOperator(std::string& val) {
    if (val == "const")   return UnaryOperator::CONST;

    return UnaryOperator::NA;
}

BinaryOperator stringToBinaryOperator(std::string& val) {
    if (val == "+")  return BinaryOperator::ADD;
    if (val == "-")  return BinaryOperator::SUB;
    if (val == "*")  return BinaryOperator::MUL;
    if (val == "/")  return BinaryOperator::DIV;
    if (val == "%")  return BinaryOperator::MOD;
    if (val == "&")  return BinaryOperator::BIT_AND;
    if (val == "|")  return BinaryOperator::BIT_OR;
    if (val == "^")  return BinaryOperator::BIT_XOR;
    if (val == "<<") return BinaryOperator::L_SHIFT;
    if (val == ">>") return BinaryOperator::R_SHIFT;
    if (val == "&&") return BinaryOperator::BOOL_AND;
    if (val == "||") return BinaryOperator::BOOL_OR;
    if (val == "==") return BinaryOperator::EQUAL;
    if (val == "!=") return BinaryOperator::NOT_EQUAL;
    if (val == "<=") return BinaryOperator::COMP_LTE;
    if (val == ">=") return BinaryOperator::COMP_GTE;
    if (val == "<")  return BinaryOperator::COMP_LT;
    if (val == ">")  return BinaryOperator::COMP_GT;
    if (val == "=")  return BinaryOperator::ASSIGN;
    if (val == "+=") return BinaryOperator::ASSIGN_ADD;
    if (val == "-=") return BinaryOperator::ASSIGN_SUB;
    if (val == "*=") return BinaryOperator::ASSIGN_MUL;
    if (val == "/=") return BinaryOperator::ASSIGN_DIV;
    if (val == "%=") return BinaryOperator::ASSIGN_MOD;
    if (val == "&=") return BinaryOperator::ASSIGN_AND;
    if (val == "|=") return BinaryOperator::ASSIGN_OR;
    if (val == "^=") return BinaryOperator::ASSIGN_XOR;

    return BinaryOperator::NA;
}
