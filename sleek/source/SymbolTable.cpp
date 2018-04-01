#include "SymbolTable.hpp"
#include "Variable.hpp"

#include <string>
#include <stdexcept>
#include <set>
#include <map>

Symbol::Symbol() : Symbol(false){
}

Symbol::Symbol(bool isDefinition) {
    this->definition = isDefinition;

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
}

void Symbol::aliasTo(Symbol val) {
    // This only makes sense if val is a definition and this is not
    if (definition || !val.definition)
        return;

    this->value = val.value;
}

bool Symbol::isDefinition() {
    return definition;
}

Variable& Symbol::getValue() {
    return *value;
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
