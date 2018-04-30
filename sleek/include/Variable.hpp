#ifndef LEEK_SLEEK_VARIABLE_H_DEFINED
#define LEEK_SLEEK_VARIABLE_H_DEFINED

class ParseUnit;

class Variable {
    public:
        Variable();

        void use(ParseUnit& pu);
        ParseUnit& getParseUnit();
    private:

        ParseUnit* pu;
};

#endif
