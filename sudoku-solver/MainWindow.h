#pragma once

#include <QWidget>
#include <QGridLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <array>
#include <vector>
#include "Service.h"

class MainWindow : public QWidget {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

public slots:
    void onSolveClicked();
    void onGenerateClicked();
    void onHintClicked();
    void onClearClicked();
    void onCheckClicked();
    void onCellEdited();
    void onDifficultyChanged(int idx);

private:
    QGridLayout* gridLayout;
    std::array<std::array<QLineEdit*, 9>, 9> cells;

    QPushButton* solveButton;
    QPushButton* generateButton;
    QPushButton* hintButton;
    QPushButton* clearButton;
    QPushButton* checkButton;
    QComboBox* difficultyBox;
    QLabel* livesLabel;
    QLabel* difficultyLabel;

    Service service;
    int usedHints = 0;
    int lives = 3;
    int currentDifficulty = 2;
    bool editingGuard = false;

    void loadBoardToUI(const std::vector<std::vector<int>>& board);
    std::vector<std::vector<int>> loadBoardFromUI() const;
    void showMessage(const QString& message);
    void highlightCell(int row, int col, const QString& color);

    static QString baseCellStyle(int r, int c, bool isGiven = false);
    static bool hasDuplicates(const std::array<int, 9>& arr);

    void resetGame(int difficulty);
    void loseLifeAndMaybeRestart();
    void refreshInfo();
    QString renderLives() const;
};
