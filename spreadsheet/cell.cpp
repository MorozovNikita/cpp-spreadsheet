#include "cell.h"

#include <string>
#include <unordered_set>

#include "sheet.h"

class EmptyImpl : public Impl{
public:
    EmptyImpl() = default;

    const std::string GetText() const override{
        return "";
    }

    const CellInterface::Value GetValue() const override{
        return "";
    }

    const std::vector<Position> GetReferencedCells() const override{
        return {};
    }

    const std::optional<FormulaInterface::Value> GetCache() const override{
        return std::nullopt;
    }

    void ResetCache() override{
    }
};

class TextImpl : public Impl{
public:
    TextImpl(const std::string &text) : text_(text){
    }

    const std::string GetText() const override{
        return text_;
    }

    const CellInterface::Value GetValue() const override{
        if(*text_.begin() == '\'')
            return std::string(++text_.begin(), text_.end());
        return text_;
    }

    const std::vector<Position> GetReferencedCells() const override{
        return {};
    }

    const std::optional<FormulaInterface::Value> GetCache() const override{
        return std::nullopt;
    }

    void ResetCache() override{
    }
private:
    std::string text_;
};

class FormulaImpl : public Impl{
public:
    explicit FormulaImpl(std::string text, const SheetInterface &sheet)
        : formula_(ParseFormula(text))
        , sheet_(sheet)
    {
    }

    const std::string GetText() const override{
        return '=' + formula_->GetExpression();
    }

    const CellInterface::Value GetValue() const override{
        if (!cache_.has_value()) {
            cache_ = formula_->Evaluate(sheet_);
        }

        switch (cache_.value().index()) {
        case 0:
            return std::get<0>(cache_.value());
        case 1:
            return std::get<1>(cache_.value());
        default:
            throw FormulaError(FormulaError::Category::Value);
        }
    }

    const std::vector<Position> GetReferencedCells() const override{
        return formula_->GetReferencedCells();
    }

    const std::optional<FormulaInterface::Value> GetCache() const override{
        return cache_;
    }

    void ResetCache() override{
        cache_.reset();
    }
private:
    std::unique_ptr<FormulaInterface> formula_;
    const SheetInterface &sheet_;
    mutable std::optional<FormulaInterface::Value> cache_;
};

Cell::Cell(Sheet &sheet) :
    sheet_(sheet),
    impl_(std::make_unique<EmptyImpl>())
{
}

void Cell::Set(std::string text) {
    if(text.empty()){
        whoIReffering_.clear();
        impl_ = std::make_unique<EmptyImpl>();
    }else if(text.front() == '=' && text.size() > 1){
        std::unique_ptr<Impl> newImpl = std::make_unique<FormulaImpl>(FormulaImpl(std::string(++text.begin(), text.end()), sheet_));
        const auto &newImplRefs = newImpl->GetReferencedCells();

        IsCircular(newImplRefs);

        if (!newImplRefs.empty()) {
            whoIReffering_.clear();

            for(auto cell : whoRefferingToMe_){
                cell->whoRefferingToMe_.erase(this);
            }
            for (const auto &pos: newImplRefs) {
                if (!sheet_.GetCell(pos)) {
                    sheet_.SetCell(pos, "");
                }
                Cell *RefCell = dynamic_cast<Cell *>(sheet_.GetCell(pos));

                whoIReffering_.insert(RefCell);
                RefCell->whoRefferingToMe_.insert(this);
            }
        }
        whoIReffering_.clear();
        impl_ = std::move(newImpl);
    }else{
        whoIReffering_.clear();
        impl_ = std::make_unique<TextImpl>(TextImpl(text));
    }
}

void Cell::Clear() {
    impl_ = std::make_unique<EmptyImpl>(EmptyImpl());
}

Cell::Value Cell::GetValue() const {
    return impl_->GetValue();
}

std::string Cell::GetText() const {
    return impl_->GetText();
}

std::vector<Position> Cell::GetReferencedCells() const
{
    return impl_->GetReferencedCells();
}

void Cell::IsCircular(const std::vector<Position> &newImplRefs) const {
    std::unordered_set<CellInterface *> whoRefCells;
    FindCircularDependency(newImplRefs, whoRefCells);
}

void Cell::FindCircularDependency(const std::vector<Position> &newImplRefs,
                                          std::unordered_set<CellInterface *> &whoRefCells) const {
    for (const auto &pos: newImplRefs) {
        CellInterface *referenced_cell = sheet_.GetCell(pos);

        if (referenced_cell == this) {
            throw CircularDependencyException("");
        }

        if (referenced_cell && whoRefCells.count(referenced_cell) == 0) {
            const auto &another_ref_cells = referenced_cell->GetReferencedCells();
            if (!another_ref_cells.empty()) {
                FindCircularDependency(another_ref_cells, whoRefCells);
            }
            whoRefCells.insert(referenced_cell);
        }
    }
}
