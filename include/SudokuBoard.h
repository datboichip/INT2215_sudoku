#pragma once

#include <array>
#include <vector>
#include <random>
#include <algorithm>

class SudokuBoard {
public:
    static const int BOARD_SIZE = 9;
    static const int EMPTY_CELL = 0;
    static const int BOX_SIZE = 3;
    
    SudokuBoard();
    void NewGame(int difficulty);
    bool IsSolved() const;
    int GetCell(int row, int col) const;
    bool IsOriginalCell(int row, int col) const;
    bool IsValidMove(int row, int col, int value) const;
    void SetCell(int row, int col, int value);
    void ClearCell(int row, int col);
    bool GetHint(int& row, int& col, int& value);
    bool IsNumberValid(int row, int col) const;

private:
    std::array<std::array<int, BOARD_SIZE>, BOARD_SIZE> mBoard;
    std::array<std::array<bool, BOARD_SIZE>, BOARD_SIZE> mOriginalCells;
    void GenerateCompleteSolution();
    void RemoveCells(int difficulty);
    bool SolveBoard(std::array<std::array<int, BOARD_SIZE>, BOARD_SIZE>& board);
    bool IsValidInRow(int row, int value) const;
    bool IsValidInColumn(int col, int value) const;
    bool IsValidInBox(int boxRow, int boxCol, int value) const;
    std::mt19937 mRng;
}; 