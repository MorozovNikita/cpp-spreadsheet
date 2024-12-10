#pragma once

#include "common.h"
#include "formula.h"

#include <optional>
#include <set>
#include <unordered_set>

class Sheet;
class Impl {
public:
    ~Impl() = default;
    virtual const std::string GetText() const = 0;
    virtual const CellInterface::Value GetValue() const = 0;
    virtual const std::vector<Position> GetReferencedCells() const = 0;
    virtual const std::optional<FormulaInterface::Value> GetCache() const = 0;
    virtual void ResetCache() = 0;
};


class Cell : public CellInterface {
public:
    Cell(Sheet& sheet);
    ~Cell() = default;

    void Set(std::string text);
    void Clear();
    Value GetValue() const override;
    std::string GetText() const override;
    
    std::vector<Position> GetReferencedCells() const override;
    bool IsReferenced() const;
private:
    Sheet& sheet_;
    std::set<Cell*> whoIReffering_;
    std::set<Cell*> whoRefferingToMe_;
    std::unique_ptr<Impl> impl_;

    void IsCircular(const std::vector<Position> &ref_cells) const;
    void FindCircularDependency(const std::vector<Position> &ref_cells,
                                std::unordered_set<CellInterface *> &visited_cells) const;
};
