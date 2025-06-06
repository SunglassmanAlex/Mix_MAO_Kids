#include "Game2048.h"
#include "../gif/gif_wrapper.h"
#include <stdexcept>
#include <random>
#include <sstream>
#include <iostream>
#include <SFML/Graphics.hpp>

// Window dimensions
constexpr int WINDOW_WIDTH = 800;
constexpr int WINDOW_HEIGHT = 800;
// Grid offsets
int GRID_OFFSET_X = 100;
int GRID_OFFSET_Y = 200;
// Tile size and spacing
int TILE_SIZE = 30;
int TILE_MARGIN = 5;
// Grid line properties
constexpr int GRID_LINE_THICKNESS = 4;
const sf::Color GRID_LINE_COLOR = sf::Color(119, 110, 101);

// Animation speed - 修改这里可以调整主菜单GIF移动速度
constexpr float GIF_MOVE_SPEED = 100.0f; // pixels per second (减慢了速度)

Game::Game() : window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Mix MAO Game"),
               currentState(GameState::MAIN_MENU),
               currentVersion(GameVersion::ORIGINAL),
               gridSize(4),
               score(0),
               gameOver(false),
               gameWon(false),
               winDialogShown(false),
               gifXPosition(WINDOW_WIDTH),
               secondGifXPosition(WINDOW_WIDTH + 150) { // 第二个GIF初始位置偏移
    if (!font.loadFromFile("assets/fonts/arial.ttf")) {
        throw std::runtime_error("Failed to load font");
    }
    
    setupTileColors();
    initializeUI();
    setupExitConfirmUI();
    setupWinUI();
    
    // 加载胜利图片
    if (!winTexture.loadFromFile("assets/picture/win.jpg")) {
        std::cerr << "Failed to load win.jpg" << std::endl;
    }

    // 加载主菜单封面GIF (使用主菜单背景色)
    sf::Color mainMenuBackgroundColor(187, 173, 160); // 主菜单背景色
    GifWrapper coverGif;
    if (coverGif.loadFromFile("assets/picture/2.gif", mainMenuBackgroundColor)) {
        std::cout << "Loaded cover GIF with background color" << std::endl;
        gifTexture = coverGif.getCurrentFrame();
    } else {
        std::cerr << "Failed to load cover GIF" << std::endl;
    }
    
    // 加载第二个GIF (同样的GIF，用于双重动画效果)
    GifWrapper secondCoverGif;
    if (secondCoverGif.loadFromFile("assets/picture/2.gif", mainMenuBackgroundColor)) {
        std::cout << "Loaded second cover GIF with background color" << std::endl;
        secondGifTexture = secondCoverGif.getCurrentFrame();
    } else {
        std::cerr << "Failed to load second cover GIF" << std::endl;
    }
    
    // 加载主菜单装饰动画GIF
    std::vector<std::string> decorativeFiles = {
        "assets/picture/4.gif",
        "assets/picture/8.gif", 
        "assets/picture/16.gif",
        "assets/picture/32.gif",
        "assets/picture/64.gif"
    };
    
    decorativeTextures.resize(decorativeFiles.size());
    decorativeSprites.resize(decorativeFiles.size());
    decorativeGifWrappers.resize(decorativeFiles.size());
    
    for (size_t i = 0; i < decorativeFiles.size(); ++i) {
        if (decorativeGifWrappers[i].loadFromFile(decorativeFiles[i], mainMenuBackgroundColor)) {
            decorativeTextures[i] = decorativeGifWrappers[i].getCurrentFrame();
            decorativeSprites[i].setTexture(decorativeTextures[i]);
            
            // 设置装饰图片的大小和位置 (避免与文字重合)
            float scale = 80.0f / std::max(decorativeTextures[i].getSize().x, decorativeTextures[i].getSize().y);
            decorativeSprites[i].setScale(scale, scale);
            
            // 根据索引设置不同位置
            switch (i) {
                case 0: // 左上角
                    decorativeSprites[i].setPosition(50, 50);
                    break;
                case 1: // 右上角
                    decorativeSprites[i].setPosition(WINDOW_WIDTH - 130, 50);
                    break;
                case 2: // 左下角 (避开动画GIF区域)
                    decorativeSprites[i].setPosition(50, WINDOW_HEIGHT - 200);
                    break;
                case 3: // 右下角 (避开动画GIF区域)
                    decorativeSprites[i].setPosition(WINDOW_WIDTH - 130, WINDOW_HEIGHT - 200);
                    break;
                case 4: // 左中
                    decorativeSprites[i].setPosition(50, WINDOW_HEIGHT / 2);
                    break;
            }
            
            std::cout << "Loaded decorative animated GIF: " << decorativeFiles[i] << std::endl;
        } else {
            std::cerr << "Failed to load decorative animated GIF: " << decorativeFiles[i] << std::endl;
        }
    }

    // 加载所有可能的GIF纹理
    std::vector<int> values = {2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768};
    for (int value : values) {
        std::string filename = "assets/picture/" + std::to_string(value) + ".gif";
        GifWrapper wrapper;
        
        // 获取该值对应的方块背景色
        sf::Color tileBackgroundColor = getTileColor(value);
        
        if (wrapper.loadFromFile(filename, tileBackgroundColor)) {
            std::cout << "Loaded GIF with background: " << filename << std::endl;
            tileGifTexturesMap[value] = wrapper.getCurrentFrame();
            gifWrappers[value] = std::move(wrapper);
        } else {
            std::cerr << "Failed to load GIF: " << filename << std::endl;
            // 如果加载失败，使用32768.gif作为后备
            if (value != 32768) {
                GifWrapper fallbackWrapper;
                sf::Color fallbackBackgroundColor = getTileColor(32768);
                if (fallbackWrapper.loadFromFile("assets/picture/32768.gif", fallbackBackgroundColor)) {
                    tileGifTexturesMap[value] = fallbackWrapper.getCurrentFrame();
                    gifWrappers[value] = std::move(fallbackWrapper);
                } else {
                    // 创建后备纹理
                    sf::Texture fallback;
                    fallback.create(100, 100);
                    sf::Image img;
                    img.create(100, 100, tileBackgroundColor);
                    fallback.loadFromImage(img);
                    tileGifTexturesMap[value] = fallback;
                }
            } else {
                // 创建后备纹理
                sf::Texture fallback;
                fallback.create(100, 100);
                sf::Image img;
                img.create(100, 100, tileBackgroundColor);
                fallback.loadFromImage(img);
                tileGifTexturesMap[value] = fallback;
            }
        }
    }
}

void Game::run() {
    sf::Clock clock;
    while (window.isOpen()) {
        sf::Time deltaTime = clock.restart();
        processEvents();
        update(deltaTime);
        render();
    }
}

void Game::setupTileColors() {
    tileColors = {
        sf::Color(238, 228, 218), // 2
        sf::Color(237, 224, 200), // 4
        sf::Color(242, 177, 121), // 8
        sf::Color(245, 149, 99),  // 16
        sf::Color(246, 124, 95),  // 32
        sf::Color(246, 94, 59),   // 64
        sf::Color(237, 207, 114), // 128
        sf::Color(237, 204, 97),  // 256
        sf::Color(237, 200, 80),  // 512
        sf::Color(237, 197, 63),  // 1024
        sf::Color(237, 194, 46),  // 2048
        sf::Color(60, 58, 50)     // >2048
    };
}

void Game::setupExitConfirmUI() {
    // Semi-transparent background
    exitConfirmBackground.setSize(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
    exitConfirmBackground.setFillColor(sf::Color(0, 0, 0, 150));
    
    // Confirmation box
    exitConfirmBox.setSize(sf::Vector2f(500, 200));
    exitConfirmBox.setPosition(WINDOW_WIDTH/2 - 250, WINDOW_HEIGHT/2 - 100);
    exitConfirmBox.setFillColor(sf::Color(143, 122, 102));
    
    // Confirmation text
    exitConfirmText.setFont(font);
    exitConfirmText.setString("Are you sure you want to quit?");
    exitConfirmText.setCharacterSize(28);
    exitConfirmText.setFillColor(sf::Color::White);
    exitConfirmText.setPosition(WINDOW_WIDTH/2 - 180, WINDOW_HEIGHT/2 - 70);
    
    // Yes/No buttons
    exitConfirmYesButton.setSize(sf::Vector2f(120, 50));
    exitConfirmYesButton.setPosition(WINDOW_WIDTH/2 - 140, WINDOW_HEIGHT/2 + 20);
    exitConfirmYesButton.setFillColor(sf::Color(100, 255, 100));
    
    exitConfirmNoButton.setSize(sf::Vector2f(120, 50));
    exitConfirmNoButton.setPosition(WINDOW_WIDTH/2 + 20, WINDOW_HEIGHT/2 + 20);
    exitConfirmNoButton.setFillColor(sf::Color(255, 100, 100));
    
    exitConfirmYesText.setFont(font);
    exitConfirmYesText.setString("Yes(Y)");
    exitConfirmYesText.setCharacterSize(24);
    exitConfirmYesText.setFillColor(sf::Color::White);
    exitConfirmYesText.setPosition(WINDOW_WIDTH/2 - 115, WINDOW_HEIGHT/2 + 30);
    
    exitConfirmNoText.setFont(font);
    exitConfirmNoText.setString("No(N)");
    exitConfirmNoText.setCharacterSize(24);
    exitConfirmNoText.setFillColor(sf::Color::White);
    exitConfirmNoText.setPosition(WINDOW_WIDTH/2 + 50, WINDOW_HEIGHT/2 + 30);
}

void Game::setupWinUI() {
    // Semi-transparent background
    winBackground.setSize(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
    winBackground.setFillColor(sf::Color(0, 0, 0, 150));
    
    // Win dialog box
    winBox.setSize(sf::Vector2f(600, 400));
    winBox.setPosition(WINDOW_WIDTH/2 - 300, WINDOW_HEIGHT/2 - 200);
    winBox.setFillColor(sf::Color(250, 248, 239));
    
    // Win sprite (will be set when showing)
    winSprite.setTexture(winTexture);
    float spriteScale = 200.0f / std::max(winTexture.getSize().x, winTexture.getSize().y);
    winSprite.setScale(spriteScale, spriteScale);
    winSprite.setPosition(WINDOW_WIDTH/2 - 100, WINDOW_HEIGHT/2 - 150);
    
    // "You win!!" text
    winText.setFont(font);
    winText.setString("You win!!");
    winText.setCharacterSize(48);
    winText.setFillColor(sf::Color::Red);
    winText.setPosition(WINDOW_WIDTH/2 - 120, WINDOW_HEIGHT/2 - 20);
    
    // Continue button
    winContinueButton.setSize(sf::Vector2f(150, 60));
    winContinueButton.setPosition(WINDOW_WIDTH/2 - 180, WINDOW_HEIGHT/2 + 80);
    winContinueButton.setFillColor(sf::Color(143, 122, 102));
    
    // Quit button
    winQuitButton.setSize(sf::Vector2f(150, 60));
    winQuitButton.setPosition(WINDOW_WIDTH/2 + 30, WINDOW_HEIGHT/2 + 80);
    winQuitButton.setFillColor(sf::Color(143, 122, 102));
    
    // Button texts
    winContinueText.setFont(font);
    winContinueText.setString("Continue(C)");
    winContinueText.setCharacterSize(20);
    winContinueText.setFillColor(sf::Color::White);
    winContinueText.setPosition(WINDOW_WIDTH/2 - 165, WINDOW_HEIGHT/2 + 95);
    
    winQuitText.setFont(font);
    winQuitText.setString("Quit(Esc)");
    winQuitText.setCharacterSize(20);
    winQuitText.setFillColor(sf::Color::White);
    winQuitText.setPosition(WINDOW_WIDTH/2 + 55, WINDOW_HEIGHT/2 + 95);
}

void Game::initializeUI() {
    setupMainMenu();
    setupVersionMenu();
    
    // Score display text
    scoreText.setFont(font);
    scoreText.setCharacterSize(32);
    scoreText.setFillColor(sf::Color::White);
    scoreText.setPosition(20, 20);
    
    // Game over text
    gameOverText.setFont(font);
    gameOverText.setString("Game Over!");
    gameOverText.setCharacterSize(48);
    gameOverText.setFillColor(sf::Color::Red);
    gameOverText.setPosition(WINDOW_WIDTH/2 - 120, WINDOW_HEIGHT/2 - 50);
    
    // Restart prompt text
    restartText.setFont(font);
    restartText.setString("Press R to Restart");
    restartText.setCharacterSize(24);
    restartText.setFillColor(sf::Color::White);
    restartText.setPosition(WINDOW_WIDTH/2 - 100, WINDOW_HEIGHT/2 + 20);
}

void Game::setupMainMenu() {
    // Main menu title
    titleText.setFont(font);
    titleText.setString("2048 Game");
    titleText.setCharacterSize(64);
    titleText.setFillColor(sf::Color::White);
    titleText.setPosition(280, 100);
    
    const std::array<std::string, 3> sizeLabels = {"4 x 4", "5 x 5", "6 x 6"};
    const std::array<sf::Color, 3> buttonColors = {
        sf::Color(143, 122, 102),
        sf::Color(143, 122, 102),
        sf::Color(143, 122, 102)
    };
    
    for (size_t i = 0; i < sizeButtons.size(); ++i) {
        // Grid size selection buttons
        sizeButtons[i].setSize(sf::Vector2f(200, 80));
        sizeButtons[i].setPosition(300, 250 + i * 120);
        sizeButtons[i].setFillColor(buttonColors[i]);
        
        // Button text
        sizeButtonTexts[i].setFont(font);
        sizeButtonTexts[i].setString(sizeLabels[i]);
        sizeButtonTexts[i].setCharacterSize(32);
        sizeButtonTexts[i].setFillColor(sf::Color::White);
        sf::FloatRect textRect = sizeButtonTexts[i].getLocalBounds();
        sizeButtonTexts[i].setOrigin(textRect.left + textRect.width/2.0f,
                                    textRect.top + textRect.height/2.0f);
        sizeButtonTexts[i].setPosition(300 + 100, 250 + i * 120 + 40);
    }
}

void Game::setupVersionMenu() {
    // Version selection title
    versionTitleText.setFont(font);
    versionTitleText.setString("Choose Version");
    versionTitleText.setCharacterSize(48);
    versionTitleText.setFillColor(sf::Color::White);
    versionTitleText.setPosition(280, 100);
    
    const std::array<std::string, 2> versionLabels = {
        "Original Version (Arrow Keys)",
        "Diagonal Only (Q/E/Z/C Keys)"
    };
    
    for (size_t i = 0; i < versionButtons.size(); ++i) {
        // Version selection buttons
        versionButtons[i].setSize(sf::Vector2f(400, 80));
        versionButtons[i].setPosition(200, 250 + i * 120);
        versionButtons[i].setFillColor(sf::Color(143, 122, 102));
        
        // Button text
        versionButtonTexts[i].setFont(font);
        versionButtonTexts[i].setString(versionLabels[i]);
        versionButtonTexts[i].setCharacterSize(28);
        versionButtonTexts[i].setFillColor(sf::Color::White);
        sf::FloatRect textRect = versionButtonTexts[i].getLocalBounds();
        versionButtonTexts[i].setOrigin(textRect.left + textRect.width/2.0f,
                                       textRect.top + textRect.height/2.0f);
        versionButtonTexts[i].setPosition(200 + 200, 250 + i * 120 + 40);
    }
}

void Game::processEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
        // Window close event
        if (event.type == sf::Event::Closed) {
            currentState = GameState::EXIT_CONFIRM;
        }

        // Keyboard input
        if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::Escape) {
                currentState = GameState::EXIT_CONFIRM;
            }

            // Exit confirmation handling
            if (currentState == GameState::EXIT_CONFIRM) {
                if (event.key.code == sf::Keyboard::Y) {
                    window.close();
                } else if (event.key.code == sf::Keyboard::N) {
                    if (grid.empty()) {
                        currentState = GameState::MAIN_MENU;
                    } else {
                        currentState = GameState::GAME;
                    }
                }
            }

            // Direction keys and R key logic: only in game state
            if (currentState == GameState::GAME) {
                if (event.key.code == sf::Keyboard::R) {
                    resetGame();
                } else if (!gameOver) {
                    handleGameInput(event.key.code);
                }
            }
        }

        // Mouse click events
        if (event.type == sf::Event::MouseButtonPressed) {
            sf::Vector2f mousePos(event.mouseButton.x, event.mouseButton.y);

            if (currentState == GameState::EXIT_CONFIRM) {
                if (exitConfirmYesButton.getGlobalBounds().contains(mousePos)) {
                    window.close();
                } else if (exitConfirmNoButton.getGlobalBounds().contains(mousePos)) {
                    if (grid.empty()) {
                        currentState = GameState::MAIN_MENU;
                    } else {
                        currentState = GameState::GAME;
                    }
                }
                continue;
            }

            // 胜利界面点击处理
            if (currentState == GameState::GAME && gameWon && winDialogShown) {
                handleWinDialogClick(mousePos);
                continue;
            }

            // Menu click logic
            if (currentState == GameState::MAIN_MENU) {
                handleMainMenuClick(mousePos);
            } else if (currentState == GameState::VERSION_MENU) {
                handleVersionMenuClick(mousePos);
            }
        }
    }
}

void Game::handleMainMenuClick(const sf::Vector2f& pos) {
    for (size_t i = 0; i < sizeButtons.size(); ++i) {
        if (sizeButtons[i].getGlobalBounds().contains(pos)) {
            gridSize = 4 + i;
            currentState = GameState::VERSION_MENU;
            return;
        }
    }
}

void Game::handleVersionMenuClick(const sf::Vector2f& pos) {
    for (size_t i = 0; i < versionButtons.size(); ++i) {
        if (versionButtons[i].getGlobalBounds().contains(pos)) {
            currentVersion = static_cast<GameVersion>(i);
            initializeGame(gridSize, currentVersion);
            currentState = GameState::GAME;
            return;
        }
    }
}

void Game::handleGameInput(sf::Keyboard::Key key) {
    // 如果显示胜利对话框，优先处理胜利界面的输入
    if (gameWon && winDialogShown) {
        handleWinDialogKeyInput(key);
        return;
    }
    
    bool moved = false;
    
    if (currentVersion == GameVersion::ORIGINAL) {
        // Original version: only arrow keys
        switch (key) {
            case sf::Keyboard::Up:
                moved = moveTiles(0, -1);
                break;
            case sf::Keyboard::Down:
                moved = moveTiles(0, 1);
                break;
            case sf::Keyboard::Left:
                moved = moveTiles(-1, 0);
                break;
            case sf::Keyboard::Right:
                moved = moveTiles(1, 0);
                break;
            default:
                break;
        }
    } else if (currentVersion == GameVersion::MODIFIED) {
        // Diagonal version: only diagonal moves
        switch (key) {
            case sf::Keyboard::Q: // Top-left
                moved = moveTiles(-1, -1);
                break;
            case sf::Keyboard::E: // Top-right
                moved = moveTiles(1, -1);
                break;
            case sf::Keyboard::Z: // Bottom-left
                moved = moveTiles(-1, 1);
                break;
            case sf::Keyboard::C: // Bottom-right
                moved = moveTiles(1, 1);
                break;
            default:
                break;
        }
    }
    
    if (moved) {
        addRandomTile();
        gameOver = isGameOver();
    }
}

void Game::handleWinDialogClick(const sf::Vector2f& mousePos) {
    // 检查Continue按钮
    if (winContinueButton.getGlobalBounds().contains(mousePos)) {
        winDialogShown = false;
        // 继续游戏，不重置gameWon状态，这样玩家可以继续玩到更高分数
        return;
    }
    
    // 检查Quit按钮
    if (winQuitButton.getGlobalBounds().contains(mousePos)) {
        currentState = GameState::MAIN_MENU;
        winDialogShown = false;
        gameWon = false;
        return;
    }
}

void Game::handleWinDialogKeyInput(sf::Keyboard::Key key) {
    if (key == sf::Keyboard::C) {
        // Continue game
        winDialogShown = false;
        return;
    }
    
    if (key == sf::Keyboard::Escape) {
        // Quit to main menu
        currentState = GameState::MAIN_MENU;
        winDialogShown = false;
        gameWon = false;
        return;
    }
}

bool Game::loadGif(const std::string& filename, sf::Texture& texture) {
    GifWrapper wrapper;
    if (wrapper.loadFromFile(filename)) {
        texture = wrapper.getCurrentFrame();
        return true;
    }
    
    // 创建后备纹理
    sf::Image fallbackImage;
    fallbackImage.create(64, 64, sf::Color(128, 128, 128, 255));
    texture.loadFromImage(fallbackImage);
    return false;
}

void Game::animateGifOnCover(sf::RenderWindow& window, sf::Texture& gifTexture) {
    // 第一个GIF (稍微加大一点)
    sf::Sprite gifSprite(gifTexture);
    float gifScale = 1.5f; // 增大GIF尺寸
    gifSprite.setScale(gifScale, gifScale);
    
    // 进一步调整Y位置，避免与左下角和右下角的装饰GIF冲突
    // 装饰GIF位置是WINDOW_HEIGHT - 200，为了避免冲突，移动GIF需要更靠上
    float gifYPosition = WINDOW_HEIGHT - (gifTexture.getSize().y * gifScale) - 250; // 从-120改为-250，再往上移动130像素
    
    gifSprite.setPosition(gifXPosition, gifYPosition);
    window.draw(gifSprite);
    
    // 第二个GIF (同样大小，在右下方一点)
    sf::Sprite secondGifSprite(secondGifTexture);
    secondGifSprite.setScale(gifScale, gifScale);
    
    // 第二个GIF的Y位置稍微向下偏移，但仍要避免与装饰GIF冲突
    float secondGifYPosition = gifYPosition + 30; // 从+40改为+30，进一步减少偏移量
    
    secondGifSprite.setPosition(secondGifXPosition, secondGifYPosition);
    window.draw(secondGifSprite);

    // 仅当在主菜单时才更新位置
    if (currentState == GameState::MAIN_MENU) {
        float deltaTime = gifMoveClock.restart().asSeconds();
        
        // 两个GIF以相同速度移动
        gifXPosition -= GIF_MOVE_SPEED * deltaTime;
        secondGifXPosition -= GIF_MOVE_SPEED * deltaTime;
        
        // 当第一个GIF移出左边界时重置位置
        if (gifXPosition < -static_cast<float>(gifTexture.getSize().x * gifScale)) {
            gifXPosition = WINDOW_WIDTH;
        }
        
        // 当第二个GIF移出左边界时重置位置
        if (secondGifXPosition < -static_cast<float>(secondGifTexture.getSize().x * gifScale)) {
            secondGifXPosition = WINDOW_WIDTH + 150; // 保持150像素的间距
        }
    }
}

void Game::drawGifsOnGrid(sf::RenderWindow& window) {
    for (int y = 0; y < gridSize; ++y) {
        for (int x = 0; x < gridSize; ++x) {
            int tileValue = grid[y][x];
            if (tileValue > 0) {
                // 使用映射表获取纹理
                auto it = tileGifTexturesMap.find(tileValue);
                if (it == tileGifTexturesMap.end()) {
                    // 如果找不到，使用32768的纹理
                    it = tileGifTexturesMap.find(32768);
                    if (it == tileGifTexturesMap.end()) {
                        continue; // 如果还是找不到，跳过
                    }
                }
                
                sf::Sprite tileSprite(it->second);
                sf::Vector2f tilePos = getTilePosition(x, y);
                
                // 缩放GIF以适合方块
                float scaleX = static_cast<float>(TILE_SIZE + TILE_MARGIN) / it->second.getSize().x;
                float scaleY = static_cast<float>(TILE_SIZE + TILE_MARGIN) / it->second.getSize().y;
                tileSprite.setScale(scaleX, scaleY);
                
                tileSprite.setPosition(tilePos);
                window.draw(tileSprite);
            }
        }
    }
}

void Game::update(sf::Time deltaTime) {
    // 更新动画
    for (auto it = newTileAnimations.begin(); it != newTileAnimations.end();) {
        it->progress += (1.0f / (60.0f * spawnAnimationDuration));
        if (it->progress >= 1.0f) {
            it = newTileAnimations.erase(it);
        } else {
            ++it;
        }
    }

    // 更新所有GIF动画
    for (auto& [value, wrapper] : gifWrappers) {
        wrapper.updateFrame();
        // 总是更新纹理，不管是否是动画
        tileGifTexturesMap[value] = wrapper.getCurrentFrame();
    }
    
    // 更新装饰GIF动画
    for (size_t i = 0; i < decorativeGifWrappers.size(); ++i) {
        decorativeGifWrappers[i].updateFrame();
        decorativeTextures[i] = decorativeGifWrappers[i].getCurrentFrame();
        decorativeSprites[i].setTexture(decorativeTextures[i]);
    }

    // 更新主菜单GIF动画 (这里也会被animateGifOnCover函数处理，但保留备用)
    if (currentState == GameState::MAIN_MENU) {
        // GIF动画位置更新在animateGifOnCover函数中处理
        // 这里可以添加其他主菜单相关的更新逻辑
    }
}

void Game::render() {
    window.clear();

    if (currentState == GameState::MAIN_MENU) {
        renderMainMenu();
        animateGifOnCover(window, gifTexture);
    } else if (currentState == GameState::VERSION_MENU) {
        renderVersionMenu();
    } else if (currentState == GameState::GAME) {
        renderGame(); // 这里会绘制网格和数字
        // 不再调用drawGifsOnGrid，因为我们在renderGame中已经处理了渲染
        
        // 如果胜利且显示对话框，绘制胜利界面
        if (gameWon && winDialogShown) {
            window.draw(winBackground);
            window.draw(winBox);
            window.draw(winSprite);
            window.draw(winText);
            window.draw(winContinueButton);
            window.draw(winContinueText);
            window.draw(winQuitButton);
            window.draw(winQuitText);
        }
    } else if (currentState == GameState::EXIT_CONFIRM) {
        // 根据当前状态绘制背景
        if (!grid.empty()) {
            renderGame();
        } else if (currentState == GameState::VERSION_MENU) {
            renderVersionMenu();
        } else {
            renderMainMenu();
        }
        // 绘制确认对话框
        window.draw(exitConfirmBackground);
        window.draw(exitConfirmBox);
        window.draw(exitConfirmText);
        window.draw(exitConfirmYesButton);
        window.draw(exitConfirmYesText);
        window.draw(exitConfirmNoButton);
        window.draw(exitConfirmNoText);
    }

    window.display();
}

void Game::renderGame() {
    window.clear(sf::Color(187, 173, 160));
    
    // 更新分数显示
    std::stringstream ss;
    ss << "Score: " << score;
    scoreText.setString(ss.str());
    window.draw(scoreText);
    
    // 绘制网格
    drawGrid();
    
    // 绘制数字和GIF
    for (int y = 0; y < gridSize; ++y) {
        for (int x = 0; x < gridSize; ++x) {
            if (grid[y][x] != 0) {
                sf::Vector2f pos = getTilePosition(x, y);
                
                // 计算内嵌方块的尺寸和位置（留出一些边距）
                const int innerPadding = TILE_MARGIN / 2; // 内边距
                const int innerTileSize = TILE_SIZE - innerPadding * 2;
                sf::Vector2f innerPos(pos.x + innerPadding, pos.y + innerPadding);
                
                // 绘制方块背景（内嵌在格子中）
                sf::RectangleShape tile(sf::Vector2f(innerTileSize, innerTileSize));
                tile.setPosition(innerPos);
                tile.setFillColor(getTileColor(grid[y][x]));
                window.draw(tile);

                // 尝试绘制GIF（内嵌在格子中）
                auto it = tileGifTexturesMap.find(grid[y][x]);
                if (it != tileGifTexturesMap.end()) {
                    // 检查纹理尺寸
                    sf::Vector2u texSize = it->second.getSize();
                    if (texSize.x > 0 && texSize.y > 0) {
                        sf::Sprite sprite(it->second);
                        sprite.setPosition(innerPos);
                        
                        // 缩放GIF以适应内嵌方块大小
                        float scaleX = static_cast<float>(innerTileSize) / texSize.x;
                        float scaleY = static_cast<float>(innerTileSize) / texSize.y;
                        sprite.setScale(scaleX, scaleY);
                        
                        window.draw(sprite);
                    } else {
                        std::cout << "Warning: Empty texture for value " << grid[y][x] << std::endl;
                    }
                } else {
                    std::cout << "Warning: No texture found for value " << grid[y][x] << std::endl;
                }
                
                // 绘制数字（始终在左上角）
                sf::Text text;
                text.setFont(font);
                text.setString(std::to_string(grid[y][x]));
                text.setCharacterSize(innerTileSize / 4); // 根据内嵌大小调整字体
                text.setFillColor(grid[y][x] <= 4 ? sf::Color(119, 110, 101) : sf::Color::White);
                text.setPosition(innerPos.x + 3, innerPos.y + 3);
                window.draw(text);
            }
        }
    }
    
    // 如果游戏结束，显示消息
    if (gameOver) {
        window.draw(gameOverText);
        window.draw(restartText);
    }
}

void Game::renderMainMenu() {
    window.clear(sf::Color(187, 173, 160));
    
    // 绘制装饰图片
    for (const auto& sprite : decorativeSprites) {
        window.draw(sprite);
    }
    
    // 绘制主要界面元素
    window.draw(titleText);
    
    for (const auto& button : sizeButtons) {
        window.draw(button);
    }
    
    for (const auto& text : sizeButtonTexts) {
        window.draw(text);
    }
}

void Game::renderVersionMenu() {
    window.clear(sf::Color(187, 173, 160));
    window.draw(versionTitleText);
    
    for (const auto& button : versionButtons) {
        window.draw(button);
    }
    
    for (const auto& text : versionButtonTexts) {
        window.draw(text);
    }
}

sf::Vector2f Game::getTilePosition(int x, int y) const {
    return sf::Vector2f(GRID_OFFSET_X + x * (TILE_SIZE + TILE_MARGIN),
                       GRID_OFFSET_Y + y * (TILE_SIZE + TILE_MARGIN));
}

void Game::calculateGridLayout() {
    // 根据网格大小计算方块尺寸和间距
    const float maxGridWidth = WINDOW_WIDTH * 0.8f;  // 网格最大宽度为窗口宽度的80%
    const float maxGridHeight = WINDOW_HEIGHT * 0.6f; // 网格最大高度为窗口高度的60%
    
    // 计算适合的方块大小
    float tileSizeWithMargin = std::min(
        maxGridWidth / (gridSize + 0.5f),  // 加0.5f为边距留空间
        maxGridHeight / (gridSize + 0.5f)
    );
    
    // 设置方块大小和间距（间距为大小的1/5）
    TILE_SIZE = static_cast<int>(tileSizeWithMargin * 0.83f); // 方块占83%
    TILE_MARGIN = static_cast<int>(tileSizeWithMargin * 0.17f); // 间距占17%
    
    // 计算网格起始位置（居中）
    GRID_OFFSET_X = (WINDOW_WIDTH - (gridSize * (TILE_SIZE + TILE_MARGIN) + TILE_MARGIN)) / 2;
    GRID_OFFSET_Y = WINDOW_HEIGHT * 0.3f; // 将网格放在窗口上方1/3处
}

void Game::initializeGame(int size, GameVersion version) {
    gridSize = size;
    currentVersion = version;
    
    // 必须在添加方块前计算布局
    calculateGridLayout();
    
    // 初始化网格
    grid.clear();
    grid.resize(gridSize, std::vector<int>(gridSize, 0));
    
    // 重置游戏状态
    score = 0;
    gameOver = false;
    gameWon = false;
    winDialogShown = false;
    
    // 添加初始方块
    addRandomTile();
    addRandomTile();
}

void Game::resetGame() {
    initializeGame(gridSize, currentVersion);
}

void Game::addRandomTile() {
    // 找到所有空单元格
    std::vector<std::pair<int, int>> emptyCells;
    for (int y = 0; y < gridSize; ++y) {
        for (int x = 0; x < gridSize; ++x) {
            if (grid[y][x] == 0) {
                emptyCells.emplace_back(x, y);
            }
        }
    }
    
    if (emptyCells.empty()) return;
    
    // 随机选择一个空单元格
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, emptyCells.size() - 1);
    
    auto [x, y] = emptyCells[dist(gen)];
    grid[y][x] = (dist(gen) % 10 < 8) ? 2 : 4;
    
    // 添加新方块动画
    newTileAnimations.push_back({
        getTilePosition(x, y),
        0.0f // 初始进度为0
    });
}

bool Game::moveTiles(int dx, int dy) {
    bool moved = false;
    std::vector<std::vector<bool>> merged(gridSize, std::vector<bool>(gridSize, false));

    // 根据移动方向确定遍历顺序
    const bool horizontal = (dx != 0);
    const int start = (dx > 0 || dy > 0) ? gridSize - 1 : 0;
    const int step = (dx > 0 || dy > 0) ? -1 : 1;

    for (int i = 0; i < gridSize; ++i) {
        for (int j = start; j >= 0 && j < gridSize; j += step) {
            int x = horizontal ? j : i;
            int y = horizontal ? i : j;
            
            if (grid[y][x] == 0) continue;

            int newX = x;
            int newY = y;
            int prevX = x;
            int prevY = y;
            bool hasMerged = false;

            while (true) {
                int nextX = newX + dx;
                int nextY = newY + dy;

                // 边界检查
                if (nextX < 0 || nextX >= gridSize || nextY < 0 || nextY >= gridSize) break;

                // 如果目标位置为空，继续移动
                if (grid[nextY][nextX] == 0) {
                    prevX = newX;
                    prevY = newY;
                    newX = nextX;
                    newY = nextY;
                    moved = true;
                } 
                // 如果目标位置有相同值且未合并过
                else if (grid[nextY][nextX] == grid[y][x] && !merged[nextY][nextX]) {
                    merged[nextY][nextX] = true;
                    grid[nextY][nextX] *= 2;
                    score += grid[nextY][nextX];
                    
                    // 检测是否达到2048胜利条件
                    if (grid[nextY][nextX] == 2048 && !gameWon && !winDialogShown) {
                        gameWon = true;
                        winDialogShown = true;
                    }
                    
                    grid[y][x] = 0;
                    moved = true;
                    hasMerged = true;
                    break;
                } 
                // 其他情况停止移动
                else {
                    break;
                }
            }

            // 如果没有合并但移动了位置
            if (!hasMerged && (newX != x || newY != y)) {
                grid[newY][newX] = grid[y][x];
                grid[y][x] = 0;
                moved = true;
            }
        }
    }

    return moved;
}

// Placeholder for moveTilesContinuous (not implemented in original code)
bool Game::moveTilesContinuous(int dx, int dy) {
    return moveTiles(dx, dy); // Fallback to regular moveTiles
}

bool Game::isGameOver() const {
    // Check for empty tiles
    for (int y = 0; y < gridSize; ++y) {
        for (int x = 0; x < gridSize; ++x) {
            if (grid[y][x] == 0) {
                return false;
            }
        }
    }
    
    // Check for possible moves
    if (currentVersion == GameVersion::ORIGINAL) {
        // Check adjacent tiles for possible merges
        for (int y = 0; y < gridSize; ++y) {
            for (int x = 0; x < gridSize; ++x) {
                int value = grid[y][x];
                
                // Check right
                if (x < gridSize - 1 && grid[y][x + 1] == value) {
                    return false;
                }
                
                // Check down
                if (y < gridSize - 1 && grid[y + 1][x] == value) {
                    return false;
                }
            }
        }
    } else if (currentVersion == GameVersion::MODIFIED) {
        // Check diagonal moves for possible merges
        for (int y = 0; y < gridSize; ++y) {
            for (int x = 0; x < gridSize; ++x) {
                int value = grid[y][x];
                
                // Check top-right
                if (x < gridSize - 1 && y > 0 && grid[y - 1][x + 1] == value) {
                    return false;
                }
                
                // Check bottom-right
                if (x < gridSize - 1 && y < gridSize - 1 && grid[y + 1][x + 1] == value) {
                    return false;
                }
                
                // Check bottom-left
                if (x > 0 && y < gridSize - 1 && grid[y + 1][x - 1] == value) {
                    return false;
                }
                
                // Check top-left
                if (x > 0 && y > 0 && grid[y - 1][x - 1] == value) {
                    return false;
                }
            }
        }
    }
    
    return true;
}

// Placeholder for isGameOver_grid (not implemented in original code)
bool Game::isGameOver_grid() const {
    return isGameOver(); // Fallback to isGameOver
}

// Placeholder for isGameOVer_diagonal (not implemented in original code)
bool Game::isGameOVer_diagonal() const {
    return isGameOver(); // Fallback to isGameOver
}

sf::Color Game::getTileColor(int value) const {
    if (value <= 0) return sf::Color::Transparent;
    
    // Calculate index
    int index = static_cast<int>(std::log2(value)) - 1;
    
    // Ensure index is within bounds
    if (index >= 0 && index < static_cast<int>(tileColors.size())) {
        return tileColors[index];
    }
    
    // Return last color for values beyond range
    return tileColors.back();
}

sf::Color Game::getCellBackgroundColor(int x, int y) const {
    // Return checkerboard pattern for modified version
    if ((x + y) % 2 == 0) {
        return sf::Color(205, 193, 171);
    } else {
        return sf::Color(205, 193, 180);
    }
}

void Game::drawGrid() {
    // 计算网格的实际尺寸（不包含多余的边距）
    int gridWidth = gridSize * (TILE_SIZE + TILE_MARGIN) - TILE_MARGIN;
    int gridHeight = gridSize * (TILE_SIZE + TILE_MARGIN) - TILE_MARGIN;
    
    // 绘制整体背景
    sf::RectangleShape background(sf::Vector2f(gridWidth + TILE_MARGIN * 2, gridHeight + TILE_MARGIN * 2));
    background.setPosition(GRID_OFFSET_X - TILE_MARGIN, GRID_OFFSET_Y - TILE_MARGIN);
    background.setFillColor(sf::Color(187, 173, 160));
    window.draw(background);
    
    // 绘制每个格子的背景
    for (int y = 0; y < gridSize; ++y) {
        for (int x = 0; x < gridSize; ++x) {
            sf::RectangleShape cell(sf::Vector2f(TILE_SIZE, TILE_SIZE));
            cell.setPosition(
                GRID_OFFSET_X + x * (TILE_SIZE + TILE_MARGIN),
                GRID_OFFSET_Y + y * (TILE_SIZE + TILE_MARGIN)
            );
            
            // 根据游戏版本设置背景颜色
            if (currentVersion == GameVersion::MODIFIED) {
                cell.setFillColor(getCellBackgroundColor(x, y));
            } else {
                cell.setFillColor(sf::Color(205, 193, 180));
            }
            
            window.draw(cell);
        }
    }
    
    // 绘制网格线 - 修正长度，确保对齐
    for (int i = 0; i <= gridSize; ++i) {
        // 垂直线 - 只绘制到网格的实际高度
        sf::RectangleShape vLine(sf::Vector2f(GRID_LINE_THICKNESS, gridHeight));
        vLine.setPosition(
            GRID_OFFSET_X + i * (TILE_SIZE + TILE_MARGIN) - TILE_MARGIN/2 - GRID_LINE_THICKNESS/2,
            GRID_OFFSET_Y
        );
        vLine.setFillColor(GRID_LINE_COLOR);
        window.draw(vLine);
        
        // 水平线 - 只绘制到网格的实际宽度
        sf::RectangleShape hLine(sf::Vector2f(gridWidth, GRID_LINE_THICKNESS));
        hLine.setPosition(
            GRID_OFFSET_X,
            GRID_OFFSET_Y + i * (TILE_SIZE + TILE_MARGIN) - TILE_MARGIN/2 - GRID_LINE_THICKNESS/2
        );
        hLine.setFillColor(GRID_LINE_COLOR);
        window.draw(hLine);
    }
}

void Game::updateGifFrame(sf::Texture& texture, int value) {
    auto it = gifWrappers.find(value);
    if (it != gifWrappers.end()) {
        it->second.updateFrame();
        texture = it->second.getCurrentFrame();
    }
}