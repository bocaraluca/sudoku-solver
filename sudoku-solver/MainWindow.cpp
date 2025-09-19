#include "MainWindow.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFont>
#include <QMessageBox>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QSignalBlocker>
#include <QTimer>
#include <array>
#include "SudokuBoard.h"

QString MainWindow::baseCellStyle(int r, int c, bool isGiven) {
    QString s =
        "QLineEdit {"
        "  border: 1px solid #666;"
        "  background-color: white;"
        "  selection-background-color: #cfe9ff;"
        "  border-radius: 4px;"
        "}";
    if (r % 3 == 0) s += " QLineEdit { border-top: 2px solid #222; }";
    if (c % 3 == 0) s += " QLineEdit { border-left: 2px solid #222; }";
    if (r == 8)     s += " QLineEdit { border-bottom: 2px solid #222; }";
    if (c == 8)     s += " QLineEdit { border-right: 2px solid #222; }";
    if (isGiven)    s += " QLineEdit { background-color: #f0f0f0; font-weight: 600; }";
    s += " QLineEdit:focus { outline: none; box-shadow: 0 0 0 2px #7db7ff; }";
    return s;
}

bool MainWindow::hasDuplicates(const std::array<int, 9>& arr) {
    bool seen[10] = { false };
    for (int x : arr) {
        if (x == 0) continue;
        if (seen[x]) return true;
        seen[x] = true;
    }
    return false;
}

MainWindow::MainWindow(QWidget* parent)
    : QWidget(parent)
{
    setStyleSheet("QWidget { background: #fafafa; } QPushButton { padding: 6px 10px; }");
    auto* mainLayout = new QVBoxLayout(this);

    auto* infoBar = new QHBoxLayout();
    infoBar->setContentsMargins(10, 10, 10, 0);
    infoBar->setSpacing(12);
    livesLabel = new QLabel(this);
    difficultyLabel = new QLabel(this);
    livesLabel->setStyleSheet("QLabel { font-weight: 600; }");
    difficultyLabel->setStyleSheet("QLabel { color: #333; }");
    infoBar->addWidget(livesLabel);
    infoBar->addStretch(1);
    infoBar->addWidget(difficultyLabel);
    mainLayout->addLayout(infoBar);

    gridLayout = new QGridLayout();
    gridLayout->setSpacing(4);
    gridLayout->setContentsMargins(10, 10, 10, 10);

    auto* cellValidator = new QRegularExpressionValidator(QRegularExpression("^[1-9]?$"), this);

    QFont cellFont;
    cellFont.setPointSize(18);

    for (int r = 0; r < 9; ++r) {
        for (int c = 0; c < 9; ++c) {
            auto* cell = new QLineEdit(this);
            cell->setMaxLength(1);
            cell->setAlignment(Qt::AlignCenter);
            cell->setFont(cellFont);
            cell->setValidator(cellValidator);
            cell->setFixedSize(48, 48);
            cell->setProperty("row", r);
            cell->setProperty("col", c);
            cell->setStyleSheet(baseCellStyle(r, c));
            connect(cell, &QLineEdit::editingFinished, this, &MainWindow::onCellEdited);
            cells[r][c] = cell;
            gridLayout->addWidget(cell, r, c);
        }
    }
    mainLayout->addLayout(gridLayout);

    auto* controls = new QHBoxLayout();
    controls->setContentsMargins(10, 0, 10, 0);
    controls->setSpacing(8);

    difficultyBox = new QComboBox(this);
    difficultyBox->addItems({ "Easy", "Medium", "Hard" });
    difficultyBox->setCurrentIndex(currentDifficulty - 1);

    generateButton = new QPushButton("Generate", this);
    solveButton = new QPushButton("Solve", this);
    hintButton = new QPushButton("Hint (3 left)", this);
    clearButton = new QPushButton("Clear", this);
    checkButton = new QPushButton("Check", this);

    controls->addWidget(difficultyBox);
    controls->addWidget(generateButton);
    controls->addWidget(solveButton);
    controls->addWidget(hintButton);
    controls->addWidget(clearButton);
    controls->addWidget(checkButton);

    mainLayout->addLayout(controls);

    connect(solveButton, &QPushButton::clicked, this, &MainWindow::onSolveClicked);
    connect(generateButton, &QPushButton::clicked, this, &MainWindow::onGenerateClicked);
    connect(hintButton, &QPushButton::clicked, this, &MainWindow::onHintClicked);
    connect(clearButton, &QPushButton::clicked, this, &MainWindow::onClearClicked);
    connect(checkButton, &QPushButton::clicked, this, &MainWindow::onCheckClicked);
    connect(difficultyBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onDifficultyChanged);

    resize(540, 660);
    resetGame(currentDifficulty);
}

MainWindow::~MainWindow() = default;

void MainWindow::refreshInfo() {
    difficultyLabel->setText("Difficulty: " + difficultyBox->currentText());
    livesLabel->setText("Lives: " + renderLives());
}

QString MainWindow::renderLives() const {
    QString s;
    for (int i = 0; i < 3; ++i) s += (i < lives ? "♥" : "♡");
    return s;
}

void MainWindow::resetGame(int difficulty) {
    currentDifficulty = difficulty;
    auto board = service.generatePuzzle(currentDifficulty);
    SudokuBoard sb; sb.setGrid(board);
    service.loadBoard(sb);
    for (int r = 0; r < 9; ++r) {
        for (int c = 0; c < 9; ++c) {
            QSignalBlocker b1(cells[r][c]);
            if (board[r][c] == 0) cells[r][c]->clear();
            else cells[r][c]->setText(QString::number(board[r][c]));
            bool given = (board[r][c] != 0);
            cells[r][c]->setReadOnly(given);
            cells[r][c]->setStyleSheet(baseCellStyle(r, c, given));
        }
    }
    usedHints = 0;
    lives = 3;
    hintButton->setText("Hint (3 left)");
    refreshInfo();
}

void MainWindow::loseLifeAndMaybeRestart() {
    lives -= 1;
    if (lives > 0) {
        refreshInfo();
        showMessage(QString("Mistake! Lives left: %1").arg(lives));
    }
    else {
        showMessage("You lost all 3 lives. Starting a new game.");
        resetGame(currentDifficulty);
    }
}

void MainWindow::onSolveClicked()
{
    SudokuBoard sb; sb.setGrid(loadBoardFromUI());
    service.loadBoard(sb);
    if (service.solve()) {
        loadBoardToUI(service.getBoard().getGrid());
        for (int r = 0; r < 9; ++r)
            for (int c = 0; c < 9; ++c) {
                QSignalBlocker b(cells[r][c]);
                cells[r][c]->setReadOnly(true);
                cells[r][c]->setStyleSheet(baseCellStyle(r, c, true));
            }
        showMessage("Solved!");
    }
    else {
        showMessage("No solution exists for the current board.");
    }
}

void MainWindow::onGenerateClicked()
{
    int difficulty = difficultyBox->currentIndex() + 1;
    resetGame(difficulty);
    showMessage("New puzzle generated.");
}

void MainWindow::onHintClicked()
{
    if (usedHints >= 3) { showMessage("No hints left!"); return; }
    SudokuBoard sb; sb.setGrid(loadBoardFromUI());
    service.loadBoard(sb);
    auto [row, col, value] = service.getHint();
    if (row == -1) { showMessage("No simple hint available!"); return; }
    {
        QSignalBlocker b1(cells[row][col]);
        cells[row][col]->setText(QString::number(value));
    }
    highlightCell(row, col, "#dcedc8");
    usedHints++;
    hintButton->setText(QString("Hint (%1 left)").arg(3 - usedHints));
}

void MainWindow::onClearClicked()
{
    for (int r = 0; r < 9; ++r)
        for (int c = 0; c < 9; ++c) {
            QSignalBlocker b1(cells[r][c]);
            cells[r][c]->clear();
            cells[r][c]->setReadOnly(false);
            cells[r][c]->setStyleSheet(baseCellStyle(r, c));
        }
    usedHints = 0;
    lives = 3;
    hintButton->setText("Hint (3 left)");
    refreshInfo();
}

void MainWindow::onCheckClicked()
{
    auto board = loadBoardFromUI();
    for (int r = 0; r < 9; ++r)
        for (int c = 0; c < 9; ++c) {
            QSignalBlocker b1(cells[r][c]);
            bool given = cells[r][c]->isReadOnly();
            cells[r][c]->setStyleSheet(baseCellStyle(r, c, given));
        }

    bool ok = true;

    for (int r = 0; r < 9; ++r) {
        std::array<int, 9> row{};
        for (int c = 0; c < 9; ++c) row[c] = board[r][c];
        if (hasDuplicates(row)) {
            ok = false;
            for (int c = 0; c < 9; ++c) highlightCell(r, c, "#ffcccb");
        }
    }

    for (int c = 0; c < 9; ++c) {
        std::array<int, 9> col{};
        for (int r = 0; r < 9; ++r) col[r] = board[r][c];
        if (hasDuplicates(col)) {
            ok = false;
            for (int r = 0; r < 9; ++r) highlightCell(r, c, "#ffcccb");
        }
    }

    for (int br = 0; br < 3; ++br) {
        for (int bc = 0; bc < 3; ++bc) {
            std::array<int, 9> box{};
            int idx = 0;
            for (int r = br * 3; r < br * 3 + 3; ++r)
                for (int c = bc * 3; c < bc * 3 + 3; ++c)
                    box[idx++] = board[r][c];
            if (hasDuplicates(box)) {
                ok = false;
                for (int r = br * 3; r < br * 3 + 3; ++r)
                    for (int c = bc * 3; c < bc * 3 + 3; ++c)
                        highlightCell(r, c, "#ffcccb");
            }
        }
    }

    if (!ok) loseLifeAndMaybeRestart();
    else showMessage("No mistakes found.");
}

void MainWindow::onCellEdited()
{
    if (editingGuard) return;
    auto* cell = qobject_cast<QLineEdit*>(sender());
    if (!cell) return;
    if (cell->isReadOnly()) return;

    QSignalBlocker blockCell(cell);
    editingGuard = true;

    int r = cell->property("row").toInt();
    int c = cell->property("col").toInt();

    const QString t = cell->text().trimmed();
    if (t.isEmpty()) {
        cell->setStyleSheet(baseCellStyle(r, c, false));
        editingGuard = false;
        return;
    }

    int val = t.toInt();
    auto board = loadBoardFromUI();
    board[r][c] = 0;
    SudokuBoard sb; sb.setGrid(board);
    service.loadBoard(sb);

    if (!service.isValidMove(r, c, val)) {
        highlightCell(r, c, "#ffcccb");
        QTimer::singleShot(0, this, [this]() { loseLifeAndMaybeRestart(); editingGuard = false; });
    }
    else {
        highlightCell(r, c, "#e8f5e9");
        editingGuard = false;
    }
}

void MainWindow::onDifficultyChanged(int idx)
{
    refreshInfo();
    resetGame(idx + 1);
}

void MainWindow::loadBoardToUI(const std::vector<std::vector<int>>& board)
{
    for (int r = 0; r < 9; ++r)
        for (int c = 0; c < 9; ++c) {
            QSignalBlocker b1(cells[r][c]);
            if (board[r][c] == 0)
                cells[r][c]->clear();
            else
                cells[r][c]->setText(QString::number(board[r][c]));
            bool given = cells[r][c]->isReadOnly();
            cells[r][c]->setStyleSheet(baseCellStyle(r, c, given));
        }
}

std::vector<std::vector<int>> MainWindow::loadBoardFromUI() const
{
    std::vector<std::vector<int>> board(9, std::vector<int>(9, 0));
    for (int r = 0; r < 9; ++r)
        for (int c = 0; c < 9; ++c) {
            const QString t = cells[r][c]->text().trimmed();
            board[r][c] = t.isEmpty() ? 0 : t.toInt();
        }
    return board;
}

void MainWindow::showMessage(const QString& message)
{
    QMessageBox::information(this, "Sudoku", message);
}

void MainWindow::highlightCell(int row, int col, const QString& color)
{
    QSignalBlocker b1(cells[row][col]);
    bool given = cells[row][col]->isReadOnly();
    cells[row][col]->setStyleSheet(
        baseCellStyle(row, col, given) +
        QString(" QLineEdit { background-color: %1; }").arg(color)
    );
}
