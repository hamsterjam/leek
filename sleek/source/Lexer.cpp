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
    scopeLevel = 0;

    lexingParamList = false;
    lexingArgList   = false;
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

void Lexer::lexStatement() {
    char peek = in.peek();
    if (isLetter(peek)) {
        in.bufferIdentifier();
        lexWhitespace();

        if (in.peek() == ':') {
            // We have a definition
            lexDefinition();
            lexWhitespace();
        }
        else {
            if (in.getBufferedIdentifier() == "defer") {
                // Still an expression, but we need to push a defer token
                in.clearBuffer();
                Token defer;
                defer.type = Token::Type::KEYWORD;
                strncpy(defer.stringVal, "defer", 8);
                tokQueue.push(defer);
            }
            lexExpression();
            lexWhitespace();
        }
        //TODO// Control structures
    }
    else if (peek == '{') {
        // Discard the { character
        in.get();
        lexWhitespace();
        scopeLevel += 1;
        sym = sym->newScope();

        Token open;
        open.type = Token::Type::OPENING_BLOCK;
        tokQueue.push(open);
    }
    else if (peek == '}') {
        if (scopeLevel == 0) {
            // ERROR: Exiting global scope
            std::cerr << "Unexpected '}' character, no matching '{' character ";
            std::cerr << "at (" << in.getLine() << ", " << in.getColumn() << ")" << std::endl;
            return;
        }
        // Discard the } character
        in.get();
        lexWhitespace();

        bool wasFunction = sym->isFunctionExpression || sym->isFunctionExpressionCT;

        scopeLevel -= 1;
        sym = sym->exitScope();

        Token close;
        close.type = Token::Type::CLOSING_BLOCK;
        tokQueue.push(close);

        if (wasFunction) {
            lexPostExpression();
        }
    }
    else if (peek == ';') {
        // Empty statement
    }
    else {
        // Just treat it as an expression
        lexExpression();
        lexWhitespace();
    }

    // If the last token from the statement wasn't a { or a }, we require a ;
    Token::Type lastType = tokQueue.back().type;
    if (lastType != Token::Type::OPENING_BLOCK && lastType != Token::Type::CLOSING_BLOCK) {
        if (in.peek() != ';') {
            // ERROR: Missing semicolon
            std::cerr << "Expected ';' character ";
            std::cerr << "at (" << in.getLine() << ", " << in.getColumn() << ")" << std::endl;
            return;
        }

        // Discard the ; character
        in.get();
        lexWhitespace();

        // Push an end of statement token
        Token eos;
        eos.type = Token::Type::END_OF_STATEMENT;
        tokQueue.push(eos);
    }

}

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

    else if (isUnaryOperatorChar(peek)) {
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

            // Enter a new scope
            sym = sym->newScope();
            sym->isFunctionExpression = true;
            scopeLevel += 1;

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

            if (in.peek() != '{') {
                // ERROR: No function definition
                std::cerr << "Expected function definition after function declaration ";
                std::cerr << "at (" << in.getLine() << ", " << in.getColumn() << ")" << std::endl;
                return;
            }

            // Discard the { character
            in.get();
            lexWhitespace();

            Token openBlock;
            openBlock.type = Token::Type::OPENING_BLOCK;
            tokQueue.push(openBlock);

            // Stop here, next call to lexStatement will lex more tokens
            return;
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
    // The > is interpreted as a binary op, but it could actually be the end
    // of a param or arg list as well
    else if ((lexingParamList || lexingArgList) && tokQueue.back().type == Token::Type::BINARY_OPERATOR
                                                && tokQueue.back().stringVal[0] == '>'
                                                && tokQueue.back().stringVal[1] == 0)
    {
        // The previous token wasn't a bin op
        if (lexingParamList) {
            tokQueue.back().type = Token::Type::CLOSING_PARAM_LIST_CT;
        }
        else {
            tokQueue.back().type = Token::Type::CLOSING_ARG_LIST_CT;
        }
        // In either case, we should not run the postExpression stuff
        return;
    }
    else if (peek == '<') {
        // Has to be a compile time function returning void

        // Discard the < character
        in.get();
        lexWhitespace();

        Token retType;
        retType.type = Token::Type::KEYWORD;
        strncpy(retType.stringVal, "void", 8);
        tokQueue.push(retType);

        Token open;
        open.type = Token::Type::OPENING_PARAM_LIST_CT;
        tokQueue.push(open);

        // Enter a new scope
        sym = sym->newScope();
        sym->isFunctionExpressionCT = true;
        scopeLevel += 1;

        lexParamList();
        lexWhitespace();

        // If we don't already have a closing token, we need one
        if (tokQueue.back().type != Token::Type::CLOSING_PARAM_LIST_CT) {
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

        if (in.peek() != '{') {
            // ERROR: No function definition
            std::cerr << "Expected function definition after function declaration ";
            std::cerr << "at (" << in.getLine() << ", " << in.getColumn() << ")" << std::endl;
            return;
        }

        // Discard the { character
        in.get();
        lexWhitespace();

        Token openBlock;
        openBlock.type = Token::Type::OPENING_BLOCK;
        tokQueue.push(openBlock);

        // Stop here, next call to lexStatement will lex more tokens
        return;
    }

    else {
        // ERROR: Unexpectd char
        std::cerr << "Malformed expression, unexpected character '" << peek << "' ";
        std::cerr << "at (" << in.getLine() << ", " << in.getColumn() << ")" << std::endl;
        return;
    }

    lexPostExpression();
}

void Lexer::lexPostExpression() {
    char peek = in.peek();
    if (isOperatorChar(peek)) {
        // Must be a Binary Operator
        lexBinaryOperator();
        lexWhitespace();
        // If it was a < operation...
        if (tokQueue.back().stringVal[0] == '<' && tokQueue.back().stringVal[1] == 0) {
            // It might be a function call instead of an operator, keep track
            // of the last token so we can change it later
            Token& unknown = tokQueue.back();
            bool isFunctionCall = false;
            bool isFunctionExp  = false;

            // We need to tell if we have an expression or a definition so we
            // can decided between function expression and call
            if (isLetter(in.peek())) {
                in.bufferIdentifier();
                lexWhitespace();
                if (in.peek() == ':') {
                    isFunctionExp = true;
                }
            }

            if (isFunctionExp) {
                unknown.type = Token::Type::OPENING_PARAM_LIST_CT;

                sym = sym->newScope();
                sym->isFunctionExpressionCT = true;
                scopeLevel += 1;

                lexParamList();
                lexWhitespace();

                if (tokQueue.back().type != Token::Type::CLOSING_PARAM_LIST_CT) {
                    if (in.peek() != '>') {
                        // ERROR: unclosed param list
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

                if (in.peek() != '{') {
                    // ERROR: no function definition
                    std::cerr << "Expected function definition after function declaration ";
                    std::cerr << "at (" << in.getLine() << ", " << in.getColumn() << ")" << std::endl;
                    return;
                }

                // Discard the { character
                in.get();
                lexWhitespace();

                Token openBlock;
                openBlock.type = Token::Type::OPENING_BLOCK;
                tokQueue.push(openBlock);
            }
            else if (in.peek() == '>') {
                // Then it's an empty argument list
                isFunctionCall = true;
            }
            else {
                // Lex an expression, we are interested in the character *after* it
                bool prevLexingArgList = lexingArgList;
                lexingArgList = true;
                lexExpression();
                lexWhitespace();
                lexingArgList = prevLexingArgList;

                if (in.peek() == ',') {
                    isFunctionCall = true;

                    // Discard the , character
                    in.get();
                    lexWhitespace();

                    Token sep;
                    sep.type = Token::Type::COMMA;
                    tokQueue.push(sep);

                    // Read the rest of the arg list
                    lexArgList();
                }
                else if (in.peek() == '>') {
                    isFunctionCall = true;
                }
                else if (tokQueue.back().type == Token::Type::CLOSING_ARG_LIST_CT) {
                    isFunctionCall = true;
                }
                // else it is just a regular greater than operator, we don't
                // have to do anything more
            }

            if (isFunctionCall) {
                unknown.type = Token::Type::OPENING_ARG_LIST_CT;

                // Arg list should already be lexed, check if we have a closing
                // token
                if (tokQueue.back().type != Token::Type::CLOSING_ARG_LIST_CT) {
                    if (in.peek() != '>') {
                        std::cerr << "Missing '>' character ";
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
                lexPostExpression();
            }
        }
        else {
            // For other operators there is nothing fancy going on, just lex an
            // expression
            lexExpression();
        }
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
        // Function call or expression

        // Discard the ( character
        in.get();
        lexWhitespace();

        bool isExpression = false;

        if (isLetter(in.peek())) {
            in.bufferIdentifier();
            lexWhitespace();
            if (in.peek() == ':') {
                isExpression = true;
            }
        }

        if (isExpression) {
            // It's a function expression
            Token open;
            open.type = Token::Type::OPENING_PARAM_LIST;
            tokQueue.push(open);

            // Enter a new scope
            sym = sym->newScope();
            sym->isFunctionExpression = true;
            scopeLevel += 1;

            lexParamList();
            lexWhitespace();

            if (in.peek() != ')') {
                // ERROR: unclosed param list
                std::cerr << "Missing '(' character ";
                std::cerr << "at (" << in.getLine() << ", " << in.getColumn() << ")" << std::endl;
                return;
            }

            // Discard the ) character
            in.get();
            lexWhitespace();

            Token close;
            close.type = Token::Type::CLOSING_PARAM_LIST;
            tokQueue.push(close);

            if (in.peek() != '{') {
                // ERROR: no function definition
                std::cerr << "Expected function definition after function declaration ";
                std::cerr << "at (" << in.getLine() << ", " << in.getColumn() << ")" << std::endl;
                return;
            }

            // Discard the { character
            in.get();
            lexWhitespace();

            Token openBlock;
            openBlock.type = Token::Type::OPENING_BLOCK;
            tokQueue.push(openBlock);
        }
        else {
            // It's a function call
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
    }
    // else do nothing. We allow an empty post expression
}

void Lexer::lexDefinition() {
    lexIdentifier(true);
    lexWhitespace();

    if (in.peek() != ':') {
        // ERROR: Missing colon
        std::cerr << "Malformed definition, expected ':' character ";
        std::cerr << "at (" << in.getLine() << ", " << in.getColumn() << ")" << std::endl;
        return;
    }

    // Discard the : character. Don't touch that whitespace yo
    in.get();

    if (in.peek() == ':') {
        Token defOp;
        defOp.type = Token::Type::OVERLOAD;
        tokQueue.push(defOp);

        // Discard the additional ':'
        in.get();
        lexWhitespace();

        // Get the value
        lexExpression();
    }
    else {
        lexWhitespace();

        Token defOp;
        defOp.type = Token::Type::DEFINITION;
        tokQueue.push(defOp);

        // Get the type (unless type is to be infered)
        if (in.peek() != '=') {
            lexExpression();
        }

        if (in.peek() == '=') {
            // Discard the = character
            in.get();
            lexWhitespace();

            Token op;
            op.type = Token::Type::BINARY_OPERATOR;
            strncpy(op.stringVal, "=", 8);
            tokQueue.push(op);

            // Get the value
            lexExpression();
        }
    }
}

void Lexer::lexArgList() {
    if (in.peek() == '>' || in.peek() == ')') {
        // Empty arg list, just immediatly end
        return;
    }

    bool prevLexingArgList = lexingArgList;
    lexingArgList = true;
    lexExpression();

    if (in.peek() == ',') {
        Token sep;
        sep.type = Token::Type::COMMA;
        tokQueue.push(sep);

        // Discard the , character
        in.get();
        lexWhitespace();

        lexArgList();
    }
    lexingArgList = prevLexingArgList;
}

void Lexer::lexParamList() {
    if (in.peek() == '>' || in.peek() == ')') {
        // Empty param list, just immediatly end
        return;
    }
    bool prevLexingParamList = lexingParamList;
    lexingParamList = true;
    lexDefinition();
    if (in.peek() == ',') {
        Token sep;
        sep.type = Token::Type::COMMA;
        tokQueue.push(sep);

        // Discard the , character
        in.get();
        lexWhitespace();

        lexParamList();
    }
    lexingParamList = prevLexingParamList;
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

    // Discard the character
    in.get();
    lexWhitespace();

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
