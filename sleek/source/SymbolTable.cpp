#include "SymbolTable.hpp"
#include "Variable.hpp"

#include <string>
#include <stdexcept>
#include <map>

SymbolTable::~SymbolTable() {
    SymbolTable* curr = this;

    while (curr->child) curr = curr->child;

    while (curr->parent) {
        curr = curr->parent;
        delete curr->child;
    }
}

SymbolTable* SymbolTable::enterScope() {
    if (!child) {
        child = new SymbolTable();
        this->child->parent = this;

        return child;
    }
    return child;
}

SymbolTable* SymbolTable::exitScope() {
    if (parent) {
        return parent;
    }
    return NULL;
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
