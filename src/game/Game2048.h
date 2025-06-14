#ifndef GAME2048_H
#define GAME2048_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <array>
#include <algorithm>
#include "../gif/gif_wrapper.h"
#include <iostream>
#include <unordered_map>

enum class GameState {
    MAIN_MENU,
    VERSION_MENU,
    GAME,
    EXIT_CONFIRM,
};

enum class GameVersion {
    ORIGINAL,
    MODIFIED
};

class Game {
public:
    Game();
    void run();

private:
    // Window and state
    sf::RenderWindow window;
    GameState currentState;
    GameVersion currentVersion;
    int gridSize;

    constexpr static int MAX_GRID_SIZE = 6;

    // Animation related members
    struct NewTileAnimation {
        sf::Vector2f position;
        float progress;
    };
    std::vector<NewTileAnimation> newTileAnimations;
    float spawnAnimationDuration = 0.3f; // New tile spawn animation duration

    // Game data
    std::vector<std::vector<int>> grid;
    int score;
    bool gameOver;
    bool gameWon;
    bool winDialogShown;
    bool achievedWin;
    bool winAchievementDialogShown;
    bool isPaused; // 新增：暂停状态
    
    // Resources
    sf::Font font;
    sf::Texture tileTexture;
    sf::Texture winTexture;
    sf::Texture loseTexture;
    sf::Texture texture32768; // 32768的jpg纹理
    std::array<sf::Color, 12> tileColors;
    
    // UI Elements - Main Menu
    sf::Text titleText;
    std::array<sf::RectangleShape, 3> sizeButtons;
    std::array<sf::Text, 3> sizeButtonTexts;
    
    // UI Elements - Version Menu
    sf::Text versionTitleText;
    std::array<sf::RectangleShape, 2> versionButtons;
    std::array<sf::Text, 2> versionButtonTexts;
    
    // Game UI
    sf::Text scoreText;
    sf::Text gameOverText;
    sf::Text restartText;
    
    // Pause button UI
    sf::RectangleShape pauseButton;
    sf::Text pauseButtonText;

    // Exit confirmation UI
    sf::RectangleShape exitConfirmBackground;
    sf::RectangleShape exitConfirmBox;
    sf::Text exitConfirmText;
    sf::RectangleShape exitConfirmYesButton;
    sf::RectangleShape exitConfirmNoButton;
    sf::Text exitConfirmYesText;
    sf::Text exitConfirmNoText;

    // Win dialog UI
    sf::RectangleShape winBackground;
    sf::RectangleShape winBox;
    sf::Sprite winSprite;
    sf::Text winText;
    sf::RectangleShape winContinueButton;
    sf::RectangleShape winQuitButton;
    sf::Text winContinueText;
    sf::Text winQuitText;

    // Win achievement dialog UI
    sf::RectangleShape winAchievementBackground;
    sf::RectangleShape winAchievementBox;
    sf::Sprite winAchievementSprite;
    sf::Text winAchievementText;
    sf::RectangleShape winAchievementContinueButton;
    sf::RectangleShape winAchievementMenuButton;
    sf::Text winAchievementContinueText;
    sf::Text winAchievementMenuText;

    // Game over dialog UI
    sf::RectangleShape gameOverBackground;
    sf::RectangleShape gameOverBox;
    sf::Sprite gameOverSprite;
    sf::Text gameOverDialogText;
    sf::RectangleShape gameOverRestartButton;
    sf::RectangleShape gameOverMenuButton;
    sf::Text gameOverRestartText;
    sf::Text gameOverMenuText;

    // Pause dialog UI - 新增
    sf::RectangleShape pauseBackground;
    sf::RectangleShape pauseBox;
    sf::Sprite pauseSprite;
    sf::Text pauseText;
    sf::RectangleShape pauseContinueButton;
    sf::RectangleShape pauseMenuButton;
    sf::Text pauseContinueText;
    sf::Text pauseMenuText;

    void setupExitConfirmUI();
    void setupWinUI();
    void setupWinAchievementUI();
    void setupGameOverUI();
    void setupPauseUI(); // 新增
    void setupWinSprites();
    void drawRoundedRectangle(sf::RenderWindow& window, const sf::Vector2f& position, const sf::Vector2f& size, const sf::Color& color, float cornerRadius = 10.0f);

    // Draw black and white grids in the modified version
    sf::Color getCellBackgroundColor(int x, int y) const;
    
    // Core functions
    void processEvents();
    void update(sf::Time deltaTime);
    void render();
    
    // State rendering
    void renderMainMenu();
    void renderVersionMenu();
    void renderGame();

    void calculateGridLayout();
    sf::Vector2f getTilePosition(int x, int y) const;
    
    // Input handling
    void handleMainMenuClick(const sf::Vector2f& mousePos);
    void handleVersionMenuClick(const sf::Vector2f& mousePos);
    void handleGameInput(sf::Keyboard::Key key);
    void handleWinDialogClick(const sf::Vector2f& mousePos);
    void handleWinDialogKeyInput(sf::Keyboard::Key key);
    void handleWinAchievementDialogClick(const sf::Vector2f& mousePos);
    void handleWinAchievementDialogKeyInput(sf::Keyboard::Key key);
    void handleGameOverDialogClick(const sf::Vector2f& mousePos);
    void handleGameOverDialogKeyInput(sf::Keyboard::Key key);
    void handlePauseDialogClick(const sf::Vector2f& mousePos); // 新增
    void handlePauseDialogKeyInput(sf::Keyboard::Key key); // 新增
    
    // Game logic
    void initializeGame(int size, GameVersion version);
    void resetGame();
    void addRandomTile();
    bool moveTiles(int dx, int dy);
    bool moveTilesContinuous(int dx, int dy);
    bool isGameOver() const;
    bool isGameOver_grid() const;
    bool isGameOVer_diagonal() const;
    
    // Helper functions
    void initializeUI();
    void setupMainMenu();
    void setupVersionMenu();
    void setupTileColors();
    sf::Color getTileColor(int value) const;
    void drawGrid();

    // GIF handling functions
    bool loadGif(const std::string& filename, sf::Texture& texture);
    void animateGifOnCover(sf::RenderWindow& window, sf::Texture& gifTexture);
    void drawGifsOnGrid(sf::RenderWindow& window);
    void updateGifFrame(sf::Texture& texture, int value);

    sf::Texture gifTexture;
    sf::Texture secondGifTexture;
    float gifXPosition;
    float secondGifXPosition;
    std::vector<sf::Texture> tileGifTextures;
    sf::Clock animationClock;
    sf::Clock gifMoveClock;
    
    std::unordered_map<int, sf::Texture> tileGifTexturesMap;
    std::unordered_map<int, GifWrapper> gifWrappers;
    
    // 主菜单装饰元素 - 新增
    std::vector<sf::Texture> decorativeTextures;
    std::vector<sf::Sprite> decorativeSprites;
    std::vector<GifWrapper> decorativeGifWrappers; // 新增：装饰GIF包装器用于动画
};

#endif // GAME2048_H