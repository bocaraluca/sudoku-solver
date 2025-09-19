#include "SudokuBoard.h"

SudokuBoard::SudokuBoard() : grid_(9, std::vector<int>(9, 0)) {}

void SudokuBoard::setGrid(const std::vector<std::vector<int>>& newGrid) {
    grid_ = newGrid;
}

const std::vector<std::vector<int>>& SudokuBoard::getGrid() const {
    return grid_;
}

int SudokuBoard::at(int r, int c) const {
    return grid_[r][c];
}

void SudokuBoard::put(int r, int c, int v) {
    grid_[r][c] = v;
}

bool SudokuBoard::isSafe(int row, int col, int num) const {
    for (int i = 0; i < 9; ++i) if (grid_[row][i] == num || grid_[i][col] == num) return false;
    int sr = row - row % 3, sc = col - col % 3;
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            if (grid_[sr + i][sc + j] == num) return false;
    return true;
}
