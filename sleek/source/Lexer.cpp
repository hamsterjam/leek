#include "Lexer.hpp"
#include "SymbolTable.hpp"
#include "helper.hpp"

#include <queue>
#include <stdexcept>
#include <sstream>
#include <string>
#include <iostream>

#include <cstring>

Lexer::Lexer(const char* filename, SymbolTable& sym) : in(filename) {
    this->sym = &sym;
}

Token Lexer::peek() {
    return tokQueue.front();
}

Token Lexer::get() {
    Token ret = tokQueue.front();
    tokQueue.pop();
    return ret;
}

void Lexer::operator>>(Token& out) {
    out = get();
}

/*
 * Recursive Lexers
 */

void Lexer::lexExpression() {
    char peek = in.peek();

    // If the first character is a letter, treat it as an identifier
    if (isLetter(peek) || in.isBuffered()) {
        lexIdentifier(false);
        lexWhitespace();

        // Check that it actually was an identifier, it's possible that it was
        // a keyword instead
        Token id = tokQueue.back();
        if (id.type == Token::Type::UNARY_OPERATOR) {
            // Keyword style unary op (such as const)
            lexExpression();
        }
        else if (id.type == Token::Type::CLASS) {
            // class keyword
            //TODO// Lex classes
        }
    }

    // If the first character is a number, treat it as a number
    else if (isNumber(peek)) {
        lexNumber();
        lexWhitespace();
    }

    else if (isOperatorChar(peek)) {
        lexUnaryOperator();
        lexWhitespace();
        lexExpression();
    }

    // An opening paren here means either it is an actual paren, or it is a
    // function expression (returning void)
    else if (peek == '(') {
        // Discard the ( and any whitespace
        in.get();
        lexWhitespace();

        bool isFunction = false;

        if (isLetter(in.peek())) {
            // If there is an identifier, buffer it. It's either an expression
            // or a definition depending on what the paren represents
            in.bufferIdentifier();
            lexWhitespace();

            if (in.peek() == ':') {
                // It was a definition, we should treat this as a function
                isFunction = true;
            }
        }

        if (isFunction) {
            // This is a paramater list of a function
            Token retType;
            retType.type = Token::Type::KEYWORD;
            strncpy(retType.stringVal, "void", 8);
            tokQueue.push(retType);

            Token open;
            open.type = Token::Type::OPENING_PARAM_LIST;
            tokQueue.push(open);

            //TODO// Before this we need to enter a new scope
            lexParamList();

            if (in.peek() != ')') {
                // ERROR: missing closing paren
                std::cerr << "Missing ')' character ";
                std::cerr << "at (" << in.getLine() << ", " << in.getColumn() << ")" << std::endl;
                return;
            }

            // Discard the ) character
            in.get();
            lexWhitespace();

            Token close;
            close.type = Token::Type::CLOSING_PARAM_LIST;
            tokQueue.push(close);

            //TODO// Lex the actual function
        }
        else {
            // This is just some parens
            Token open;
            open.type = Token::Type::OPENING_PAREN;
            tokQueue.push(open);

            lexExpression();

            if (in.peek() != ')') {
                // ERROR: missing closing paren
                std::cerr << "Missing ')' character ";
                std::cerr << "at (" << in.getLine() << ", " << in.getColumn() << ")" << std::endl;
                return;
            }

            // Discard the ) character
            in.get();
            lexWhitespace();

            Token close;
            close.type = Token::Type::CLOSING_PAREN;
            tokQueue.push(close);
        }
    }
    else if (peek == '<') {
        // Has to be a compile time function returning void

        // Discard the < charachter
        in.get();
        lexWhitespace();

        Token retType;
        retType.type = Token::Type::KEYWORD;
        strncpy(retType.stringVal, "void", 8);
        tokQueue.push(retType);

        Token open;
        open.type = Token::Type::OPENING_PARAM_LIST_CT;
        tokQueue.push(open);

        //TODO// Before this we need to enter a new scope
        lexParamList();

        if (in.peek() != '>') {
            // ERROR: missing closing bracket
            std::cerr << "Missing '>' character ";
            std::cerr << "at (" << in.getLine() << ", " << in.getColumn() << ")" << std::endl;
            return;
        }

        // Discard the > character
        in.get();
        lexWhitespace();

        Token close;
        close.type = Token::Type::CLOSING_PARAM_LIST_CT;
        tokQueue.push(close);
    }

    // Post expression stuff
    peek = in.peek();
    if (isOperatorChar(peek)) {
        // Must be a Binary Operator
        lexBinaryOperator();
        lexWhitespace();
        lexExpression();
    }
    else if (peek == '[') {
        // Must be an array index

        // Discard the [ character
        in.get();
        lexWhitespace();

        Token open;
        open.type = Token::Type::OPENING_INDEX_BRACKET;
        tokQueue.push(open);

        lexExpression();

        if (in.peek() != ']') {
            // ERROR: missing closing bracket
            std::cerr << "Missing ']' character ";
            std::cerr << "at (" << in.getLine() << ", " << in.getColumn() << ")" << std::endl;
            return;
        }

        // Discard the ] character
        in.get();
        lexWhitespace();

        Token close;
        close.type = Token::Type::CLOSING_INDEX_BRACKET;
        tokQueue.push(close);
    }
    else if (peek == '(') {
        // Function call

        // Discard the ( character
        in.get();
        lexWhitespace();

        Token open;
        open.type = Token::Type::OPENING_ARG_LIST;
        tokQueue.push(open);

        lexArgList();

        if (in.peek() != ')') {
            // ERROR: missing closing paren
            std::cerr << "Missing ')' character ";
            std::cerr << "at (" << in.getLine() << ", " << in.getColumn() << ")" << std::endl;
            return;
        }

        // Discard the ) character
        in.get();
        lexWhitespace();

        Token close;
        close.type = Token::Type::CLOSING_ARG_LIST;
        tokQueue.push(close);
    }
    else if (peek == '<') {
        // Compile time function call

        // Discard the < character
        in.get();
        lexWhitespace();

        Token open;
        open.type = Token::Type::OPENING_ARG_LIST_CT;
        tokQueue.push(open);

        lexArgList();

        if (in.peek() != '>') {
            // ERROR: missing closing bracket
            std::cerr << "Missing '<' character ";
            std::cerr << "at (" << in.getLine() << ", " << in.getColumn() << ")" << std::endl;
            return;
        }

        // Discard the > character
        in.get();
        lexWhitespace();

        Token close;
        close.type = Token::Type::CLOSING_ARG_LIST_CT;
        tokQueue.push(close);
    }
}

void Lexer::lexDefinition() {
}

void Lexer::lexArgList() {
}

void Lexer::lexParamList() {
}

/*
 * Terminal Lexers
 */

void Lexer::lexWhitespace() {
    // Just discard it, whitespace has no meaning in sleek
    in.eatWhitespace();
}

void Lexer::lexUnaryOperator() {
    unsigned int lineNumber = in.getLine();
    unsigned int colNumber  = in.getColumn();

    Token ret;
    ret.type = Token::Type::UNARY_OPERATOR;
    char peek = in.peek();
    switch (peek) {
        case '&':
        case '-':
        case '!':
            ret.stringVal[0] = peek;
            ret.stringVal[1] = 0;
            break;
        default:
            std::cerr << "Invalid unary operator ";
            std::cerr << "at (" << lineNumber << ", " << colNumber << ")" << std::endl;
            return;
    }

    tokQueue.push(ret);
}

void Lexer::lexBinaryOperator() {
    unsigned int lineNumber = in.getLine();
    unsigned int colNumber  = in.getColumn();

    Token ret;
    ret.type = Token::Type::BINARY_OPERATOR;

    std::string id = "";

    char first = in.peek();
    switch (first) {
        case '+':
        case '-':
        case '*':
        case '/': // May be succeded by an '='
        case '%':
        case '^':
        case '=':
            id += in.get();
            if (in.peek() == '=') {
                id += in.get();
            }
            break;
        case '&':
        case '|': // May be succeded by itself or an '='
        case '<':
        case '>':
            id += in.get();
            if (in.peek() == '=') {
                id += in.get();
            }
            else if (in.peek() == first) {
                id += in.get();
            }
            break;
        case '!': // Must be succesded by an '='
            id += in.get();
            if (in.peek() == '=') {
                id += in.get();
                break;
            }
            // Else fall through to default (error)
        default:
            std::cerr << "Invalid Binary Operator ";
            std::cerr << "at (" << lineNumber << ", " << colNumber << ")" << std::endl;
            return;
    }

    int i = 0;
    for (; i < id.length(); ++i) {
        ret.stringVal[i] = id[i];
    }
    ret.stringVal[i] = 0;

    tokQueue.push(ret);
}

void Lexer::lexIdentifier(bool definition) {
    if (!in.isBuffered()) in.bufferIdentifier();

    unsigned int lineNumber = in.getBufferLine();
    unsigned int colNumber  = in.getBufferColumn();

    std::string id = in.getBufferedIdentifier();
    in.clearBuffer();

    if (id == "op") {
        // Operator identifier
        //TODO//
    }

    Token ret;

    // Handle keywords
    if (id == "class") {
        ret.type = Token::Type::CLASS;
        tokQueue.push(ret);
        return;
    }
    if (id == "const") {
        ret.type = Token::Type::UNARY_OPERATOR;
        strncpy(ret.stringVal, "const", 8);
        tokQueue.push(ret);
        return;
    }

    ret.type = Token::Type::IDENTIFIER;

    if (definition) {
        try {
            ret.varVal = &sym->define(id);
        }
        catch (std::out_of_range e) {
            // ERROR: Variable already exists
            std::cerr << "Variable \"" << id << "\" redefined ";
            std::cerr << "at (" << lineNumber << ", " << colNumber << ")" << std::endl;
            return;
        }
    }
    else {
        ret.varVal = &sym->get(id);
    }

    tokQueue.push(ret);
}

void Lexer::lexNumber() {
    unsigned int lineNumber = in.getLine();
    unsigned int colNumber  = in.getColumn();

    if (!isNumber(in.peek())) {
        // ERROR: invalid number
        std::cerr << "Invalid number ";
        std::cerr << "at (" << lineNumber << ", " << colNumber << ")" << std::endl;
        return;
    }

    int base = 10;
    if (in.peek() == '0') {
        // Check the next symbol for the base

        // Discard the 0
        in.get();

        // If the next char is a number, its not a base specifier
        if (!isNumber(in.peek())) {
            char specifier = in.get();
            switch (specifier) {
                case 'x':
                case 'X':
                    base = 16;
                    break;
                case 'o':
                case 'O':
                    base = 8;
                    break;
                case 'b':
                case 'B':
                    base = 2;
                    break;
                default:
                    std::cerr << "Invalid number: unrecognised base specifier \"" << specifier << "\" ";
                    std::cerr << "at (" << lineNumber << ", " << colNumber << ")" << std::endl;
                    return;
            }
        }
    }

    std::stringstream buff;
    while (isHexNumber(in.peek())) {
        buff << (char) in.get();
    }

    std::string numString;
    buff >> numString;

    unsigned long retVal = std::stoul(numString, 0, base);

    if (retVal > 0xffff) {
        std::cerr << "Number is bigger than 16 bits ";
        std::cerr << "at (" << lineNumber << ", " << colNumber << ")" << std::endl;
        return;
    }

    Token ret;
    ret.type = Token::Type::INTEGER;
    ret.intVal = retVal;

    tokQueue.push(ret);
}
