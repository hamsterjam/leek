#ifndef LEEK_SLEEK_SYMBOL_TABLE_H_DEFINED
#define LEEK_SLEEK_SYMBOL_TABLE_H_DEFINED

#include "Variable.hpp"

#include <string>
#include <set>
#include <map>

class SymbolTable {
    public:
        SymbolTable();
        ~SymbolTable();

        SymbolTable* newScope();
        SymbolTable* exitScope();
        SymbolTable* getRoot();

        bool exists(std::string key);

        Variable& get(std::string key);
        Variable& define(std::string key);

        bool isFunctionExpression;
        bool isFunctionExpressionCT;

    private:
        SymbolTable* root;
        SymbolTable* parent;
        std::set<SymbolTable*> children;
        std::map<std::string, Variable> data;

        Variable* getPointer(std::string key);

};

#endif
