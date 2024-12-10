#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << "#ARITHM!";
}

namespace {
class Formula : public FormulaInterface {
public:
    // Реализуйте следующие методы:
    explicit Formula(std::string expression) try
        : ast_(ParseFormulaAST(expression)){
    } catch (...) {
        throw FormulaException("");
    }
    Value Evaluate(const SheetInterface& sheet) const override {
        Value result;

        try {
            result = ast_.Execute(sheet);
        } catch (const FormulaError &err) {
            return err;
        }

        return result;
    }
    std::string GetExpression() const override{
        std::stringstream ss;
        ast_.FormulaAST::PrintFormula(ss);

        return ss.str();
    }

    std::vector<Position> GetReferencedCells() const override{
        auto list = ast_.GetCells();
        std::set<Position> set(list.begin(), list.end());
        return std::vector<Position>(set.begin(), set.end());
    }

private:
    FormulaAST ast_;
};
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    return std::make_unique<Formula>(std::move(expression));
}
