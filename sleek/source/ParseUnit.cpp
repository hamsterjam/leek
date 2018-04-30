#include "ParseUnit.hpp"
#include "Token.hpp"
#include "Variable.hpp"
#include "SymbolTable.hpp"

#include <thread>

void ParseUnit::push(Token val) {
    iBuff.push(val);
}

void ParseUnit::generate() {
    auto routine = [this]() {
        Token next;
        Token peek = iBuff.get();

        Variable* var = NULL;

        // Need to decide what to do with the first few tokens
        if (peek.type == Token::Type::IDENTIFIER) {
            next = peek;
            peek = iBuff.get();

            // This MUST be a definition (so it always has a value)
            if (peek.type == Token::Type::DEFINITION) {
                // It's a definition
                var = &next.symbolVal->getValue();
                var->use(*this);
            }
            else {
                //TODO// It's an overload (will require working types)
            }

            peek = iBuff.get();
        }

        ParseUnit* pipe = NULL;
        unsigned int blockDepth = 0;

        do {
            next = peek;
            peek = iBuff.get();

            if (pipe && (blockDepth == 0 && next.type == Token::Type::END_OF_STATEMENT
                      || blockDepth == 1 && next.type == Token::Type::CLOSING_BLOCK))
            {
                // Condition to end a pipe
                pipe->push(next);

                Token end;
                end.type = Token::Type::END_OF_FILE;
                pipe->push(end);

                pipe = NULL;
                blockDepth = 0;
                continue;
            }

            if (!pipe && (peek.type == Token::Type::DEFINITION
                       || peek.type == Token::Type::OVERLOAD))
            {
                // Condition to start a pipe
                children.emplace_back();
                pipe = &children.back();
                pipe->generate();
            }

            if (pipe) {
                if (next.type == Token::Type::OPENING_BLOCK) ++blockDepth;
                if (next.type == Token::Type::CLOSING_BLOCK) --blockDepth;

                pipe->push(next);
            }
            else {
                // Just discard for now
            }
        } while (peek.type != Token::Type::END_OF_FILE);

        // Join all the children after everything is done to prevent it from
        // pausing unessecairaly
        for (auto& c : children) c.join();
    };

    th = std::thread(routine);
}

void ParseUnit::join() {
    th.join();
}
