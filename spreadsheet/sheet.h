#pragma once

#include "cell.h"
#include "common.h"

#include <functional>
#include <unordered_map>

class Cell;

class Sheet : public SheetInterface {
public:
    ~Sheet();

    void SetCell(Position pos, std::string text) override;

    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;

    const CellInterface* operator()(Position*) const;

private:
    std::unordered_map<std::string, std::unique_ptr<Cell>> cells_;
};
