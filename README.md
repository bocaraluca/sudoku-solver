# Sudoku Solver

This is a Qt-based application that can solve, generate, and classify Sudoku puzzles.
It combines algorithmic problem-solving with a fully interactive GUI, showcasing both backend logic and frontend design.

---

## Features

- **Interactive Sudoku Board**: A visually appealing and user-friendly interface for solving puzzles.
- **Puzzle Generation**: Generate Sudoku puzzles of varying difficulty levels (Easy, Medium, Hard).
- **Solver**: Automatically solve any valid Sudoku puzzle using a recursive backtracking algorithm.
- **Hints System**: Get hints for the next logical move, with a limit of 3 hints per game.
- **Validation**: Check for mistakes in your solution and receive feedback.
- **Difficulty Classification**: Classifies puzzles based on their solving complexity.
- **Lives System**: Adds a fun challenge by limiting the number of mistakes you can make.
  
---

## Technologies Used

- **C++**: Core logic for solving and generating Sudoku puzzles.
- **Qt Framework**: For building the graphical user interface (GUI).
- **Object-Oriented Design**: Modular and reusable code structure.
- **Algorithms**: Recursive backtracking, logical solving strategies (e.g., Naked Singles, Hidden Singles, Naked Pairs).

---

## How It Works

1. **Puzzle Generation**: 
   - Randomly generates a complete Sudoku board.
   - Removes cells based on the selected difficulty while ensuring the puzzle has a unique solution.

2. **Solving Algorithm**:
   - Uses recursive backtracking to explore all possible solutions.
   - Implements logical strategies to solve puzzles step-by-step.

3. **UI Interaction**:
   - Input numbers directly into the grid.
   - Buttons for generating puzzles, solving, clearing, and validating the board.
   - Visual feedback for mistakes and hints.

