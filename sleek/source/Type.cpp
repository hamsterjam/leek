#include "Type.hpp"

#include <vector>

Type::Type() {
    this->category = UNDEFINED;
}

Type::Type(PrimitiveType prim) {
    this->category = PRIMITIVE;
    this->prim     = prim;
}

Type Type::referenceType(Type modified) {
    Type ret;
    ret.category  = REFERENCE;
    ret.signature = std::vector<Type>();
    ret.signature.push_back(modified);

    return ret;
}

Type Type::constType(Type modified) {
    Type ret;
    ret.category  = CONST;
    ret.signature = {modified};
    ret.signature = std::vector<Type>();
    ret.signature.push_back(modified);

    return ret;
}

Type Type::functionType(bool compileTime, std::vector<Type>& signature) {
    Type ret;
    ret.category  = compileTime ? CT_FUNCTION : FUNCTION;
    ret.signature = signature;

    return ret;
}

Type Type::functionType(bool compileTime, std::vector<Type>&& signature) {
    Type ret;
    ret.category  = compileTime ? CT_FUNCTION : FUNCTION;
    ret.signature = std::move(signature);

    return ret;
}

bool operator==(const Type& lhs, const Type& rhs) {
    if (lhs.category != rhs.category)    return false;
    if (lhs.category == Type::UNDEFINED) return false;
    if (lhs.category == Type::PRIMITIVE) return lhs.prim == rhs.prim;

    // else
    // This might actually not work 100% of the time depending on how == works
    // for vectors, it *should* be fine though...
    return lhs.signature == rhs.signature;
}

bool operator!=(const Type& lhs, const Type& rhs) { return !(lhs == rhs);}
