#include "SymbolTable.hpp"
#include "Variable.hpp"

#include <string>
#include <stdexcept>
#include <utility>
#include <set>
#include <map>

Symbol::Symbol() : Symbol(false){
}

Symbol::Symbol(bool isDefinition) {
    this->definition = isDefinition;
    scope = NULL;

    if (isDefinition) {
        value = new Variable;
    }
    else {
        value = NULL;
    }
}

Symbol::~Symbol() {
    if (definition) {
        delete value;
    }

    if (scope) {
        delete scope;
    }
}

void Symbol::aliasTo(Symbol val) {
    // This only makes sense if val is a definition and this is not
    if (definition || !val.definition)
        return;

    // First we need to alias the symbol scopes

    // If only one variable has a scope, use that scope
    if (this->scope && !val.scope) {
        this->scope = val.scope;
        val.scope = NULL;
    }
    // If they both have a scope, take a union
    else if (this->scope && val.scope) {
        for (auto p : val.scope->data) {
            std::string key = p.first;
            Symbol      sym = p.second;

            // If this symbol exists in our scope as well, alias those symbols
            if (this->scope->data.count(key)) {
                this->scope->data[key].aliasTo(sym);
            }
            // Otherwise create a (non deifnition) symbol in this scope
            else {
                Symbol dummy(false);
                this->scope->data[key] = dummy;
            }
        }
    }


    this->value = val.value;
}

bool Symbol::isDefinition() {
    return definition;
}

Variable& Symbol::getValue() {
    return *value;
}

SymbolTable* Symbol::getScope() {
    if (!scope) {
        scope = new SymbolTable;
    }

    return scope;
}

SymbolTable::SymbolTable() {
    root   = this;
    parent = NULL;

    isFunctionExpression   = false;
    isFunctionExpressionCT = false;
}

SymbolTable::~SymbolTable() {
    for (SymbolTable* child : children) {
        delete child;
    }
}

SymbolTable* SymbolTable::newScope() {
    SymbolTable* child = new SymbolTable();
    child->parent = this;
    children.insert(child);
    return child;
}

SymbolTable* SymbolTable::exitScope() {
    return parent;
}

SymbolTable* SymbolTable::getRoot() {
    return root;
}

Symbol* SymbolTable::getRaw(std::string& key) {
    // If a sybol with that name exists in this scope, return it
    if (data.count(key)) {
        return &data[key];
    }

    // Else, check the parent scope
    if (parent) {
        return parent->getRaw(key);
    }

    // If no parent scope, then the variable does not exist
    return NULL;
}

Symbol& SymbolTable::get(std::string& key) {
    Symbol* ret = getRaw(key);

    if (!ret) {
        // Symbol does not exist in this scope, make a new one
        data[key] = Symbol(false);
        ret = &data[key];
    }

    return *ret;
}

Symbol& SymbolTable::define(std::string& key) {
    // Check this scope and all parent scopes for a variable of the same name
    SymbolTable* curr = this;
    while (curr) {
        if (data.count(key)) {
            throw std::out_of_range("SymbolTable::define");
        }
        curr = curr->parent;
    }

    // Define the new symbol
    Symbol& ret = data[key];
    ret = Symbol(true);

    // Alias all variables in children scopes to ret (link forward references)
    aliasAllForwardRefs(key, ret);

    return data[key];
}

void SymbolTable::aliasAllForwardRefs(std::string& key, Symbol& val) {
    if (data.count(key)) {
        data[key].aliasTo(val);
    }

    for (auto child : children) {
        child->aliasAllForwardRefs(key, val);
    }
}
