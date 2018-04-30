#include "Variable.hpp"
#include "ParseUnit.hpp"

#include <stdexcept>

Variable::Variable() {
    pu = NULL;
}

void Variable::use(ParseUnit& pu) {
    if (this->pu) throw std::invalid_argument("Variable::use(ParseUnit&)");

    this->pu = &pu;
}

ParseUnit& Variable::getParseUnit() {
    return *pu;
}
