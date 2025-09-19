#pragma once
#include <vector>

class SudokuBoard {
public:
    SudokuBoard();
    void setGrid(const std::vector<std::vector<int>>& newGrid);
    const std::vector<std::vector<int>>& getGrid() const;
    int at(int r, int c) const;
    void put(int r, int c, int v);
    bool isSafe(int row, int col, int num) const;

private:
    std::vector<std::vector<int>> grid_;
};
