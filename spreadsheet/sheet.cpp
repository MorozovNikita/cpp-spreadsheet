#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>

using namespace std::literals;

Sheet::~Sheet() {}

void Sheet::SetCell(Position pos, std::string text) {
    if(!pos.IsValid())
        throw InvalidPositionException("");

    if(pos.row >= size_.rows){
        data_.resize(pos.row+1);
        size_.rows = pos.row+1;
    }

    for(auto &row : data_){
        row.resize(size_.cols);
    }

    if(pos.col >= size_.cols){
        for(auto &row : data_){
            row.resize(pos.col+1);
        }
        size_.cols = pos.col+1;
    }

    if(data_[pos.row][pos.col] == nullptr)
        data_[pos.row][pos.col] = std::make_unique<Cell>(*this);
    data_[pos.row][pos.col]->Set(text);
}

const CellInterface* Sheet::GetCell(Position pos) const {
    if(!pos.IsValid())
        throw InvalidPositionException("");

    if(pos.row >= size_.rows || pos.col >= size_.cols)
        return nullptr;

    if(data_[pos.row][pos.col].get() == nullptr)
        return nullptr;

    return data_.at(pos.row).at(pos.col).get();
}
CellInterface* Sheet::GetCell(Position pos) {
    if(!pos.IsValid())
        throw InvalidPositionException("");

    if(pos.row >= size_.rows || pos.col >= size_.cols)
        return nullptr;

    if(data_[pos.row][pos.col].get() == nullptr)
        return nullptr;

    return data_[pos.row][pos.col].get();
}

void Sheet::ClearCell(Position pos) {
    if(!pos.IsValid())
        throw InvalidPositionException("");

    if(pos.row >= size_.rows || pos.col >= size_.cols)
        return;

    if(data_[pos.row][pos.col].get() == nullptr)
        return;

    if(pos.row < size_.rows && pos.col < size_.cols){
        data_[pos.row][pos.col] = nullptr;

        auto iter = data_.rbegin();
        while(iter < data_.rend()){
            bool isEmpty = true;
            for(auto& cell : *iter){
                if(cell.get() != nullptr){
                    isEmpty = false;
                }
            }
            if(isEmpty){

                data_.resize(size_.rows-1);
                --size_.rows;
                ++iter;
            }else{
                break;
            }
        }

        int ind = size_.cols - 1;
        while(ind >= 0){
            bool isEmpty = true;
            for(auto& row : data_){
                if(row[ind].get() != nullptr)
                    isEmpty = false;
            }
            if(isEmpty){

                for(auto &row : data_){
                    row.resize(size_.cols-1);
                }
                --size_.cols;
                --ind;
            }else{
                break;
            }
        }
    }
}

Size Sheet::GetPrintableSize() const {
    return size_;
}

void Sheet::PrintValues(std::ostream& output) const {
    for (int x = 0; x < size_.rows; ++x) {
        for (int y = 0; y < size_.cols; ++y) {
            auto cell = GetCell(Position{x, y});
            Cell::Value val;
            if (cell) {
                val = cell->GetValue();
            }

            if(val.index() == 0){
                output << std::get<std::string>(val);
            }else if(val.index() == 1){
                output << std::get<double>(val);
            }else if(val.index() == 2){
                output << std::get<FormulaError>(val);
            }

            if(y + 1 != size_.cols)
                output << "\t";
        }
        output << '\n';
    }
}
void Sheet::PrintTexts(std::ostream& output) const {
    for(const auto &row : data_){
        bool isFirst = true;
        for(const auto &cell : row){
            if(!isFirst)
                output << '\t';
            if(cell.get() != nullptr)
                output << cell->GetText();
            isFirst = false;
        }
        output << '\n';
    }
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}
