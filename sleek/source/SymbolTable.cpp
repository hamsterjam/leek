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

    ownsScope = false;
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

    if (scope && ownsScope) {
        delete scope;
    }
}

void Symbol::aliasTo(Symbol& val) {
    // This only makes sense if val is a definition and this is not
    if (definition || !val.definition)
        return;

    // Alias the symbol scopes

    // If the scopes are aliases, we don't need to do anything
    if (this->scope == val.scope) {
    }
    // If val owns a scope, and we don't, just use that scope
    // ownership remains with val
    else if (!this->scope && val.scope) {
        this->scope = val.scope;
    }
    // If we have a scope and val does not, we are going to give it our scope.
    else if (this->scope && !val.scope) {
        val.scope = this->scope;
        val.ownsScope = this->ownsScope;
        this->ownsScope = false;
    }
    // If they both have a scope, take a union
    else if (this->scope && val.scope) {
        for (auto p : this->scope->data) {
            std::string key = p.first;
            Symbol&     sym = p.second;

            // If this symbol exists in vals scope as well, alias those symbols
            if (val.scope->data.count(key)) {
                sym.aliasTo(val.scope->data[key]);
            }
            // Otherwise create a (non deifnition) symbol in vals scope
            else {
                Symbol dummy;
                this->scope->data[key] = dummy;
            }
        }

        // Set our scope to point to the (unioned) scope in val
        if (this->ownsScope) {
            // Note that we can guarantee that the scopes are not aliased
            // this delete isn't deleting the scope in val
            this->ownsScope = false;
            delete this->scope;
        }
        this->scope = val.scope;
    }

    // Alias the values
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
        ownsScope = true;
        scope = new SymbolTable;
    }

    return scope;
}

SymbolTable::SymbolTable() {
    root   = this;
    parent = NULL;

    isFunctionExpression   = false;
    isFunctionExpressionCT = false;

    isClassScope = false;

    // Auto define this to prevent it being redefined
    std::string key("this");
    define(key);
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
        data.emplace(key, false);
        ret = &data[key];
    }

    return *ret;
}

Symbol& SymbolTable::define(std::string& key) {
    // Check if we are redefining this variable. Don't check parent scopes, we
    // allow shadowing of variables
    if (data.count(key) && data[key].isDefinition()) {
        throw std::out_of_range("SymbolTable::define");
    }

    // Define the new symbol
    data.emplace(key, true);
    Symbol& ret = data[key];

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
