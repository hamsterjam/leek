#ifndef LEEK_SLEEK_SYMBOL_TABLE_H_DEFINED
#define LEEK_SLEEK_SYMBOL_TABLE_H_DEFINED

#include "Variable.hpp"

#include <string>
#include <set>
#include <map>

class Symbol;
class SymbolTable;

class Symbol {
    public:
        Symbol();
        Symbol(bool isDefinition);
        ~Symbol();

        void aliasTo(Symbol& val);
        bool isDefinition();
        Variable& getValue();
        SymbolTable* getScope();

    private:
        bool definition;
        bool ownsScope;
        Variable* value;
        SymbolTable* scope;
};

class SymbolTable {
    public:
        SymbolTable();
        ~SymbolTable();

        SymbolTable* newScope();
        SymbolTable* exitScope();
        SymbolTable* getRoot();

        Symbol& get(std::string& key);
        Symbol& define(std::string& key);

        bool isFunctionExpression;
        bool isFunctionExpressionCT;

        bool isClassScope;

    private:
        friend Symbol;

        SymbolTable* root;
        SymbolTable* parent;
        std::set<SymbolTable*> children;
        std::map<std::string, Symbol> data;

        Symbol* getRaw(std::string& key);
        void aliasAllForwardRefs(std::string& key, Symbol& val);
};

#endif
