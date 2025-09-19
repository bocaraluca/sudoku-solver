#pragma once
#include <vector>
#include <tuple>
#include <random>
#include "SudokuBoard.h"
#include "Strategy.h"

class Service {
public:
    Service();

    void loadBoard(const SudokuBoard& b);
    SudokuBoard getBoard() const;

    bool solve();
    bool isValidMove(int row, int col, int num) const;
    std::tuple<int, int, int> getHint() const;

    bool logicalStep(Step& step);
    bool logicalSolve(std::vector<Step>& steps);
    int classifyDifficulty() const;

    int countSolutions(int limit = 2) const;
    std::vector<std::vector<int>> generatePuzzle(int difficulty = 1) const;

private:
    SudokuBoard board_;
    bool solveRecursive(int row, int col);
    bool solveCount(int row, int col, int& count, int limit) const;
    bool nakedPairStep(Step& step);
};
