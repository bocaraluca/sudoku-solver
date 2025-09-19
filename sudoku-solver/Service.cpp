#include "Service.h"
#include <algorithm>
#include <array>

Service::Service() : board_() {}

void Service::loadBoard(const SudokuBoard& b) { board_ = b; }
SudokuBoard Service::getBoard() const { return board_; }

bool Service::solveRecursive(int row, int col)
{
    if (row == 9) return true;
    if (col == 9) return solveRecursive(row + 1, 0);
    if (board_.at(row, col) != 0) return solveRecursive(row, col + 1);

    for (int num = 1; num <= 9; ++num) {
        if (board_.isSafe(row, col, num)) {
            board_.put(row, col, num);
            if (solveRecursive(row, col + 1)) return true;
            board_.put(row, col, 0);
        }
    }
    return false;
}

bool Service::solve() { return solveRecursive(0, 0); }

bool Service::isValidMove(int row, int col, int num) const
{
    if (board_.at(row, col) != 0) return false;
    return board_.isSafe(row, col, num);
}

std::tuple<int, int, int> Service::getHint() const
{
    for (int r = 0; r < 9; ++r)
        for (int c = 0; c < 9; ++c)
            if (board_.at(r, c) == 0) {
                int cnt = 0, last = 0;
                for (int n = 1; n <= 9; ++n)
                    if (board_.isSafe(r, c, n)) { ++cnt; last = n; }
                if (cnt == 1) return { r,c,last };
            }
    return { -1,-1,-1 };
}

bool Service::logicalStep(Step& step)
{
    for (int r = 0; r < 9; ++r)
        for (int c = 0; c < 9; ++c)
            if (board_.at(r, c) == 0) {
                int cnt = 0, last = 0;
                for (int n = 1; n <= 9; ++n)
                    if (board_.isSafe(r, c, n)) { ++cnt; last = n; }
                if (cnt == 1) { board_.put(r, c, last); step = { Strategy::NakedSingle,r,c,last }; return true; }
            }

    for (int n = 1; n <= 9; ++n) {
        for (int r = 0; r < 9; ++r) {
            int cnt = 0, lc = -1;
            for (int c = 0; c < 9; ++c)
                if (board_.at(r, c) == 0 && board_.isSafe(r, c, n)) { ++cnt; lc = c; }
            if (cnt == 1) { board_.put(r, lc, n); step = { Strategy::HiddenSingle,r,lc,n }; return true; }
        }
        for (int c = 0; c < 9; ++c) {
            int cnt = 0, lr = -1;
            for (int r = 0; r < 9; ++r)
                if (board_.at(r, c) == 0 && board_.isSafe(r, c, n)) { ++cnt; lr = r; }
            if (cnt == 1) { board_.put(lr, c, n); step = { Strategy::HiddenSingle,lr,c,n }; return true; }
        }
        for (int br = 0; br < 3; ++br)
            for (int bc = 0; bc < 3; ++bc) {
                int cnt = 0, lr = -1, lc = -1;
                for (int dr = 0; dr < 3; ++dr)
                    for (int dc = 0; dc < 3; ++dc) {
                        int r = br * 3 + dr, c = bc * 3 + dc;
                        if (board_.at(r, c) == 0 && board_.isSafe(r, c, n)) { ++cnt; lr = r; lc = c; }
                    }
                if (cnt == 1) { board_.put(lr, lc, n); step = { Strategy::HiddenSingle,lr,lc,n }; return true; }
            }
    }

    if (nakedPairStep(step)) return true;

    step = { Strategy::None,-1,-1,-1 };
    return false;
}

bool Service::logicalSolve(std::vector<Step>& steps)
{
    bool progress = false; Step s;
    while (logicalStep(s)) { steps.push_back(s); progress = true; }
    return progress;
}

int Service::classifyDifficulty() const
{
    Service t = *this;
    std::vector<Step> steps;
    int hardest = 0;
    while (t.logicalSolve(steps)) {
        for (const auto& s : steps) {
            if (s.strategy == Strategy::NakedSingle) hardest = std::max(hardest, 1);
            else if (s.strategy == Strategy::HiddenSingle) hardest = std::max(hardest, 2);
            else if (s.strategy == Strategy::NakedPair) hardest = std::max(hardest, 3);
        }
        steps.clear();
    }
    if (!t.solve()) return 3;
    if (hardest == 0) return 1;
    return std::min(hardest, 3);
}

int Service::countSolutions(int limit) const
{
    int count = 0; solveCount(0, 0, count, limit); return count;
}

bool Service::solveCount(int row, int col, int& count, int limit) const
{
    if (count >= limit) return true;
    if (row == 9) { ++count; return count >= limit; }
    if (col == 9) return solveCount(row + 1, 0, count, limit);
    if (board_.at(row, col) != 0) return solveCount(row, col + 1, count, limit);

    for (int num = 1; num <= 9; ++num)
        if (board_.isSafe(row, col, num)) {
            const_cast<SudokuBoard&>(board_).put(row, col, num);
            bool stop = solveCount(row, col + 1, count, limit);
            const_cast<SudokuBoard&>(board_).put(row, col, 0);
            if (stop) return true;
        }
    return false;
}

std::vector<std::vector<int>> Service::generatePuzzle(int difficulty) const
{
    Service work;
    SudokuBoard b;
    std::vector<std::vector<int>> full(9, std::vector<int>(9, 0));

    std::random_device rd; std::mt19937 gen(rd());

    for (int d = 0; d < 9; d += 3) {
        std::vector<int> nums{ 1,2,3,4,5,6,7,8,9 };
        std::shuffle(nums.begin(), nums.end(), gen);
        int k = 0;
        for (int r = 0; r < 3; ++r)
            for (int c = 0; c < 3; ++c)
                full[d + r][d + c] = nums[k++];
    }

    b.setGrid(full); work.loadBoard(b); work.solve(); full = work.getBoard().getGrid();

    int targetClues;
    switch (difficulty) {
    case 1: targetClues = 36; break;
    case 2: targetClues = 30; break;
    case 3: targetClues = 26; break;
    default: targetClues = 32; break;
    }
    int toRemove = 81 - targetClues;

    std::vector<std::pair<int, int>> idx;
    idx.reserve(81);
    for (int r = 0; r < 9; ++r)
        for (int c = 0; c < 9; ++c)
            idx.emplace_back(r, c);
    std::shuffle(idx.begin(), idx.end(), gen);

    auto puzzle = full;

    for (auto [r, c] : idx) {
        if (toRemove <= 0) break;
        if (puzzle[r][c] == 0) continue;
        int saved = puzzle[r][c];
        puzzle[r][c] = 0;
        SudokuBoard tb; tb.setGrid(puzzle);
        Service t; t.loadBoard(tb);
        if (t.countSolutions(2) == 1) --toRemove;
        else puzzle[r][c] = saved;
    }

    return puzzle;
}

static inline void buildCandidates(const SudokuBoard& b, std::array<std::array<std::vector<int>, 9>, 9>& cand) {
    for (int r = 0;r < 9;++r)
        for (int c = 0;c < 9;++c) {
            cand[r][c].clear();
            if (b.at(r, c) != 0) continue;
            for (int n = 1;n <= 9;++n)
                if (b.isSafe(r, c, n)) cand[r][c].push_back(n);
        }
}

static bool applyNakedPairOnUnit(const SudokuBoard& b,
    const std::array<std::array<std::vector<int>, 9>, 9>& cand,
    const std::vector<std::pair<int, int>>& cellsIdx,
    std::array<std::array<std::vector<int>, 9>, 9>& outCand)
{
    bool changed = false;
    for (auto [r, c] : cellsIdx) outCand[r][c] = cand[r][c];

    for (size_t i = 0;i < cellsIdx.size();++i) {
        auto [r1, c1] = cellsIdx[i];
        if (b.at(r1, c1) != 0 || outCand[r1][c1].size() != 2) continue;
        for (size_t j = i + 1;j < cellsIdx.size();++j) {
            auto [r2, c2] = cellsIdx[j];
            if (b.at(r2, c2) != 0 || outCand[r2][c2].size() != 2) continue;
            if (outCand[r1][c1] == outCand[r2][c2]) {
                const auto pairVals = outCand[r1][c1];
                for (auto [rr, cc] : cellsIdx) {
                    if ((rr == r1 && cc == c1) || (rr == r2 && cc == c2)) continue;
                    if (b.at(rr, cc) != 0) continue;
                    auto& v = outCand[rr][cc];
                    size_t before = v.size();
                    v.erase(std::remove_if(v.begin(), v.end(), [&](int x) {
                        return x == pairVals[0] || x == pairVals[1];
                        }), v.end());
                    if (v.size() != before) changed = true;
                }
            }
        }
    }
    return changed;
}

bool Service::nakedPairStep(Step& step)
{
    std::array<std::array<std::vector<int>, 9>, 9> cand, after;
    buildCandidates(board_, cand);
    after = cand;

    bool changed = false;

    for (int r = 0;r < 9;++r) {
        std::vector<std::pair<int, int>> idx;
        for (int c = 0;c < 9;++c) idx.emplace_back(r, c);
        changed |= applyNakedPairOnUnit(board_, cand, idx, after);
    }
    for (int c = 0;c < 9;++c) {
        std::vector<std::pair<int, int>> idx;
        for (int r = 0;r < 9;++r) idx.emplace_back(r, c);
        changed |= applyNakedPairOnUnit(board_, cand, idx, after);
    }
    for (int br = 0;br < 3;++br) for (int bc = 0;bc < 3;++bc) {
        std::vector<std::pair<int, int>> idx;
        for (int dr = 0;dr < 3;++dr) for (int dc = 0;dc < 3;++dc)
            idx.emplace_back(br * 3 + dr, bc * 3 + dc);
        changed |= applyNakedPairOnUnit(board_, cand, idx, after);
    }

    if (!changed) return false;

    for (int r = 0;r < 9;++r)
        for (int c = 0;c < 9;++c)
            if (board_.at(r, c) == 0 && after[r][c].size() == 1) {
                int v = after[r][c][0];
                if (board_.isSafe(r, c, v)) {
                    board_.put(r, c, v);
                    step = { Strategy::NakedPair, r, c, v };
                    return true;
                }
            }
    return false;
}
