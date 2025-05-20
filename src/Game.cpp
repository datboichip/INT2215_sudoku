#include "Game.h"
#include <iostream>
#include <string>
#include <SDL.h>
#include <SDL_ttf.h>
#include <stdexcept>
#include <chrono>
#include <iomanip>
#include <sstream>

Game::Game()
    : mWindow(nullptr)
    , mRenderer(nullptr)
    , mFont(nullptr)
    , mTitleFont(nullptr)
    , mIsRunning(true)
    , mGameState(GameState::MENU)
    , mSelectedRow(-1)
    , mSelectedCol(-1)
    , mIsEditing(false)
    , mSelectedDifficulty(2)
    , mWinScreenTimer(0.0f)
    , mMistakes(0)
    , mGameOverTimer(0.0f)
    , mGameTime(0.0f)
    , mTimerActive(false)
{
}

Game::~Game() {
    Shutdown();
}

bool Game::Initialize() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        return false;
    }

    if (TTF_Init() != 0) {
        SDL_Quit();
        return false;
    }

    mWindow = SDL_CreateWindow(
        "Sudoku",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        0
    );

    if (!mWindow) {
        TTF_Quit();
        SDL_Quit();
        return false;
    }

    mRenderer = SDL_CreateRenderer(
        mWindow,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );

    if (!mRenderer) {
        SDL_DestroyWindow(mWindow);
        TTF_Quit();
        SDL_Quit();
        return false;
    }

    mFont = TTF_OpenFont("fonts/arial.ttf", 24);
    if (!mFont) {
        mFont = TTF_OpenFont("C:/Windows/Fonts/arial.ttf", 24);
        if (!mFont) {
            SDL_DestroyRenderer(mRenderer);
            SDL_DestroyWindow(mWindow);
            TTF_Quit();
            SDL_Quit();
            return false;
        }
    }

    mTitleFont = TTF_OpenFont("fonts/arial.ttf", 72);
    if (!mTitleFont) {
        mTitleFont = TTF_OpenFont("C:/Windows/Fonts/arial.ttf", 72);
        if (!mTitleFont) {
            TTF_CloseFont(mFont);
            SDL_DestroyRenderer(mRenderer);
            SDL_DestroyWindow(mWindow);
            TTF_Quit();
            SDL_Quit();
            return false;
        }
    }
    
    return true;
}

void Game::Shutdown() {
    if (mTitleFont) {
        TTF_CloseFont(mTitleFont);
        mTitleFont = nullptr;
    }

    if (mFont) {
        TTF_CloseFont(mFont);
        mFont = nullptr;
    }
    
    if (mRenderer) {
        SDL_DestroyRenderer(mRenderer);
        mRenderer = nullptr;
    }
    
    if (mWindow) {
        SDL_DestroyWindow(mWindow);
        mWindow = nullptr;
    }
    
    TTF_Quit();
    SDL_Quit();
}

void Game::RunLoop() {
    Uint32 lastTicks = SDL_GetTicks();
    
    while (mIsRunning) {
        Uint32 currentTicks = SDL_GetTicks();
        float deltaTime = (currentTicks - lastTicks) / 1000.0f;
        lastTicks = currentTicks;
        
        ProcessInput();
        Update(deltaTime);
        Render();
    }
}

void Game::ProcessInput() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                mIsRunning = false;
                break;
                
            case SDL_MOUSEBUTTONDOWN:
                if (event.button.button == SDL_BUTTON_LEFT) {
                    HandleMouseClick(event.button.x, event.button.y);
                }
                break;
                
            case SDL_KEYDOWN:
                if (mGameState == GameState::PLAYING && mSelectedRow >= 0 && mSelectedCol >= 0) {
                    if (event.key.keysym.sym >= SDLK_1 && event.key.keysym.sym <= SDLK_9) {
                        int number = event.key.keysym.sym - SDLK_0;
                        if (!mBoard.IsOriginalCell(mSelectedRow, mSelectedCol)) {
                            if (mBoard.IsValidMove(mSelectedRow, mSelectedCol, number)) {
                                mBoard.SetCell(mSelectedRow, mSelectedCol, number);
                            } else {
                                AddMistake();
                            }
                        }
                    }
                    else if (event.key.keysym.sym == SDLK_BACKSPACE || event.key.keysym.sym == SDLK_DELETE) {
                        if (!mBoard.IsOriginalCell(mSelectedRow, mSelectedCol)) {
                            mBoard.ClearCell(mSelectedRow, mSelectedCol);
                        }
                    }
                    else if (event.key.keysym.sym == SDLK_h) {
                        int hintRow, hintCol, hintValue;
                        if (mBoard.GetHint(hintRow, hintCol, hintValue)) {
                            mSelectedRow = hintRow;
                            mSelectedCol = hintCol;
                            mBoard.SetCell(hintRow, hintCol, hintValue);
                        }
                    }
                    else if (event.key.keysym.sym == SDLK_m) {
                        mGameState = GameState::MENU;
                        mSelectedRow = -1;
                        mSelectedCol = -1;
                        mTimerActive = false;
                    }
                }
                else if (mGameState == GameState::WIN || mGameState == GameState::GAME_OVER) {
                    mGameState = GameState::MENU;
                }
                break;
        }
    }
}

void Game::AddMistake() {
    mMistakes++;
    
    if (mMistakes >= MAX_MISTAKES) {
        mGameState = GameState::GAME_OVER;
        mGameOverTimer = 0.0f;
    }
}

void Game::Update(float deltaTime) {
    if (mTimerActive && mGameState == GameState::PLAYING) {
        mGameTime += deltaTime;
    }
    
    if (mGameState == GameState::PLAYING && mBoard.IsSolved()) {
        mGameState = GameState::WIN;
        mWinScreenTimer = 0.0f;
        mTimerActive = false;
    }
    
    if (mGameState == GameState::WIN) {
        mWinScreenTimer += deltaTime;
    }
    
    if (mGameState == GameState::GAME_OVER) {
        mGameOverTimer += deltaTime;
        mTimerActive = false;
    }
}

void Game::Render() {
    SDL_SetRenderDrawColor(mRenderer, 240, 240, 240, 255);
    SDL_RenderClear(mRenderer);

    switch (mGameState) {
        case GameState::MENU:
            DrawMenu();
            break;
        case GameState::PLAYING:
        case GameState::SOLVED:
        case GameState::WIN:
        case GameState::GAME_OVER:
            DrawSelection();
            DrawGrid();
            DrawNumbers();
            
            if (mGameState == GameState::PLAYING || mGameState == GameState::SOLVED) {
                DrawUI();
                DrawTimer();
            }
            
            if (mGameState == GameState::WIN) {
                DrawWinScreen();
            } else if (mGameState == GameState::GAME_OVER) {
                DrawGameOverScreen();
            }
            break;
    }

    SDL_RenderPresent(mRenderer);
}

void Game::DrawMenu() {
    SDL_Rect titleRect = {
        WINDOW_WIDTH / 2 - 200,
        30,
        400,
        100
    };
    SDL_SetRenderDrawColor(mRenderer, 100, 100, 200, 255);
    SDL_RenderFillRect(mRenderer, &titleRect);
    
    SDL_Color textColor = {255, 255, 255, 255};
    RenderCenteredTextWithFont("SUDOKU", titleRect, textColor, mTitleFont);
    
    const char* difficulties[] = {"Easy", "Medium", "Hard"};
    for (int i = 0; i < 3; ++i) {
        SDL_Rect diffBtn = {
            WINDOW_WIDTH / 2 - 100,
            180 + i * 80,
            200,
            60
        };
        
        if (i + 1 == mSelectedDifficulty) {
            SDL_SetRenderDrawColor(mRenderer, 100, 200, 100, 255);
        } else {
            SDL_SetRenderDrawColor(mRenderer, 150, 150, 150, 255);
        }
        
        SDL_RenderFillRect(mRenderer, &diffBtn);
        
        SDL_Color btnTextColor = {0, 0, 0, 255};
        RenderCenteredText(difficulties[i], diffBtn, btnTextColor);
    }
    
    SDL_Rect startBtn = {
        WINDOW_WIDTH / 2 - 120,
        450,
        240,
        70
    };
    SDL_SetRenderDrawColor(mRenderer, 100, 200, 100, 255);
    SDL_RenderFillRect(mRenderer, &startBtn);
    
    SDL_Color startTextColor = {0, 0, 0, 255};
    RenderCenteredText("Start Game", startBtn, startTextColor);
}

void Game::DrawWinScreen() {
    SDL_SetRenderDrawBlendMode(mRenderer, SDL_BLENDMODE_BLEND);
    
    SDL_SetRenderDrawColor(mRenderer, 80, 80, 180, 180);
    SDL_Rect bgRect = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
    SDL_RenderFillRect(mRenderer, &bgRect);
    
    SDL_SetRenderDrawColor(mRenderer, 255, 240, 200, 230);
    SDL_Rect congratsBox = {
        WINDOW_WIDTH / 2 - 200,
        WINDOW_HEIGHT / 2 - 150,
        400,
        300
    };
    
    SDL_RenderFillRect(mRenderer, &congratsBox);
    
    SDL_SetRenderDrawColor(mRenderer, 200, 150, 100, 255);
    SDL_Rect borderRect = congratsBox;
    borderRect.x -= 5;
    borderRect.y -= 5;
    borderRect.w += 10;
    borderRect.h += 10;
    
    for (int i = 0; i < 5; i++) {
        SDL_RenderDrawRect(mRenderer, &borderRect);
        borderRect.x++;
        borderRect.y++;
        borderRect.w -= 2;
        borderRect.h -= 2;
    }
    
    SDL_Color congratsColor = {200, 50, 50, 255};
    SDL_Rect congratsTextRect = {
        WINDOW_WIDTH / 2 - 140,
        WINDOW_HEIGHT / 2 - 120,
        280,
        40
    };
    RenderCenteredText("CONGRATULATIONS!", congratsTextRect, congratsColor);
    
    SDL_Color textColor = {0, 0, 0, 255};
    std::string difficultyText;
    switch (mSelectedDifficulty) {
        case 1: difficultyText = "Easy"; break;
        case 2: difficultyText = "Medium"; break;
        case 3: difficultyText = "Hard"; break;
        default: difficultyText = "Custom"; break;
    }
    
    SDL_Rect msg1Rect = {
        WINDOW_WIDTH / 2 - 150,
        WINDOW_HEIGHT / 2 - 30,
        300,
        30
    };
    RenderCenteredText("You have completed the", msg1Rect, textColor);
    
    SDL_Rect msg2Rect = {
        WINDOW_WIDTH / 2 - 150,
        WINDOW_HEIGHT / 2 + 10,
        300,
        30
    };
    RenderCenteredText(difficultyText + " difficulty puzzle!", msg2Rect, textColor);
    
    int totalSeconds = static_cast<int>(mGameTime);
    std::string timeText = "Time: " + FormatTime(totalSeconds);
    
    SDL_Rect timeRect = {
        WINDOW_WIDTH / 2 - 100,
        WINDOW_HEIGHT / 2 + 50,
        200,
        30
    };
    RenderCenteredText(timeText, timeRect, textColor);
    
    SDL_Rect menuBtn = {
        WINDOW_WIDTH / 2 - 100,
        WINDOW_HEIGHT / 2 + 100,
        200,
        60
    };
    
    SDL_SetRenderDrawColor(mRenderer, 100, 200, 100, 255);
    SDL_RenderFillRect(mRenderer, &menuBtn);
    
    SDL_Color btnTextColor = {0, 0, 0, 255};
    RenderCenteredText("Back to Menu", menuBtn, btnTextColor);
    
    SDL_SetRenderDrawBlendMode(mRenderer, SDL_BLENDMODE_NONE);
}

void Game::DrawGameOverScreen() {
    SDL_SetRenderDrawBlendMode(mRenderer, SDL_BLENDMODE_BLEND);
    
    SDL_SetRenderDrawColor(mRenderer, 120, 50, 50, 180);
    SDL_Rect bgRect = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
    SDL_RenderFillRect(mRenderer, &bgRect);
    
    SDL_SetRenderDrawColor(mRenderer, 50, 50, 50, 230);
    SDL_Rect gameOverBox = {
        WINDOW_WIDTH / 2 - 200,
        WINDOW_HEIGHT / 2 - 150,
        400,
        300
    };
    
    SDL_RenderFillRect(mRenderer, &gameOverBox);
    
    SDL_SetRenderDrawColor(mRenderer, 150, 50, 50, 255);
    SDL_Rect borderRect = gameOverBox;
    borderRect.x -= 3;
    borderRect.y -= 3;
    borderRect.w += 6;
    borderRect.h += 6;
    
    for (int i = 0; i < 3; i++) {
        SDL_RenderDrawRect(mRenderer, &borderRect);
        borderRect.x++;
        borderRect.y++;
        borderRect.w -= 2;
        borderRect.h -= 2;
    }
    
    SDL_Color gameOverColor = {255, 50, 50, 255};
    SDL_Rect gameOverTextRect = {
        WINDOW_WIDTH / 2 - 100,
        WINDOW_HEIGHT / 2 - 100,
        200,
        40
    };
    RenderCenteredText("GAME OVER", gameOverTextRect, gameOverColor);
    
    SDL_Color textColor = {200, 200, 200, 255};
    SDL_Rect msg1Rect = {
        WINDOW_WIDTH / 2 - 150,
        WINDOW_HEIGHT / 2 - 30,
        300,
        30
    };
    RenderCenteredText("You made " + std::to_string(MAX_MISTAKES) + " mistakes!", msg1Rect, textColor);
    
    SDL_Rect msg2Rect = {
        WINDOW_WIDTH / 2 - 150,
        WINDOW_HEIGHT / 2 + 10,
        300,
        30
    };
    RenderCenteredText("Better luck next time!", msg2Rect, textColor);
    
    SDL_Rect menuBtn = {
        WINDOW_WIDTH / 2 - 100,
        WINDOW_HEIGHT / 2 + 80,
        200,
        60
    };
    
    SDL_SetRenderDrawColor(mRenderer, 150, 50, 50, 255);
    SDL_RenderFillRect(mRenderer, &menuBtn);
    
    SDL_Color btnTextColor = {255, 255, 255, 255};
    RenderCenteredText("Try Again", menuBtn, btnTextColor);
    
    SDL_SetRenderDrawBlendMode(mRenderer, SDL_BLENDMODE_NONE);
}

void Game::DrawGrid() {
    SDL_SetRenderDrawColor(mRenderer, 0, 0, 0, 255);
    
    for (int i = 0; i <= SudokuBoard::BOARD_SIZE; ++i) {
        int thickness = (i % 3 == 0) ? 3 : 1;
        
        SDL_Rect hLine = {
            BOARD_OFFSET_X,
            BOARD_OFFSET_Y + i * CELL_SIZE,
            BOARD_SIZE,
            thickness
        };
        
        SDL_RenderFillRect(mRenderer, &hLine);
    }
    
    for (int i = 0; i <= SudokuBoard::BOARD_SIZE; ++i) {
        int thickness = (i % 3 == 0) ? 3 : 1;
        
        SDL_Rect vLine = {
            BOARD_OFFSET_X + i * CELL_SIZE,
            BOARD_OFFSET_Y,
            thickness,
            BOARD_SIZE
        };
        
        SDL_RenderFillRect(mRenderer, &vLine);
    }
}

void Game::DrawNumbers() {
    for (int row = 0; row < SudokuBoard::BOARD_SIZE; ++row) {
        for (int col = 0; col < SudokuBoard::BOARD_SIZE; ++col) {
            int value = mBoard.GetCell(row, col);
            if (value != SudokuBoard::EMPTY_CELL) {
                SDL_Rect cellRect = {
                    BOARD_OFFSET_X + col * CELL_SIZE,
                    BOARD_OFFSET_Y + row * CELL_SIZE,
                    CELL_SIZE,
                    CELL_SIZE
                };
                
                SDL_Color textColor;
                if (mBoard.IsOriginalCell(row, col)) {
                    textColor = {0, 0, 0, 255};
                } else if (!mBoard.IsNumberValid(row, col)) {
                    textColor = {255, 0, 0, 255};
                } else {
                    textColor = {0, 0, 255, 255};
                }
                
                RenderCenteredText(std::to_string(value), cellRect, textColor);
            }
        }
    }
}

void Game::DrawSelection() {
    if (mSelectedRow >= 0 && mSelectedCol >= 0) {
        SDL_SetRenderDrawColor(mRenderer, 225, 225, 225, 255);
        
        SDL_Rect rowHighlight = {
            BOARD_OFFSET_X + 1,
            BOARD_OFFSET_Y + mSelectedRow * CELL_SIZE + 1,
            BOARD_SIZE - 2,
            CELL_SIZE - 2
        };
        SDL_RenderFillRect(mRenderer, &rowHighlight);
        
        SDL_Rect colHighlight = {
            BOARD_OFFSET_X + mSelectedCol * CELL_SIZE + 1,
            BOARD_OFFSET_Y + 1,
            CELL_SIZE - 2,
            BOARD_SIZE - 2
        };
        SDL_RenderFillRect(mRenderer, &colHighlight);
        
        SDL_SetRenderDrawColor(mRenderer, 210, 210, 210, 255);
        SDL_Rect cellHighlight = {
            BOARD_OFFSET_X + mSelectedCol * CELL_SIZE + 1,
            BOARD_OFFSET_Y + mSelectedRow * CELL_SIZE + 1,
            CELL_SIZE - 2,
            CELL_SIZE - 2
        };
        SDL_RenderFillRect(mRenderer, &cellHighlight);
    }
}

void Game::DrawUI() {
    SDL_SetRenderDrawColor(mRenderer, 200, 200, 200, 255);
    
    SDL_Rect menuButton = {
        BOARD_OFFSET_X,
        BOARD_OFFSET_Y + BOARD_SIZE + 30,
        150,
        50
    };
    SDL_RenderFillRect(mRenderer, &menuButton);
    SDL_Color menuTextColor = {0, 0, 0, 255};
    RenderCenteredText("Menu", menuButton, menuTextColor);
    
    SDL_Rect hintButton = {
        BOARD_OFFSET_X + 200,
        BOARD_OFFSET_Y + BOARD_SIZE + 30,
        150,
        50
    };
    SDL_RenderFillRect(mRenderer, &hintButton);
    SDL_Color hintTextColor = {0, 0, 0, 255};
    RenderCenteredText("Hint", hintButton, hintTextColor);
    
    SDL_Color mistakeColor;
    if (mMistakes >= MAX_MISTAKES - 1) {
        mistakeColor = {255, 0, 0, 255};
    } else if (mMistakes >= MAX_MISTAKES - 2) {
        mistakeColor = {255, 165, 0, 255};
    } else {
        mistakeColor = {0, 0, 0, 255};
    }
    
    SDL_Rect mistakeRect = {
        BOARD_OFFSET_X + BOARD_SIZE - 150,
        BOARD_OFFSET_Y - 30,
        150,
        25
    };
    
    std::string mistakeText = "Mistakes: " + std::to_string(mMistakes) + "/" + std::to_string(MAX_MISTAKES);
    RenderText(mistakeText, mistakeRect.x, mistakeRect.y, mistakeColor);
}

void Game::RenderText(const std::string& text, int x, int y, SDL_Color color) {
    if (!mFont) {
        return;
    }
    
    SDL_Surface* textSurface = TTF_RenderText_Solid(mFont, text.c_str(), color);
    if (!textSurface) {
        return;
    }
    
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(mRenderer, textSurface);
    if (!textTexture) {
        SDL_FreeSurface(textSurface);
        return;
    }
    
    SDL_Rect renderRect = {x, y, textSurface->w, textSurface->h};
    SDL_RenderCopy(mRenderer, textTexture, NULL, &renderRect);
    
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
}

void Game::RenderCenteredTextWithFont(const std::string& text, SDL_Rect box, SDL_Color color, TTF_Font* font) {
    if (!font) {
        return;
    }
    
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, text.c_str(), color);
    if (!textSurface) {
        return;
    }
    
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(mRenderer, textSurface);
    if (!textTexture) {
        SDL_FreeSurface(textSurface);
        return;
    }
    
    int x = box.x + (box.w - textSurface->w) / 2;
    int y = box.y + (box.h - textSurface->h) / 2;
    
    SDL_Rect renderRect = {x, y, textSurface->w, textSurface->h};
    SDL_RenderCopy(mRenderer, textTexture, NULL, &renderRect);
    
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
}

void Game::RenderCenteredText(const std::string& text, SDL_Rect box, SDL_Color color) {
    RenderCenteredTextWithFont(text, box, color, mFont);
}

void Game::HandleMouseClick(int x, int y) {
    if (mGameState == GameState::MENU) {
        for (int i = 0; i < 3; ++i) {
            SDL_Rect diffBtn = {
                WINDOW_WIDTH / 2 - 100,
                180 + i * 80,
                200,
                60
            };
            
            if (x >= diffBtn.x && x < diffBtn.x + diffBtn.w &&
                y >= diffBtn.y && y < diffBtn.y + diffBtn.h) {
                mSelectedDifficulty = i + 1;
                break;
            }
        }
        
        SDL_Rect startBtn = {
            WINDOW_WIDTH / 2 - 120,
            450,
            240,
            70
        };
        
        if (x >= startBtn.x && x < startBtn.x + startBtn.w &&
            y >= startBtn.y && y < startBtn.y + startBtn.h) {
            mBoard.NewGame(mSelectedDifficulty);
            mGameState = GameState::PLAYING;
            mSelectedRow = -1;
            mSelectedCol = -1;
            mMistakes = 0;
            mGameTime = 0.0f;
            mTimerActive = true;
        }
    }
    else if (mGameState == GameState::PLAYING) {
        if (x >= BOARD_OFFSET_X && x < BOARD_OFFSET_X + BOARD_SIZE &&
            y >= BOARD_OFFSET_Y && y < BOARD_OFFSET_Y + BOARD_SIZE) {
            
            int col = (x - BOARD_OFFSET_X) / CELL_SIZE;
            int row = (y - BOARD_OFFSET_Y) / CELL_SIZE;
            
            mSelectedRow = row;
            mSelectedCol = col;
        }
        else if (x >= BOARD_OFFSET_X && x < BOARD_OFFSET_X + 150 &&
                y >= BOARD_OFFSET_Y + BOARD_SIZE + 30 && y < BOARD_OFFSET_Y + BOARD_SIZE + 80) {
            
            mGameState = GameState::MENU;
            mSelectedRow = -1;
            mSelectedCol = -1;
            mTimerActive = false;
        }
        else if (x >= BOARD_OFFSET_X + 200 && x < BOARD_OFFSET_X + 350 &&
                y >= BOARD_OFFSET_Y + BOARD_SIZE + 30 && y < BOARD_OFFSET_Y + BOARD_SIZE + 80) {
            
            int hintRow, hintCol, hintValue;
            if (mBoard.GetHint(hintRow, hintCol, hintValue)) {
                mSelectedRow = hintRow;
                mSelectedCol = hintCol;
                mBoard.SetCell(hintRow, hintCol, hintValue);
            }
        }
        else {
            mSelectedRow = -1;
            mSelectedCol = -1;
        }
    }
    else if (mGameState == GameState::WIN) {
        SDL_Rect menuBtn = {
            WINDOW_WIDTH / 2 - 100,
            WINDOW_HEIGHT / 2 + 80,
            200,
            60
        };
        
        if (x >= menuBtn.x - 20 && x < menuBtn.x + menuBtn.w + 20 &&
            y >= menuBtn.y && y < menuBtn.y + menuBtn.h) {
            mGameState = GameState::MENU;
        }
    }
    else if (mGameState == GameState::GAME_OVER) {
        SDL_Rect menuBtn = {
            WINDOW_WIDTH / 2 - 100,
            WINDOW_HEIGHT / 2 + 80,
            200,
            60
        };
        
        if (x >= menuBtn.x - 20 && x < menuBtn.x + menuBtn.w + 20 &&
            y >= menuBtn.y && y < menuBtn.y + menuBtn.h) {
            mGameState = GameState::MENU;
        }
    }
}

std::string Game::FormatTime(int seconds) {
    int minutes = seconds / 60;
    seconds %= 60;
    
    std::stringstream ss;
    ss << std::setw(2) << std::setfill('0') << minutes << ":"
       << std::setw(2) << std::setfill('0') << seconds;
    return ss.str();
}

void Game::DrawTimer() {
    SDL_Rect timerBg = {
        WINDOW_WIDTH / 2 - 50,
        BOARD_OFFSET_Y - 40,
        100,
        30
    };
    
    SDL_SetRenderDrawColor(mRenderer, 200, 200, 200, 255);
    SDL_RenderFillRect(mRenderer, &timerBg);
    
    SDL_SetRenderDrawColor(mRenderer, 100, 100, 100, 255);
    SDL_Rect timerBorder = timerBg;
    SDL_RenderDrawRect(mRenderer, &timerBorder);
    
    int totalSeconds = static_cast<int>(mGameTime);
    std::string timerText = FormatTime(totalSeconds);
    
    SDL_Color textColor = {0, 0, 0, 255};
    RenderCenteredText(timerText, timerBg, textColor);
} 