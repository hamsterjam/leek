#ifndef LEEK_SLEEK_TYPE_H_DEFINED
#define LEEK_SLEEK_TYPE_H_DEFINED

#include <vector>

class Type {
    public:
        enum PrimitiveType {
            VOID,
            INT
        };

        Type();
        Type(PrimitiveType prim);

        static Type referenceType(Type modified);
        static Type constType(Type modified);
        static Type functionType(bool compileTime, std::vector<Type>&  signature);
        static Type functionType(bool compileTime, std::vector<Type>&& signature);

    private:
        friend bool operator==(const Type&, const Type&);

        enum Category {
            UNDEFINED,
            PRIMITIVE,
            FUNCTION,
            CT_FUNCTION,
            REFERENCE,
            CONST
        };

        Category category;

        PrimitiveType prim;
        std::vector<Type> signature;
};

bool operator==(const Type& lhs, const Type& rhs);
bool operator!=(const Type& lhs, const Type& rhs);

#endif
