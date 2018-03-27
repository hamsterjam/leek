#include "SymbolTable.hpp"
#include "Variable.hpp"

#include <string>
#include <stdexcept>
#include <set>
#include <map>

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

Variable& SymbolTable::get(std::string key) {
    Variable* ret = getPointer(key);

    if (!ret) {
        ret = &define(key);
        ret->defined = false;
    }

    return *ret;
}

Variable& SymbolTable::define(std::string key) {
    if (data.count(key)) {
        throw std::out_of_range("SymbolTable::define");
    }

    Variable& ret = data[key];
    ret.defined = true;
}

Variable* SymbolTable::getPointer(std::string key) {
    if (data.count(key)) {
        return &data[key];
    }

    if (parent) {
        return parent->getPointer(key);
    }

    return NULL;
}
