#pragma once

#include <SDL.h>
#include <SDL_ttf.h>
#include <array>
#include <vector>
#include <string>
#include "SudokuBoard.h"

enum class GameState {
    MENU,
    PLAYING,
    SOLVED,
    WIN,
    GAME_OVER
};

class Game {
public:
    Game();
    ~Game();

    bool Initialize();
    void RunLoop();
    void Shutdown();

private:
    void ProcessInput();
    void Update(float deltaTime);
    void Render();
    void DrawGrid();
    void DrawNumbers();
    void DrawSelection();
    void DrawUI();
    void DrawMenu();
    void DrawWinScreen();
    void DrawGameOverScreen();
    void DrawTimer();
    void HandleMouseClick(int x, int y);
    void RenderText(const std::string& text, int x, int y, SDL_Color color);
    void RenderCenteredText(const std::string& text, SDL_Rect box, SDL_Color color);
    void RenderCenteredTextWithFont(const std::string& text, SDL_Rect box, SDL_Color color, TTF_Font* font);
    void AddMistake();
    std::string FormatTime(int seconds);

    SDL_Window* mWindow;
    SDL_Renderer* mRenderer;
    TTF_Font* mFont;
    TTF_Font* mTitleFont;
    bool mIsRunning;
    GameState mGameState;
    SudokuBoard mBoard;
    int mSelectedRow;
    int mSelectedCol;
    bool mIsEditing;
    int mSelectedDifficulty;
    float mWinScreenTimer;
    int mMistakes;
    const int MAX_MISTAKES = 5;
    float mGameOverTimer;
    float mGameTime;
    bool mTimerActive;
    
    const int WINDOW_WIDTH = 600;
    const int WINDOW_HEIGHT = 700;
    const int BOARD_SIZE = 450;
    const int BOARD_OFFSET_X = 75;
    const int BOARD_OFFSET_Y = 50;
    const int CELL_SIZE = 50;
}; 