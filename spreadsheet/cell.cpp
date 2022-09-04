#include "cell.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>

using namespace std::literals;

void Impl::Set(std::string text) {
    text_ = std::move(text);
}

void Impl::Clear() {
    text_ = ""s;
}

Impl::Value Impl::GetValue() const {
    return Impl::Value{text_};
}

std::string Impl::GetText() const {
    return text_;
}

std::vector<Position> Impl::GetReferencedCells() const {
    return {};
}

void Impl::CasheReferencesUpdate(CellInterface* cell, bool act) {
    if(act) {
        cashed_cells_.insert(cell);
    } else {
        cashed_cells_.erase(cell);
    }
}

void Impl::CasheClear(std::unordered_set<CellInterface*>& cells_ready) {
    cashe_valid_ = false; // cache invalidation
    for(auto& cell : cashed_cells_) {
        if(cells_ready.count(cell) == 1) {
            return; // no double check
        } else {
            cells_ready.insert(cell);
        }
        // recursion call CasheClear
        static_cast<Impl*>(cell)->CasheClear(cells_ready);
    }
}

void EmptyImpl::Set(std::string text) {
    Clear();
}

EmptyImpl::Value EmptyImpl::GetValue() const {
    return EmptyImpl::Value{text_};
}

std::string EmptyImpl::GetText() const {
    return text_;
}

std::vector<Position> EmptyImpl::GetReferencedCells() const {
    return {};
}

void TextImpl::Set(std::string text) {
    text_ = std::move(text);
    std::unordered_set<CellInterface*> cells_ready;
    CasheClear(cells_ready);
}

TextImpl::Value TextImpl::GetValue() const {
    if(text_[0] == ESCAPE_SIGN) {
        return TextImpl::Value{text_.substr(1)};
    }
    return TextImpl::Value{text_};
}

std::string TextImpl::GetText() const {
    return text_;
}

std::vector<Position> TextImpl::GetReferencedCells() const {
    return {};
}

FormulaImpl::FormulaImpl(Sheet& sheet) : sheet_(sheet) {}

void FormulaImpl::Set(std::string text) {

    formula_ = ParseFormula(text.substr(1));
    referenced_cells_old_ = std::move(referenced_cells_); // save
    referenced_cells_ = formula_->GetReferencedCells();
    std::unordered_set<CellInterface*> cells_ready;

    if(!ReferencesCheck(pos_, cells_ready)) {
        referenced_cells_ = std::move(referenced_cells_old_); // restore
        throw CircularDependencyException("FormulaImpl::Set(); ::ReferencesCheck()"s);
    }

    text_ = std::move(text);
    for(auto& pos : referenced_cells_old_) {
        auto cell = sheet_.GetCell(pos);
        static_cast<Cell*>(cell)->operator Impl *()->CasheReferencesUpdate(this, false);
    }

    for(auto& pos : referenced_cells_) {
        auto cell = sheet_.GetCell(pos);
        static_cast<Cell*>(cell)->operator Impl *()->CasheReferencesUpdate(this, true);
    }

    cells_ready.clear();
    CasheClear(cells_ready);
    cashed_value_ = GetValue();
    cashe_valid_ = true;
}

FormulaImpl::Value FormulaImpl::GetValue() const {
    if(cashe_valid_) {
        return cashed_value_;
    }

    auto res = formula_->Evaluate(sheet_);
    if(std::holds_alternative<double>(res)) {
        double value = std::get<double>(res);
        return FormulaImpl::Value{value};
    }
    return FormulaImpl::Value{std::get<FormulaError>(res)};
}

std::string FormulaImpl::GetText() const {
    return "="s + formula_->GetExpression();
}

std::vector<Position> FormulaImpl::GetReferencedCells() const {
    return referenced_cells_;
}

bool FormulaImpl::ReferencesCheck(Position pos_,
                                  std::unordered_set<CellInterface*>& cells_ready) {

    for(const auto& pos : referenced_cells_){
        auto cell = sheet_.GetCell(pos);
        if(cell == nullptr) { // create referenced cell
            sheet_.SetCell(pos, ""s);
            cell = sheet_.GetCell(pos);
        }

        if(static_cast<Impl*>(cell)->pos_ == pos_) {
            return false; // has cycle
        }

        if(cells_ready.count(cell) == 1){
            continue; // no double check
        } else {
            cells_ready.insert(cell);
        }

        if(cell->GetReferencedCells().size()) {
           return static_cast<Cell*>(cell)->
                   operator FormulaImpl *()->ReferencesCheck(pos_, cells_ready);
        }
    }
    return true;
}

// Реализуйте следующие методы
Cell::Cell(Sheet& sheet) :
    impl_(std::make_unique<EmptyImpl>()), sheet_(sheet) {}

Cell::~Cell() {}

void Cell::Set(std::string text) {

    if(impl_.get() && impl_.get()->text_ == text){ ;
        return; // change NOT
    }

    if(text[0] == FORMULA_SIGN && text.size() > 1) {
        auto cell = std::make_unique<FormulaImpl>(sheet_);
        cell.get()->pos_ = impl_.get()->pos_;
        cell.get()->Set(text);
        impl_ = std::move(cell);
    } else {
        auto cell = std::make_unique<TextImpl>();
        cell.get()->pos_ = impl_.get()->pos_;
        cell.get()->Set(text);
        impl_ = std::move(cell);
    }
}

void Cell::Clear() {
    impl_.get()->Clear();
}

Cell::Value Cell::GetValue() const {
    return impl_.get()->GetValue();
}

std::string Cell::GetText() const {
    return impl_.get()->GetText();
}

std::vector<Position> Cell::GetReferencedCells() const {
    return  impl_.get()->GetReferencedCells();
}

bool Cell::IsReferenced() const {
    return impl_.get()->GetReferencedCells().size() > 0;
}

void Cell::SetPosition(Position pos) {
    impl_.get()->pos_ = std::move(pos);
}

Cell::operator FormulaImpl*() {
    if(IsReferenced()) {
        return static_cast<FormulaImpl*>(impl_.get());
    }
    return nullptr;
}

Cell::operator Impl*() {
    return impl_.get();
}
