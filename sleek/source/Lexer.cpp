#include "Lexer.hpp"
#include "SymbolTable.hpp"
#include "Error.hpp"
#include "helper.hpp"

#include <queue>
#include <stdexcept>
#include <sstream>
#include <ostream>
#include <string>
#include <iostream>

#include <cstring>

Lexer::Lexer(const char* filename, SymbolTable& sym) :
    in(filename),
    out(std::cerr)
{
    this->sym = &sym;
    scopeLevel = 0;

    lexingParamList = false;
    lexingArgList   = false;

    errors = 0;

    lexWhitespace();
}

Lexer::Lexer(std::string& in, SymbolTable& sym, std::ostream& err) :
    in(in),
    out(err)
{
    this->sym = &sym;
    scopeLevel = 0;

    lexingParamList = false;
    lexingArgList   = false;

    errors = 0;

    lexWhitespace();
}

Lexer::Lexer(std::string&& in, SymbolTable& sym, std::ostream& err) :
    in(std::forward<std::string&&>(in)),
    out(err)
{
    this->sym = &sym;
    scopeLevel = 0;

    lexingParamList = false;
    lexingArgList   = false;

    errors = 0;

    lexWhitespace();
}

Token Lexer::peek() {
    lexSomeTokens();
    return tokQueue.front();
}

Token Lexer::get() {
    lexSomeTokens();
    Token ret = tokQueue.front();
    tokQueue.pop();
    return ret;
}

void Lexer::operator>>(Token& out) {
    out = get();
}

unsigned int Lexer::errorCount() {
    return errors;
}

void Lexer::lexAll() {
    do {
        try {
            lexStatement();
        }
        catch (Error e) {
            errors += 1;
            out << "LEX ERROR (" << errors << "): ";
            e.print(out);
            lexingArgList = false;
            lexingParamList = false;
            in.clearBuffer();
        }
    } while (tokQueue.back().type != Token::Type::END_OF_FILE);
}

void Lexer::lexSomeTokens() {
    while (tokQueue.size() < 1) {
        try {
            lexStatement();
        }
        catch (Error e) {
            errors += 1;
            out << "LEX ERROR (" << errors << "): ";
            e.print(out);
            lexingArgList = false;
            lexingParamList = false;
            in.clearBuffer();
        }
        lexWhitespace();
    }
}

/*
 * Recursive Lexers
 */

void Lexer::lexStatement() {
    lexRawStatement();
    if (tokQueue.back().type == Token::Type::END_OF_FILE) return;
    lexPostStatement();
}

void Lexer::lexRawStatement() {
    if (sym->isClassScope) {
        lexClassStatement();
    }
    else {
        lexRegularStatement();
    }
}

void Lexer::lexClassStatement() {
    // Similar to a regular statement, but we only allow definitions (and we
    // have to support access specifiers)

    int peek = in.peek();
    if (isLetter(peek)) {
        // It is either a definition or a visibility (and then a definition)
        in.bufferIdentifier();
        std::string id = in.getBufferedIdentifier();
        if (id == "public" || id == "private" || id == "nowrite") {
            lexKeyword();
            lexWhitespace();
        }
        lexDefinition();
        lexWhitespace();
    }
    else if (peek == '~') {
        // These are here incase of an error
        unsigned int line = in.getLine();
        unsigned int col  = in.getColumn();

        // Remove the ~ character, then add it to the end of the id
        in.get();
        in.bufferIdentifier();
        std::string& id = in.getBufferedIdentifier();
        if (id != "this") {
            // ERROR: bad identifier
            std::string msg("Malformed definition, illegal identifier '");
            msg += "~" + id + "'";
            throw std::move(Error(std::move(msg),
                        line, col));
        }
        id = "~" + id;
        lexDefinition();
        lexWhitespace();
    }
    else if (peek == '}') {
        // Finish the class definition
        if (scopeLevel == 0) {
            // ERROR: Exiting global scope
            // (should be unreachable)
            throw std::move(Error("Unexpected '}' character, no matching '{' character",
                        in.getLine(), in.getColumn()));
        }

        // Discard the } character
        in.get();
        lexWhitespace();

        scopeLevel -= 1;
        sym = sym->exitScope();

        Token close;
        close.type = Token::Type::CLOSING_BLOCK;
        tokQueue.push(close);

        // This scope must be a class scope, so lex a post expression
        lexPostExpression();
    }
    else if (peek == ';') {
        // Empty statement
        in.get();
        lexWhitespace();
    }
    else {
        // ERROR: Unexpected character
        unsigned int line = in.getLine();
        unsigned int col  = in.getColumn();

        // Remove unexpected char from stream so we don't inifnite loop
        std::string message("Malformed class statement, unexpected '");
        message += (char) in.get();
        message += "' character";

        lexWhitespace();

        throw std::move(Error(std::move(message),
                    line, col));
    }
}

void Lexer::lexRegularStatement() {
    // Can't be a char because eof isnt a char
    int peek = in.peek();
    if (isLetter(peek)) {
        in.bufferIdentifier();
        lexWhitespace();

        if (in.peek() == ':') {
            // We have a definition
            lexDefinition();
            lexWhitespace();
        }
        else {
            std::string id = in.getBufferedIdentifier();
            if (id == "break" || id == "pass") {
                // These are expected to be the entire statement
                lexKeyword();
                lexWhitespace();
            }
            else if (id == "defer" || id == "return") {
                // Still an expression, but we need to push a keyword token
                lexKeyword();
                lexExpression();
                lexWhitespace();
            }
            else if (id == "static") {
                // Expect a definition, don't forget the keyword token though
                lexKeyword();
                lexDefinition();
                lexWhitespace();
            }
            else if (id == "if" || id == "elif" || id == "while") {
                // Expect an expression (for the condition) and a statement
                lexKeyword();
                lexWhitespace();
                lexExpression();
                lexWhitespace();
                // Specifically disallow a closing block here
                if (in.peek() == '}') {
                    // ERROR
                    throw std::move(Error("Unexpected '}' character, expected a statement",
                                in.getLine(), in.getColumn()));
                }
                lexRawStatement();
                lexWhitespace();
            }
            else if (id == "do" || id == "else") {
                // Expect a statement for the body, the condition will be lexed
                // on a later call to lexStatement
                lexKeyword();
                lexWhitespace();
                // Specifically disallow a closing block here
                if (in.peek() == '}') {
                    // ERROR
                    throw std::move(Error("Unexpected '}' character, expected a statement",
                                in.getLine(), in.getColumn()));
                }
                lexRawStatement();
                lexWhitespace();
            }
            else if (id == "for") {
                // Expect 3 statements seperated by semicolons (ALWAYS) in
                // parens followed by a body statement. For now, don't allow
                // block statements inside the parens
                lexKeyword();
                lexWhitespace();

                if (in.peek() != '(') {
                    // ERROR: No for loop head
                    throw std::move(Error("Malformed 'for' statement, expected '(' character at",
                                in.getLine(), in.getColumn()));
                }

                // Discard the ( character
                in.get();
                lexWhitespace();

                // Statements in the head are part of the same scope as the body
                sym = sym->newScope();
                scopeLevel += 1;

                for (int i = 0; i < 3; ++i) {
                    // Disallow closing blocks
                    if (in.peek() == '}') {
                        // ERROR
                        throw std::move(Error("Unexpected '}' character, expected a statement",
                                    in.getLine(), in.getColumn()));
                    }

                    lexRawStatement();

                    // Disallow any statements that open a scope
                    if (tokQueue.back().type == Token::Type::OPENING_BLOCK) {
                        // ERROR: no opening blocks in if statements
                        throw std::move(Error("Statments that open new scopes are forbidden in for statements,",
                                    in.getLine(), in.getColumn()));
                    }

                    lexWhitespace();

                    // We don't want a semicolon on the last loop
                    if (i >= 2) break;

                    if (in.peek() != ';') {
                        // ERROR: no semicolon
                        throw std::move(Error("Expected ';' character",
                                    in.getLine(), in.getColumn()));
                    }

                    // Discard the ; character
                    in.get();
                    lexWhitespace();

                    Token sep;
                    sep.type = Token::Type::END_OF_STATEMENT;
                    tokQueue.push(sep);
                }

                // Make sure we have a closing paren
                if (in.peek() != ')') {
                    // ERROR, unclosed for head
                    throw std::move(Error("Malformed for statement, expected ')' character",
                                in.getLine(), in.getColumn()));
                }

                // Discard the ) character
                in.get();
                lexWhitespace();

                // Last we need a statement for the body
                // Disallow clossing blocks
                if (in.peek() == '}') {
                    // ERROR
                    throw std::move(Error("Unexpected '}' character, expected a statement",
                                in.getLine(), in.getColumn()));
                }
                lexRawStatement();

                // We need to exit a scope here regardless of what the previous
                // statement was.
                scopeLevel -= 1;
                sym = sym->exitScope();
            }
            else {
                // It's just an expression
                lexExpression();
                lexWhitespace();
            }
        }
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
            throw std::move(Error("Unexpected '}' character, no matching '{' character",
                        in.getLine(), in.getColumn()));
        }
        // Discard the } character
        in.get();
        lexWhitespace();

        bool wasFunction = sym->isFunctionExpression || sym->isFunctionExpressionCT;
        bool wasClass    = sym->isClassScope;

        scopeLevel -= 1;
        sym = sym->exitScope();

        Token close;
        close.type = Token::Type::CLOSING_BLOCK;
        tokQueue.push(close);

        if (wasFunction || wasClass) {
            lexPostExpression();
        }
    }
    else if (peek == ';') {
        // Empty statement
    }
    else if (peek == FileTracker::eof()) {
        Token eof;
        eof.type = Token::Type::END_OF_FILE;
        tokQueue.push(eof);
    }
    else {
        // Just treat it as an expression
        lexExpression();
        lexWhitespace();
    }
}

void Lexer::lexPostStatement() {
    // If the last token from the statement wasn't a { or a }, we require a ;
    Token::Type lastType = tokQueue.back().type;
    if (lastType != Token::Type::OPENING_BLOCK && lastType != Token::Type::CLOSING_BLOCK) {
        if (in.peek() != ';') {
            // ERROR: Missing semicolon
            throw std::move(Error("Expected ';' character",
                        in.getLine(), in.getColumn()));
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
        else if (id.type == Token::Type::KEYWORD) {
            // Some keywords have special properties
            if (!strncmp(id.stringVal, "new", 8)) {
                // Just lex an expression from here
                lexExpression();
                return;
            }
            else if (!strncmp(id.stringVal, "class", 8)) {
                // Expect an opening block
                if (in.peek() != '{') {
                    // ERROR: no opening block
                    throw std::move(Error("Missing class definition, expected '{' character",
                                in.getLine(), in.getColumn()));
                }

                // Discard the { character
                in.get();
                lexWhitespace();

                Token open;
                open.type = Token::Type::OPENING_BLOCK;
                tokQueue.push(open);

                // Increase the scope
                sym = sym->newScope();
                sym->isClassScope = true;
                scopeLevel += 1;

                // Stop here
                return;
            }
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
        else if (in.peek() == ')') {
            // Must be an opening param list
            isFunction = true;
        }

        if (isFunction) {
            // This is a paramater list of a function
            lexVoidFunctionExpression(false);

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
                throw std::move(Error("Unclosed parentheses, expected ')' character",
                            in.getLine(), in.getColumn()));
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
        lexVoidFunctionExpression(true);

        // Stop here, next call to lexStatement will lex more tokens
        return;
    }

    else {
        // It's not an error if we are parsing a function expression!
        if (lexingParamList && (peek == ')' || peek == ',')) {
            // Just stop
            return;
        }
        // ERROR: Unexpectd char
        unsigned int line = in.getLine();
        unsigned int col  = in.getColumn();

        // Use get so the unexpected character is removed from the stream
        // (so that it doesnt immediatly come back to this error)
        std::string message("Malformed expression, unexpected '");
        message += (char) in.get();
        message += "' character";

        lexWhitespace();

        throw std::move(Error(std::move(message),
                    line, col));
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
            else if (in.peek() == '>') {
                // Could either be an empty param list or an empty arg list
                // For now, discard the > character.
                in.get();
                lexWhitespace();

                if (in.peek() == '{') {
                    // It was an empty param list

                    /*
                     * Note that this will get called if the previously lexed
                     * expression was a function expression (as if the type of
                     * this function is the previous function). Although that
                     * is an incorrect interpretation, it still produces the
                     * correct token stream so all is fine
                     */

                    unknown.type = Token::Type::OPENING_PARAM_LIST_CT;

                    Token close;
                    close.type = Token::Type::CLOSING_PARAM_LIST_CT;
                    tokQueue.push(close);

                    // Increase the scope
                    sym = sym->newScope();
                    sym->isFunctionExpression = true;
                    scopeLevel += 1;

                    lexFunctionExpressionFromBlock(true);

                    // For now, stop
                    return;
                }
                else {
                    // It was an arg list
                    unknown.type = Token::Type::OPENING_ARG_LIST_CT;

                    Token close;
                    close.type = Token::Type::CLOSING_ARG_LIST_CT;
                    tokQueue.push(close);

                    // Lex another postExpression and stop
                    lexPostExpression();
                    return;
                }
            }

            if (isFunctionExp) {
                unknown.type = Token::Type::OPENING_PARAM_LIST_CT;

                lexFunctionExpressionFromList(true);
            }
            else if (in.peek() == '>' && !in.isBuffered()) {
                // Then it's an empty argument list
                isFunctionCall = true;
            }
            else if (in.peek() == '>' && in.isBuffered()) {
                // If we just let this lex as an expression, the > character
                // would become an operator and break everything

                // lex the buffered identifier
                lexIdentifier(false);
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
                        out << "Missing '>' character ";
                        out << "at (" << in.getLine() << ", " << in.getColumn() << ")" << std::endl;
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
    else if (peek == '[' && tokQueue.back().type != Token::Type::CLOSING_BLOCK) {
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
            throw std::move(Error("Unclosed indexing expression, expected ']' character",
                        in.getLine(), in.getColumn()));
        }

        // Discard the ] character
        in.get();
        lexWhitespace();

        Token close;
        close.type = Token::Type::CLOSING_INDEX_BRACKET;
        tokQueue.push(close);

        lexPostExpression();
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
        else if (in.peek() == ')') {
            // It's either an opening param list or an opening arg list
            // For now, discard that symbol as well
            in.get();
            lexWhitespace();

            if (in.peek() == '{') {
                // It was an empty param list

                /*
                 * Note that this will get called if the previously lexed
                 * expression was a function expression (as if the type of
                 * this function is the previous function). Although that
                 * is an incorrect interpretation, it still produces the
                 * correct token stream so all is fine
                 */

                Token open;
                open.type = Token::Type::OPENING_PARAM_LIST;
                tokQueue.push(open);

                Token close;
                close.type = Token::Type::CLOSING_PARAM_LIST;
                tokQueue.push(close);

                // Increase the scope
                sym = sym->newScope();
                sym->isFunctionExpression = true;
                scopeLevel += 1;

                lexFunctionExpressionFromBlock(false);

                // Stop here
                return;
            }
            else {
                // It was an empty arg list
                Token open;
                open.type = Token::Type::OPENING_ARG_LIST;
                tokQueue.push(open);

                Token close;
                close.type = Token::Type::CLOSING_ARG_LIST;
                tokQueue.push(close);

                // lex the post expression stuff again and then stop
                lexPostExpression();
                return;
            }
        }

        if (isExpression) {
            // It's a function expression
            lexFunctionExpression(false);
        }
        else {
            // It's a function call
            Token open;
            open.type = Token::Type::OPENING_ARG_LIST;
            tokQueue.push(open);

            lexArgList();

            if (in.peek() != ')') {
                // ERROR: missing closing paren
                throw std::move(Error("Unclosed arg list, expected ')' character",
                            in.getLine(), in.getColumn()));
            }

            // Discard the ) character
            in.get();
            lexWhitespace();

            Token close;
            close.type = Token::Type::CLOSING_ARG_LIST;
            tokQueue.push(close);

            lexPostExpression();
        }
    }
    // else do nothing. We allow an empty post expression
}

void Lexer::lexDefinition() {
    // In the case of an overload operator, this identifier isnt actually
    // lexed as a definition
    if (!in.isBuffered()) in.bufferIdentifier();
    lexWhitespace();

    if (in.peek() != ':') {
        // ERROR: Missing colon
        throw std::move(Error("Malformed definition, expected ':' character",
                    in.getLine(), in.getColumn()));
    }

    // Discard the : character. Don't touch that whitespace yo
    in.get();

    if (in.peek() == ':') {
        // Lex the buffered identifier (not as a definition)
        lexIdentifier(false);

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
        // Lex the buffered identifier (as a definition)
        lexIdentifier(true);
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
    //TODO// Make this non-recursive

    if ((in.peek() == '>' || in.peek() == ')') && !in.isBuffered()) {
        // Empty arg list, just immediatly end
        return;
    }
    else if (in.peek() == ',' && !in.isBuffered()) {
        // Skipped argument
        Token sep;
        sep.type = Token::Type::COMMA;
        tokQueue.push(sep);

        in.get();
        lexWhitespace();
        lexArgList();
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
 * Function Expression Lexers
 */

void Lexer::lexVoidFunctionExpression(bool compileTime) {
    Token retType;
    retType.type = Token::Type::KEYWORD;
    strncpy(retType.stringVal, "void", 8);
    tokQueue.push(retType);

    lexFunctionExpression(compileTime);
}

void Lexer::lexFunctionExpression(bool compileTime) {
    // Assume it is a function expression but the opening char may or may not
    // have been discarded
    char openChar = (compileTime) ? '<' : '(';
    if (in.peek() == openChar) {
        in.get();
        lexWhitespace();
    }

    Token open;
    if (compileTime) open.type = Token::Type::OPENING_PARAM_LIST_CT;
    else             open.type = Token::Type::OPENING_PARAM_LIST;
    tokQueue.push(open);

    lexFunctionExpressionFromList(compileTime);
}

void Lexer::lexFunctionExpressionFromList(bool compileTime) {
    // Enter a new scope
    sym = sym->newScope();
    if (compileTime) sym->isFunctionExpressionCT = true;
    else             sym->isFunctionExpression   = true;
    scopeLevel += 1;

    lexParamList();
    lexWhitespace();

    char closeChar = (compileTime) ? '>' : ')';
    Token::Type closeType;
    if (compileTime) closeType = Token::Type::CLOSING_PARAM_LIST_CT;
    else             closeType = Token::Type::CLOSING_PARAM_LIST;

    if (tokQueue.back().type != closeType) {
        if (in.peek() != closeChar) {
            // ERROR: missing closing bracket
            std::string message("Unclosed param list, expected '");
            message += closeChar;
            message += "' character";

            throw std::move(Error(std::move(message),
                        in.getLine(), in.getColumn()));
        }

        // Discard the closeChar character
        in.get();
        lexWhitespace();

        Token close;
        close.type = closeType;
        tokQueue.push(close);
    }

    lexFunctionExpressionFromBlock(compileTime);
}

void Lexer::lexFunctionExpressionFromBlock(bool compileTime) {
    if (in.peek() != '{') {
        // ERROR: No function definition
        throw std::move(Error("Missing function definition, expected '{' character",
                    in.getLine(), in.getColumn()));
    }

    // Discard the { character
    in.get();
    lexWhitespace();

    Token openBlock;
    openBlock.type = Token::Type::OPENING_BLOCK;
    tokQueue.push(openBlock);
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
            throw std::move(Error("Invalid unary operator",
                        lineNumber, colNumber));
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
            throw std::move(Error("Invalid binary operator",
                        lineNumber, colNumber));
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

    // Handle Keywords
    // "this" is handled seperatley (since it is kind of a variables)
    if (isKeyword(id)) {
        lexKeyword();
        return;
    }
    if (isKeywordOperator(id)) {
        lexKeywordOperator();
        return;
    }

    if (id == "op") {
        // Operator identifier
        while (isOperatorChar(in.peek())) {
            id += in.get();
        }
    }

    Token ret;
    ret.type = Token::Type::IDENTIFIER;

    if (definition) {
        try {
            ret.varVal = &sym->define(id);
        }
        catch (std::out_of_range e) {
            // ERROR: Variable already exists
            std::string message("Variable \"");
            message += id;
            message += "\" redefined";

            throw std::move(Error(std::move(message),
                        lineNumber, colNumber));
        }
    }
    else {
        ret.varVal = &sym->get(id);
    }

    tokQueue.push(ret);
    in.clearBuffer();

    if (in.peek() == '.') {
        // This is a property acces
        Token prop;
        prop.type = Token::Type::PROPERTY_ACCESS;
        tokQueue.push(prop);

        // Discard the . character
        in.get();

        SymbolTable* currSym = sym;
        sym = ret.varVal->getScope();

        lexIdentifier(false);

        sym = currSym;
    }
}

void Lexer::lexKeyword() {
    if (!in.isBuffered()) in.bufferIdentifier();

    unsigned int lineNumber = in.getBufferLine();
    unsigned int colNumber  = in.getBufferColumn();

    Token ret;
    ret.type = Token::Type::KEYWORD;
    strncpy(ret.stringVal, in.getBufferedIdentifier().c_str(), 8);
    tokQueue.push(ret);

    in.clearBuffer();
}

void Lexer::lexKeywordOperator() {
    lexKeyword();
    tokQueue.back().type = Token::Type::UNARY_OPERATOR;
}

void Lexer::lexNumber() {
    unsigned int lineNumber = in.getLine();
    unsigned int colNumber  = in.getColumn();

    // Assume that it is a number
    /*
    if (!isNumber(in.peek())) {
        // ERROR: invalid number
        throw std::move(Error("Invalid number",
                    lineNumber, colNumber));
    }
    */

    int base = 10;
    std::stringstream buff;

    if (in.peek() == '0') {
        // Check the next symbol for the base

        // Discard the 0
        in.get();

        // If the next char is a number, its not a base specifier
        if (!isNumber(in.peek())) {
            char specifier = in.peek();
            switch (specifier) {
                case 'x':
                case 'X':
                    base = 16;
                    in.get();
                    break;
                case 'c':
                case 'C':
                    base = 8;
                    in.get();
                    break;
                case 'b':
                case 'B':
                    base = 2;
                    in.get();
                    break;
                default:
                    // Assume it was just a 0
                    buff << '0';
            }
        }
    }

    while (isHexNumber(in.peek())) {
        buff << (char) in.get();
    }

    std::string numString;
    buff >> numString;

    unsigned long retVal = std::stoul(numString, 0, base);

    if (retVal > 0xffff) {
        // ERROR number too big
        throw std::move(Error("Number is bigger than 16 bits",
                    lineNumber, colNumber));
    }

    Token ret;
    ret.type = Token::Type::INTEGER;
    ret.intVal = retVal;

    tokQueue.push(ret);
}
