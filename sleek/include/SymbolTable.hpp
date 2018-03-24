#ifndef LEEK_SLEEK_SYMBOL_TABLE_H_DEFINED
#define LEEK_SLEEK_SYMBOL_TABLE_H_DEFINED

#include "Variable.hpp"

#include <string>
#include <map>

class SymbolTable {
    public:
        SymbolTable();
        ~SymbolTable();

        SymbolTable* enterScope();
        SymbolTable* exitScope();

        bool exists(std::string key);

        Variable& get(std::string key);
        Variable& define(std::string key);

    private:
        SymbolTable* parent;
        SymbolTable* child;

        Variable* getPointer(std::string key);

        std::map<std::string, Variable> data;

};

#endif
