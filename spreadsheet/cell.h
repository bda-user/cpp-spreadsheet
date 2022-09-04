#pragma once

#include "common.h"
#include "formula.h"
#include "sheet.h"

#include <functional>
#include <unordered_set>

using namespace std::literals;

class Sheet;

class Cell;

class Impl : public CellInterface {

public:
    ~Impl() {};
    void Set(std::string text);
    void Clear();
    virtual Value GetValue() const override;
    virtual std::string GetText() const override;
    virtual std::vector<Position> GetReferencedCells() const override;

    void CasheReferencesUpdate(CellInterface*, bool); // bool = true ? ADD : DEL
    void CasheClear(std::unordered_set<CellInterface*>&);

private:

    friend class EmptyImpl;
    friend class TextImpl;
    friend class FormulaImpl;
    friend class Cell;

    std::string text_ = ""s;
    Position pos_{-1, -1};

    std::unordered_set<CellInterface*> cashed_cells_;
    bool cashe_valid_ = false;
    Value cashed_value_;
};

class EmptyImpl : public Impl {

public:
    ~EmptyImpl() {};
    void Set(std::string text);
    Value GetValue() const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;
};

class TextImpl : public Impl {

public:
    ~TextImpl() {};
    void Set(std::string text);
    Value GetValue() const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;
};

class FormulaImpl : public Impl {

public:
    FormulaImpl(Sheet& sheet);
    ~FormulaImpl() {};
    void Set(std::string text);
    Value GetValue() const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;

    bool ReferencesCheck(Position, std::unordered_set<CellInterface*>&);

private:
    std::unique_ptr<FormulaInterface> formula_;
    Sheet& sheet_;
    std::vector<Position> referenced_cells_old_;
    std::vector<Position> referenced_cells_;
};


class Cell : public CellInterface {
public:
    Cell(Sheet& sheet);
    ~Cell();

    void Set(std::string text);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;

    bool IsReferenced() const;

    void SetPosition(Position);

    operator FormulaImpl*();

    operator Impl*();

private:

    std::unique_ptr<Impl> impl_;
    Sheet& sheet_;
};
