#include "SudokuBoard.h"
#include <ctime>
#include <iostream>
#include <random>
#include <algorithm>

SudokuBoard::SudokuBoard() 
    : mRng(std::random_device{}())
{
    for (int row = 0; row < BOARD_SIZE; ++row) {
        for (int col = 0; col < BOARD_SIZE; ++col) {
            mBoard[row][col] = EMPTY_CELL;
            mOriginalCells[row][col] = false;
        }
    }
}

void SudokuBoard::NewGame(int difficulty) {
    for (int row = 0; row < BOARD_SIZE; ++row) {
        for (int col = 0; col < BOARD_SIZE; ++col) {
            mBoard[row][col] = EMPTY_CELL;
            mOriginalCells[row][col] = false;
        }
    }
    
    GenerateCompleteSolution();
    
    RemoveCells(difficulty);
}

bool SudokuBoard::IsSolved() const {
    for (int row = 0; row < BOARD_SIZE; ++row) {
        for (int col = 0; col < BOARD_SIZE; ++col) {
            if (mBoard[row][col] == EMPTY_CELL) {
                return false;
            }
        }
    }
    
    for (int i = 0; i < BOARD_SIZE; ++i) {
        for (int num = 1; num <= BOARD_SIZE; ++num) {
            if (!IsValidInRow(i, num)) {
                return false;
            }
        }
        
        for (int num = 1; num <= BOARD_SIZE; ++num) {
            if (!IsValidInColumn(i, num)) {
                return false;
            }
        }
    }
    
    for (int boxRow = 0; boxRow < BOX_SIZE; ++boxRow) {
        for (int boxCol = 0; boxCol < BOX_SIZE; ++boxCol) {
            for (int num = 1; num <= BOARD_SIZE; ++num) {
                if (!IsValidInBox(boxRow, boxCol, num)) {
                    return false;
                }
            }
        }
    }
    
    return true;
}

int SudokuBoard::GetCell(int row, int col) const {
    if (row >= 0 && row < BOARD_SIZE && col >= 0 && col < BOARD_SIZE) {
        return mBoard[row][col];
    }
    return EMPTY_CELL;
}

bool SudokuBoard::IsOriginalCell(int row, int col) const {
    if (row >= 0 && row < BOARD_SIZE && col >= 0 && col < BOARD_SIZE) {
        return mOriginalCells[row][col];
    }
    return false;
}

bool SudokuBoard::IsValidMove(int row, int col, int value) const {
    if (row < 0 || row >= BOARD_SIZE || col < 0 || col >= BOARD_SIZE || 
        value < 1 || value > BOARD_SIZE) {
        return false;
    }
    
    if (mOriginalCells[row][col]) {
        return false;
    }
    
    for (int c = 0; c < BOARD_SIZE; ++c) {
        if (mBoard[row][c] == value) {
            return false;
        }
    }
    
    for (int r = 0; r < BOARD_SIZE; ++r) {
        if (mBoard[r][col] == value) {
            return false;
        }
    }
    
    int boxRowStart = (row / BOX_SIZE) * BOX_SIZE;
    int boxColStart = (col / BOX_SIZE) * BOX_SIZE;
    
    for (int r = boxRowStart; r < boxRowStart + BOX_SIZE; ++r) {
        for (int c = boxColStart; c < boxColStart + BOX_SIZE; ++c) {
            if (mBoard[r][c] == value) {
                return false;
            }
        }
    }
    
    return true;
}

void SudokuBoard::SetCell(int row, int col, int value) {
    if (row >= 0 && row < BOARD_SIZE && col >= 0 && col < BOARD_SIZE &&
        value >= 1 && value <= BOARD_SIZE && !mOriginalCells[row][col]) {
        mBoard[row][col] = value;
    }
}

void SudokuBoard::ClearCell(int row, int col) {
    if (row >= 0 && row < BOARD_SIZE && col >= 0 && col < BOARD_SIZE && 
        !mOriginalCells[row][col]) {
        mBoard[row][col] = EMPTY_CELL;
    }
}

bool SudokuBoard::GetHint(int& row, int& col, int& value) {
    std::vector<std::pair<int, int>> emptyCells;
    
    for (int r = 0; r < BOARD_SIZE; ++r) {
        for (int c = 0; c < BOARD_SIZE; ++c) {
            if (mBoard[r][c] == EMPTY_CELL) {
                emptyCells.push_back({r, c});
            }
        }
    }
    
    if (emptyCells.empty()) {
        return false;
    }
    
    std::uniform_int_distribution<int> dist(0, static_cast<int>(emptyCells.size()) - 1);
    int index = dist(mRng);
    row = emptyCells[index].first;
    col = emptyCells[index].second;
    
    auto boardCopy = mBoard;
    
    if (SolveBoard(boardCopy)) {
        value = boardCopy[row][col];
        return true;
    }
    
    return false;
}

void SudokuBoard::GenerateCompleteSolution() {
    for (int row = 0; row < BOARD_SIZE; ++row) {
        for (int col = 0; col < BOARD_SIZE; ++col) {
            mBoard[row][col] = EMPTY_CELL;
        }
    }
    
    SolveBoard(mBoard);
}

void SudokuBoard::RemoveCells(int difficulty) {
    int cellsToRemove;
    switch (difficulty) {
        case 1:
            cellsToRemove = 40;
            break;
        case 2:
            cellsToRemove = 50;
            break;
        case 3:
            cellsToRemove = 60;
            break;
        default:
            cellsToRemove = 50;
    }
    
    for (int row = 0; row < BOARD_SIZE; ++row) {
        for (int col = 0; col < BOARD_SIZE; ++col) {
            mOriginalCells[row][col] = true;
        }
    }
    
    std::vector<std::pair<int, int>> cells;
    for (int row = 0; row < BOARD_SIZE; ++row) {
        for (int col = 0; col < BOARD_SIZE; ++col) {
            cells.push_back({row, col});
        }
    }
    
    std::shuffle(cells.begin(), cells.end(), mRng);
    
    for (int i = 0; i < cellsToRemove && i < cells.size(); ++i) {
        int row = cells[i].first;
        int col = cells[i].second;
        
        int temp = mBoard[row][col];
        mBoard[row][col] = EMPTY_CELL;
        mOriginalCells[row][col] = false;
        auto boardCopy = mBoard;
        
        if (!SolveBoard(boardCopy)) {
            mBoard[row][col] = temp;
            mOriginalCells[row][col] = true;
        }
    }
}

bool SudokuBoard::SolveBoard(std::array<std::array<int, BOARD_SIZE>, BOARD_SIZE>& board) {
    int row = -1;
    int col = -1;
    bool isEmpty = false;
    
    for (int r = 0; r < BOARD_SIZE; ++r) {
        for (int c = 0; c < BOARD_SIZE; ++c) {
            if (board[r][c] == EMPTY_CELL) {
                row = r;
                col = c;
                isEmpty = true;
                break;
            }
        }
        if (isEmpty) {
            break;
        }
    }
    
    if (!isEmpty) {
        return true;
    }
    
    std::vector<int> numbers(BOARD_SIZE);
    for (int i = 0; i < BOARD_SIZE; ++i) {
        numbers[i] = i + 1;
    }
    std::shuffle(numbers.begin(), numbers.end(), mRng);
    
    for (int num : numbers) {
        bool isValid = true;
        
        for (int c = 0; c < BOARD_SIZE; ++c) {
            if (board[row][c] == num) {
                isValid = false;
                break;
            }
        }
        
        if (!isValid) {
            continue;
        }
        
        for (int r = 0; r < BOARD_SIZE; ++r) {
            if (board[r][col] == num) {
                isValid = false;
                break;
            }
        }
        
        if (!isValid) {
            continue;
        }
        
        int boxRowStart = (row / BOX_SIZE) * BOX_SIZE;
        int boxColStart = (col / BOX_SIZE) * BOX_SIZE;
        
        for (int r = boxRowStart; r < boxRowStart + BOX_SIZE; ++r) {
            for (int c = boxColStart; c < boxColStart + BOX_SIZE; ++c) {
                if (board[r][c] == num) {
                    isValid = false;
                    break;
                }
            }
            if (!isValid) {
                break;
            }
        }
        
        if (!isValid) {
            continue;
        }
        
        board[row][col] = num;
        
        if (SolveBoard(board)) {
            return true;
        }
        
        board[row][col] = EMPTY_CELL;
    }
    return false;
}

bool SudokuBoard::IsValidInRow(int row, int value) const {
    int count = 0;
    for (int col = 0; col < BOARD_SIZE; ++col) {
        if (mBoard[row][col] == value) {
            count++;
        }
    }
    return count == 1;
}

bool SudokuBoard::IsValidInColumn(int col, int value) const {
    int count = 0;
    for (int row = 0; row < BOARD_SIZE; ++row) {
        if (mBoard[row][col] == value) {
            count++;
        }
    }
    return count == 1;
}

bool SudokuBoard::IsValidInBox(int boxRow, int boxCol, int value) const {
    int count = 0;
    int rowStart = boxRow * BOX_SIZE;
    int colStart = boxCol * BOX_SIZE;
    
    for (int row = rowStart; row < rowStart + BOX_SIZE; ++row) {
        for (int col = colStart; col < colStart + BOX_SIZE; ++col) {
            if (mBoard[row][col] == value) {
                count++;
            }
        }
    }   
    return count == 1;
}

bool SudokuBoard::IsNumberValid(int row, int col) const {
    if (row < 0 || row >= BOARD_SIZE || col < 0 || col >= BOARD_SIZE) {
        return false;
    }
    
    int value = mBoard[row][col];
    if (value == EMPTY_CELL) {
        return true;
    }
    
    for (int c = 0; c < BOARD_SIZE; ++c) {
        if (c != col && mBoard[row][c] == value) {
            return false;
        }
    }
    
    for (int r = 0; r < BOARD_SIZE; ++r) {
        if (r != row && mBoard[r][col] == value) {
            return false;
        }
    }
    
    int boxRowStart = (row / BOX_SIZE) * BOX_SIZE;
    int boxColStart = (col / BOX_SIZE) * BOX_SIZE;
    
    for (int r = boxRowStart; r < boxRowStart + BOX_SIZE; ++r) {
        for (int c = boxColStart; c < boxColStart + BOX_SIZE; ++c) {
            if ((r != row || c != col) && mBoard[r][c] == value) {
                return false;
            }
        }
    }
    return true;
} 