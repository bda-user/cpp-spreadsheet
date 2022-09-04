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
    if(!pos.IsValid()) {
        throw InvalidPositionException("!pos.IsValid()"s);
    }
    auto pos_ = pos.ToString();
    if(auto it = cells_.find(pos_); it == cells_.end()) {
        auto it_cell = cells_.insert({std::move(pos_), std::make_unique<Cell>(*this)});
        it_cell.first->second->SetPosition(pos);
        it_cell.first->second->Set(text);
    } else {
        it->second->Set(text);
    }
}

const CellInterface* Sheet::GetCell(Position pos) const {
    if(!pos.IsValid()) {
        throw InvalidPositionException("!pos.IsValid()"s);
    }

    if(auto it = cells_.find(pos.ToString()); it == cells_.end()) {
        return nullptr;
    } else {
        return it->second.get();
    }
}

CellInterface* Sheet::GetCell(Position pos) {
    if(!pos.IsValid()) {
        throw InvalidPositionException("!pos.IsValid()"s);
    }

    auto pos_ = pos.ToString();
    if(auto it = cells_.find(pos_); it == cells_.end()) {
        return nullptr;
    } else {
        return it->second.get();
    }
}

void Sheet::ClearCell(Position pos) {
    if(!pos.IsValid()) {
        throw InvalidPositionException("!pos.IsValid()"s);
    }

    if(auto it = cells_.find(pos.ToString()); it != cells_.end()) {
        cells_.erase(it);
    }
}

Size Sheet::GetPrintableSize() const {
    Size size{0, 0};
    if(cells_.size() ==0) {
        return size;
    }

    for(const auto& [pos, _] : cells_) {
        auto pos_ = Position::FromString(pos);
        if(pos_.row > size.rows) {
            size.rows = pos_.row;
        }
        if(pos_.col > size.cols) {
            size.cols = pos_.col;
        }
    }
    ++size.rows;
    ++size.cols;
    return size;
}

void Sheet::PrintValues(std::ostream& output) const {
    auto size = GetPrintableSize();
    int col = 0;
    bool first_col = true;
    for(int row = 0; row < size.rows; ++row) {
        for(col = 0; col < size.cols; ++col) {
            if(first_col) {
                first_col = false;
            } else {
                output << '\t';
            }
            if(auto it = cells_.find(Position{row, col}.ToString()); it != cells_.end()) {
                auto value = it->second->GetValue();
                if(std::holds_alternative<std::string>(value)) {
                    output << std::get<std::string>(value);
                } else
                if(std::holds_alternative<double>(value)) {
                    output << std::get<double>(value);
                } else
                if(std::holds_alternative<FormulaError>(value)) {
                    output << "#DIV/0!"s;
                }
            } else {
                output << ""s;
            }
        }
        first_col = true;
        output << '\n';
    }
}

void Sheet::PrintTexts(std::ostream& output) const {
    auto size = GetPrintableSize();
    int col = 0;
    bool first_col = true;
    for(int row = 0; row < size.rows; ++row) {
        for(col = 0; col < size.cols; ++col) {
            if(first_col) {
                first_col = false;
            } else {
                output << '\t';
            }
            if(auto it = cells_.find(Position{row, col}.ToString()); it != cells_.end()) {
                output << it->second->GetText();
            } else {
                output << ""s;
            }
        }
        first_col = true;
        output << '\n';
    }
}

const CellInterface* Sheet::operator()(Position* pos) const {
    return GetCell(*pos);
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}
