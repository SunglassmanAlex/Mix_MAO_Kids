#include "Game2048.h"
#include "../gif/gif_wrapper.h"
#include <stdexcept>
#include <random>
#include <sstream>
#include <iostream>
#include <locale>
#include <codecvt>
#include <cmath>
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

// 胜利条件配置 - 修改这里可以改变胜利所需的数值
constexpr int WIN_VALUE = 16; // 当前设为16，以后可改为2048

// UTF-8 字符串转换辅助函数
sf::String toUTF8String(const std::string& str) {
    return sf::String::fromUtf8(str.begin(), str.end());
}

Game::Game() : window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), L"合成耄孩子"),
               currentState(GameState::MAIN_MENU),
               currentVersion(GameVersion::ORIGINAL),
               gridSize(4),
               score(0),
               gameOver(false),
               gameWon(false),
               winDialogShown(false),
               achievedWin(false),
               winAchievementDialogShown(false),
               isPaused(false),
               gifXPosition(WINDOW_WIDTH),
               secondGifXPosition(WINDOW_WIDTH + 150) { // 第二个GIF初始位置偏移
    
    // 设置UTF-8语言环境支持中文
    std::setlocale(LC_ALL, "en_US.UTF-8");
    
    // 尝试加载支持中文的字体 - 优先使用确定有效的项目字体
    bool fontLoaded = false;
    
    // 优先使用项目内的字体文件，确保路径正确
    std::vector<std::pair<std::string, std::string>> projectFonts = {
        {"../assets/fonts/SourceHanSansSC.otf", "思源黑体"},
        {"../assets/fonts/simhei.ttf", "黑体"},
        {"../assets/fonts/msyh.ttc", "微软雅黑"},
        {"../assets/fonts/simsun.ttc", "宋体"}
    };
    
    for (const auto& fontPair : projectFonts) {
        if (font.loadFromFile(fontPair.first)) {
            std::cout << "✓ 成功加载字体: " << fontPair.second << " (" << fontPair.first << ")" << std::endl;
            
            // 测试中文字符渲染能力
            sf::Text testText;
            testText.setFont(font);
            testText.setString(toUTF8String("测试中文"));
            testText.setCharacterSize(24);
            
            std::cout << "✓ 字体支持中文字符测试完成" << std::endl;
            fontLoaded = true;
            break;
        } else {
            std::cout << "✗ 加载字体失败: " << fontPair.second << " (" << fontPair.first << ")" << std::endl;
        }
    }
    
    // 如果项目字体失败，尝试系统字体作为备用
    if (!fontLoaded) {
        std::vector<std::pair<std::string, std::string>> systemFonts = {
            {"/mnt/c/Windows/Fonts/msyh.ttc", "系统微软雅黑"},
            {"/mnt/c/Windows/Fonts/simsun.ttc", "系统宋体"},
            {"/mnt/c/Windows/Fonts/simhei.ttf", "系统黑体"}
        };
        
        for (const auto& fontPair : systemFonts) {
            if (font.loadFromFile(fontPair.first)) {
                std::cout << "✓ 成功加载系统字体: " << fontPair.second << " (" << fontPair.first << ")" << std::endl;
                fontLoaded = true;
                break;
            }
        }
    }
    
    if (!fontLoaded) {
        std::cout << "✗ 所有字体加载失败，使用默认字体" << std::endl;
        // 不抛出异常，而是使用默认字体继续运行
        std::cout << "⚠ 警告：将使用默认字体，中文可能无法正确显示" << std::endl;
    }
    
    setupTileColors();
    initializeUI();
    setupExitConfirmUI();
    setupWinUI();
    setupWinAchievementUI();
    setupGameOverUI();
    setupPauseUI();
    
    // 加载胜利图片
    if (!winTexture.loadFromFile("assets/picture/win.jpg")) {
        std::cerr << "✗ Failed to load win.jpg" << std::endl;
    } else {
        std::cout << "✓ Successfully loaded win.jpg (" << winTexture.getSize().x << "x" << winTexture.getSize().y << ")" << std::endl;
    }

    // 加载失败图片
    if (!loseTexture.loadFromFile("assets/picture/lose.jpg")) {
        std::cerr << "✗ Failed to load lose.jpg" << std::endl;
    } else {
        std::cout << "✓ Successfully loaded lose.jpg (" << loseTexture.getSize().x << "x" << loseTexture.getSize().y << ")" << std::endl;
    }
    
    // 图片加载完成后，重新设置sprite的纹理和位置
    setupWinSprites();

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
    
    // 对应的方块值，用于获取正确的背景色
    std::vector<int> decorativeValues = {4, 8, 16, 32, 64};
    
    decorativeTextures.resize(decorativeFiles.size());
    decorativeSprites.resize(decorativeFiles.size());
    decorativeGifWrappers.resize(decorativeFiles.size());
    
    for (size_t i = 0; i < decorativeFiles.size(); ++i) {
        // 所有装饰GIF都使用主菜单背景色，保持一致的渲染方式
        sf::Color decorativeBackgroundColor = mainMenuBackgroundColor;
        if (decorativeGifWrappers[i].loadFromFile(decorativeFiles[i], decorativeBackgroundColor)) {
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
                    decorativeSprites[i].setPosition(50, WINDOW_HEIGHT / 2);
                    break;
                case 3: // 右下角 (避开动画GIF区域)
                    decorativeSprites[i].setPosition(WINDOW_WIDTH - 130, WINDOW_HEIGHT / 2);
                    break;
                case 4: // 左中
                    decorativeSprites[i].setPosition(50, WINDOW_HEIGHT / 2 - 150);
                    break;
            }
            
            std::cout << "Loaded decorative animated GIF: " << decorativeFiles[i] << std::endl;
        } else {
            std::cerr << "Failed to load decorative animated GIF: " << decorativeFiles[i] << std::endl;
        }
    }

    // 先加载32768的jpg纹理
    if (!texture32768.loadFromFile("assets/picture/32768.jpg")) {
        std::cerr << "✗ Failed to load 32768.jpg" << std::endl;
    } else {
        std::cout << "✓ Successfully loaded 32768.jpg" << std::endl;
        tileGifTexturesMap[32768] = texture32768;
    }

    // 加载所有可能的GIF纹理（除了32768）
    std::vector<int> values = {2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384};
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
            // 如果加载失败，使用32768.jpg作为后备
            if (texture32768.getSize().x > 0) {
                tileGifTexturesMap[value] = texture32768;
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
    
    // Confirmation text - 横向居中
    exitConfirmText.setFont(font);
    exitConfirmText.setString(toUTF8String("确定要退出游戏吗？"));
    exitConfirmText.setCharacterSize(28);
    exitConfirmText.setFillColor(sf::Color::White);
    // 居中对齐
    sf::FloatRect confirmTextRect = exitConfirmText.getLocalBounds();
    exitConfirmText.setOrigin(confirmTextRect.left + confirmTextRect.width/2.0f,
                             confirmTextRect.top + confirmTextRect.height/2.0f);
    exitConfirmText.setPosition(WINDOW_WIDTH/2, WINDOW_HEIGHT/2 - 50);
    
    // Yes/No buttons
    exitConfirmYesButton.setSize(sf::Vector2f(120, 50));
    exitConfirmYesButton.setPosition(WINDOW_WIDTH/2 - 140, WINDOW_HEIGHT/2 + 20);
    exitConfirmYesButton.setFillColor(sf::Color(100, 255, 100));
    
    exitConfirmNoButton.setSize(sf::Vector2f(120, 50));
    exitConfirmNoButton.setPosition(WINDOW_WIDTH/2 + 20, WINDOW_HEIGHT/2 + 20);
    exitConfirmNoButton.setFillColor(sf::Color(255, 100, 100));
    
    // Button texts - 横向居中
    exitConfirmYesText.setFont(font);
    exitConfirmYesText.setString(toUTF8String("是(Y)"));
    exitConfirmYesText.setCharacterSize(24);
    exitConfirmYesText.setFillColor(sf::Color::White);
    // 居中对齐
    sf::FloatRect yesTextRect = exitConfirmYesText.getLocalBounds();
    exitConfirmYesText.setOrigin(yesTextRect.left + yesTextRect.width/2.0f,
                                yesTextRect.top + yesTextRect.height/2.0f);
    exitConfirmYesText.setPosition(WINDOW_WIDTH/2 - 80, WINDOW_HEIGHT/2 + 45); // 按钮中心位置
    
    exitConfirmNoText.setFont(font);
    exitConfirmNoText.setString(toUTF8String("否(N)"));
    exitConfirmNoText.setCharacterSize(24);
    exitConfirmNoText.setFillColor(sf::Color::White);
    // 居中对齐
    sf::FloatRect noTextRect = exitConfirmNoText.getLocalBounds();
    exitConfirmNoText.setOrigin(noTextRect.left + noTextRect.width/2.0f,
                               noTextRect.top + noTextRect.height/2.0f);
    exitConfirmNoText.setPosition(WINDOW_WIDTH/2 + 80, WINDOW_HEIGHT/2 + 45); // 按钮中心位置
}

void Game::setupWinUI() {
    // Semi-transparent background
    winBackground.setSize(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
    winBackground.setFillColor(sf::Color(0, 0, 0, 150));
    
    // Win dialog box
    winBox.setSize(sf::Vector2f(600, 400));
    winBox.setPosition(WINDOW_WIDTH/2 - 300, WINDOW_HEIGHT/2 - 200);
    winBox.setFillColor(sf::Color(250, 248, 239));
    
    // Win sprite (will be set in setupWinSprites)
    // 这里只设置默认位置，实际纹理和位置会在图片加载后设置
    winSprite.setPosition(WINDOW_WIDTH/2 - 100, WINDOW_HEIGHT/2 - 150);
    
    // "You win!!" text
    winText.setFont(font);
    winText.setString(toUTF8String("你赢了!!"));
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
    winContinueText.setString(toUTF8String("继续(Y)"));
    winContinueText.setCharacterSize(20);
    winContinueText.setFillColor(sf::Color::White);
    winContinueText.setPosition(WINDOW_WIDTH/2 - 165, WINDOW_HEIGHT/2 + 95);
    
    winQuitText.setFont(font);
    winQuitText.setString(toUTF8String("退出(Esc)"));
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
    gameOverText.setString(toUTF8String("游戏结束!"));
    gameOverText.setCharacterSize(48);
    gameOverText.setFillColor(sf::Color::Red);
    gameOverText.setPosition(WINDOW_WIDTH/2 - 120, WINDOW_HEIGHT/2 - 50);
    
    // Restart prompt text
    restartText.setFont(font);
    restartText.setString(toUTF8String("按 R 键重新开始"));
    restartText.setCharacterSize(24);
    restartText.setFillColor(sf::Color::White);
    restartText.setPosition(WINDOW_WIDTH/2 - 100, WINDOW_HEIGHT/2 + 20);
}

void Game::setupMainMenu() {
    // Main menu title
    titleText.setFont(font);
    titleText.setString(toUTF8String("合成耄孩子"));
    titleText.setCharacterSize(64);
    titleText.setFillColor(sf::Color::White);
    titleText.setPosition(250, 100);
    
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
    versionTitleText.setString(toUTF8String("选择游戏版本"));
    versionTitleText.setCharacterSize(48);
    versionTitleText.setFillColor(sf::Color::White);
    versionTitleText.setPosition(280, 100);
    
    const std::array<std::string, 2> versionLabels = {
        "经典版本 (方向键)",
        "对角线版本 (Q/E/Z/C键)"
    };
    
    for (size_t i = 0; i < versionButtons.size(); ++i) {
        // Version selection buttons
        versionButtons[i].setSize(sf::Vector2f(400, 80));
        versionButtons[i].setPosition(200, 250 + i * 120);
        versionButtons[i].setFillColor(sf::Color(143, 122, 102));
        
        // Button text
        versionButtonTexts[i].setFont(font);
        versionButtonTexts[i].setString(toUTF8String(versionLabels[i]));
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
                } else {
                    // 总是调用handleGameInput，让它内部处理各种对话框状态
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

            // 16达成界面点击处理
            if (currentState == GameState::GAME && achievedWin && winAchievementDialogShown) {
                handleWinAchievementDialogClick(mousePos);
                continue;
            }

            // 游戏结束界面点击处理
            if (currentState == GameState::GAME && gameOver && !gameWon) {
                handleGameOverDialogClick(mousePos);
                continue;
            }

            // 暂停界面点击处理
            if (currentState == GameState::GAME && isPaused) {
                handlePauseDialogClick(mousePos);
                continue;
            }

            // 暂停按钮点击处理
            if (currentState == GameState::GAME && !gameOver && !isPaused && !winAchievementDialogShown && !winDialogShown) {
                sf::FloatRect pauseButtonBounds(WINDOW_WIDTH - 100, 20, 80, 40);
                if (pauseButtonBounds.contains(mousePos)) {
                    isPaused = true;
                    continue;
                }
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
    
    // 如果显示胜利达成对话框，优先处理胜利达成界面的输入
    if (achievedWin && winAchievementDialogShown) {
        handleWinAchievementDialogKeyInput(key);
        return;
    }
    
    // 如果显示游戏结束对话框，优先处理游戏结束界面的输入
    if (gameOver && !gameWon) {
        handleGameOverDialogKeyInput(key);
        return;
    }
    
    // 如果显示暂停对话框，优先处理暂停界面的输入
    if (isPaused) {
        handlePauseDialogKeyInput(key);
        return;
    }
    
    // 检查暂停键
    if (key == sf::Keyboard::P) {
        isPaused = true;
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
    if (key == sf::Keyboard::Y) {
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
    float gifYPosition = WINDOW_HEIGHT - (gifTexture.getSize().y * gifScale) - 100; // 从-120改为-250，再往上移动130像素
    
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
        
        // 如果达成胜利且显示对话框，绘制胜利达成界面
        if (achievedWin && winAchievementDialogShown) {
            window.draw(winAchievementBackground);
            window.draw(winAchievementBox);
            window.draw(winAchievementSprite);
            window.draw(winAchievementText);
            
            // 绘制圆角按钮
            drawRoundedRectangle(window, 
                sf::Vector2f(WINDOW_WIDTH/2 - 180, WINDOW_HEIGHT/2 + 80), 
                sf::Vector2f(150, 60), 
                sf::Color(220, 50, 50), // 红色继续游戏按钮
                12.0f);
            
            drawRoundedRectangle(window, 
                sf::Vector2f(WINDOW_WIDTH/2 + 30, WINDOW_HEIGHT/2 + 80), 
                sf::Vector2f(150, 60), 
                sf::Color(50, 180, 50), // 绿色返回主菜单按钮
                12.0f);
            
            window.draw(winAchievementContinueText);
            window.draw(winAchievementMenuText);
        }
        
        // 如果游戏结束且显示对话框，绘制游戏结束界面
        if (gameOver && !gameWon) {
            window.draw(gameOverBackground);
            window.draw(gameOverBox);
            window.draw(gameOverSprite);
            window.draw(gameOverDialogText);
            
            // 绘制圆角按钮
            drawRoundedRectangle(window, 
                sf::Vector2f(WINDOW_WIDTH/2 - 180, WINDOW_HEIGHT/2 + 80), 
                sf::Vector2f(150, 60), 
                sf::Color(220, 50, 50), // 红色重新开始按钮
                12.0f);
            
            drawRoundedRectangle(window, 
                sf::Vector2f(WINDOW_WIDTH/2 + 30, WINDOW_HEIGHT/2 + 80), 
                sf::Vector2f(150, 60), 
                sf::Color(50, 180, 50), // 绿色返回主菜单按钮
                12.0f);
            
            window.draw(gameOverRestartText);
            window.draw(gameOverMenuText);
        }
        
        // 如果游戏暂停，绘制暂停界面
        if (isPaused) {
            window.draw(pauseBackground);
            window.draw(pauseBox);
            window.draw(pauseText);
            
            // 绘制圆角按钮
            drawRoundedRectangle(window, 
                sf::Vector2f(WINDOW_WIDTH/2 - 100, WINDOW_HEIGHT/2 - 40), 
                sf::Vector2f(200, 60), 
                sf::Color(50, 180, 50), // 绿色继续游戏按钮
                12.0f);
            
            drawRoundedRectangle(window, 
                sf::Vector2f(WINDOW_WIDTH/2 - 100, WINDOW_HEIGHT/2 + 60), 
                sf::Vector2f(200, 60), 
                sf::Color(220, 50, 50), // 红色返回主菜单按钮
                12.0f);
            
            window.draw(pauseContinueText);
            window.draw(pauseMenuText);
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
        
        // 绘制圆角按钮
        drawRoundedRectangle(window, 
            sf::Vector2f(WINDOW_WIDTH/2 - 140, WINDOW_HEIGHT/2 + 20), 
            sf::Vector2f(120, 50), 
            sf::Color(100, 255, 100), // 绿色是按钮
            12.0f);
        
        drawRoundedRectangle(window, 
            sf::Vector2f(WINDOW_WIDTH/2 + 20, WINDOW_HEIGHT/2 + 20), 
            sf::Vector2f(120, 50), 
            sf::Color(255, 100, 100), // 红色否按钮
            12.0f);
        
        window.draw(exitConfirmYesText);
        window.draw(exitConfirmNoText);
    }

    window.display();
}

void Game::renderGame() {
    window.clear(sf::Color(187, 173, 160));
    
    // 更新分数显示
    std::stringstream ss;
    ss << "分数: " << score;
    scoreText.setString(toUTF8String(ss.str()));
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
    
    // 绘制右上角暂停按钮（只在游戏进行中且未暂停时显示，不受胜利对话框影响）
    if (!gameOver && !isPaused && !winAchievementDialogShown && !winDialogShown) {
        drawRoundedRectangle(window, 
            sf::Vector2f(WINDOW_WIDTH - 100, 20), 
            sf::Vector2f(80, 40), 
            sf::Color(100, 100, 100, 200), // 半透明灰色
            8.0f);
        window.draw(pauseButtonText);
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
    achievedWin = false;
    winAchievementDialogShown = false;
    isPaused = false;
    
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
                    
                    // 检测是否首次达到胜利条件
                    if (grid[nextY][nextX] == WIN_VALUE && !achievedWin) {
                        achievedWin = true;
                        winAchievementDialogShown = true;
                    }
                    
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

void Game::setupWinAchievementUI() {
    // Semi-transparent background
    winAchievementBackground.setSize(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
    winAchievementBackground.setFillColor(sf::Color(0, 0, 0, 150));
    
    // Achievement dialog box with rounded corners effect
    winAchievementBox.setSize(sf::Vector2f(600, 400));
    winAchievementBox.setPosition(WINDOW_WIDTH/2 - 300, WINDOW_HEIGHT/2 - 200);
    winAchievementBox.setFillColor(sf::Color(250, 248, 239));
    
    // Achievement sprite (win image) - 将在setupWinSprites中设置
    // 这里只设置默认位置，实际纹理和位置会在图片加载后设置
    winAchievementSprite.setPosition(WINDOW_WIDTH/2 - 75, WINDOW_HEIGHT/2 - 130);
    
    // "You Win!!" text
    winAchievementText.setFont(font);
    winAchievementText.setString(toUTF8String("你赢了!!"));
    winAchievementText.setCharacterSize(36);
    winAchievementText.setFillColor(sf::Color::Red);
    // 只左右居中，不上下居中
    sf::FloatRect winTextRect = winAchievementText.getLocalBounds();
    winAchievementText.setOrigin(winTextRect.left + winTextRect.width/2.0f, 0);
    winAchievementText.setPosition(WINDOW_WIDTH/2, WINDOW_HEIGHT/2 + 20);
    
    // Continue button (红色圆角按钮)
    winAchievementContinueButton.setSize(sf::Vector2f(150, 60));
    winAchievementContinueButton.setPosition(WINDOW_WIDTH/2 - 180, WINDOW_HEIGHT/2 + 80);
    winAchievementContinueButton.setFillColor(sf::Color(220, 50, 50)); // 红色
    
    // Menu button (绿色圆角按钮)
    winAchievementMenuButton.setSize(sf::Vector2f(150, 60));
    winAchievementMenuButton.setPosition(WINDOW_WIDTH/2 + 30, WINDOW_HEIGHT/2 + 80);
    winAchievementMenuButton.setFillColor(sf::Color(50, 180, 50)); // 绿色
    
    // Button texts
    winAchievementContinueText.setFont(font);
    winAchievementContinueText.setString(toUTF8String("继续游戏(Y)"));
    winAchievementContinueText.setCharacterSize(20);
    winAchievementContinueText.setFillColor(sf::Color::White);
    // 居中对齐
    sf::FloatRect continueTextRect = winAchievementContinueText.getLocalBounds();
    winAchievementContinueText.setOrigin(continueTextRect.left + continueTextRect.width/2.0f,
                                        continueTextRect.top + continueTextRect.height/2.0f);
    winAchievementContinueText.setPosition(WINDOW_WIDTH/2 - 105, WINDOW_HEIGHT/2 + 110);
    
    winAchievementMenuText.setFont(font);
    winAchievementMenuText.setString(toUTF8String("返回主菜单(M)"));
    winAchievementMenuText.setCharacterSize(20);
    winAchievementMenuText.setFillColor(sf::Color::White);
    // 居中对齐
    sf::FloatRect menuTextRect = winAchievementMenuText.getLocalBounds();
    winAchievementMenuText.setOrigin(menuTextRect.left + menuTextRect.width/2.0f,
                                    menuTextRect.top + menuTextRect.height/2.0f);
    winAchievementMenuText.setPosition(WINDOW_WIDTH/2 + 105, WINDOW_HEIGHT/2 + 110);
}

void Game::setupGameOverUI() {
    // Semi-transparent background
    gameOverBackground.setSize(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
    gameOverBackground.setFillColor(sf::Color(0, 0, 0, 150));
    
    // Game over dialog box with rounded corners effect
    gameOverBox.setSize(sf::Vector2f(600, 400));
    gameOverBox.setPosition(WINDOW_WIDTH/2 - 300, WINDOW_HEIGHT/2 - 200);
    gameOverBox.setFillColor(sf::Color(250, 248, 239));
    
    // Game over sprite (lose image) - 将在setupWinSprites中设置
    // 这里只设置默认位置，实际纹理和位置会在图片加载后设置
    gameOverSprite.setPosition(WINDOW_WIDTH/2 - 75, WINDOW_HEIGHT/2 - 130);
    
    // "You Lose!!" text
    gameOverDialogText.setFont(font);
    gameOverDialogText.setString(toUTF8String("你输了!!"));
    gameOverDialogText.setCharacterSize(36);
    gameOverDialogText.setFillColor(sf::Color::Red);
    // 只左右居中，不上下居中
    sf::FloatRect loseTextRect = gameOverDialogText.getLocalBounds();
    gameOverDialogText.setOrigin(loseTextRect.left + loseTextRect.width/2.0f, 0);
    gameOverDialogText.setPosition(WINDOW_WIDTH/2, WINDOW_HEIGHT/2 + 20);
    
    // Restart button (红色圆角按钮)
    gameOverRestartButton.setSize(sf::Vector2f(150, 60));
    gameOverRestartButton.setPosition(WINDOW_WIDTH/2 - 180, WINDOW_HEIGHT/2 + 80);
    gameOverRestartButton.setFillColor(sf::Color(220, 50, 50)); // 红色
    
    // Menu button (绿色圆角按钮)
    gameOverMenuButton.setSize(sf::Vector2f(150, 60));
    gameOverMenuButton.setPosition(WINDOW_WIDTH/2 + 30, WINDOW_HEIGHT/2 + 80);
    gameOverMenuButton.setFillColor(sf::Color(50, 180, 50)); // 绿色
    
    // Button texts
    gameOverRestartText.setFont(font);
    gameOverRestartText.setString(toUTF8String("重新开始(R)"));
    gameOverRestartText.setCharacterSize(20);
    gameOverRestartText.setFillColor(sf::Color::White);
    // 居中对齐
    sf::FloatRect restartTextRect = gameOverRestartText.getLocalBounds();
    gameOverRestartText.setOrigin(restartTextRect.left + restartTextRect.width/2.0f,
                                 restartTextRect.top + restartTextRect.height/2.0f);
    gameOverRestartText.setPosition(WINDOW_WIDTH/2 - 105, WINDOW_HEIGHT/2 + 110);
    
    gameOverMenuText.setFont(font);
    gameOverMenuText.setString(toUTF8String("返回主菜单(M)"));
    gameOverMenuText.setCharacterSize(20);
    gameOverMenuText.setFillColor(sf::Color::White);
    // 居中对齐
    sf::FloatRect gameOverMenuTextRect = gameOverMenuText.getLocalBounds();
    gameOverMenuText.setOrigin(gameOverMenuTextRect.left + gameOverMenuTextRect.width/2.0f,
                              gameOverMenuTextRect.top + gameOverMenuTextRect.height/2.0f);
    gameOverMenuText.setPosition(WINDOW_WIDTH/2 + 105, WINDOW_HEIGHT/2 + 110);
}

void Game::handleWinAchievementDialogClick(const sf::Vector2f& mousePos) {
    // 检查Continue按钮
    if (winAchievementContinueButton.getGlobalBounds().contains(mousePos)) {
        winAchievementDialogShown = false;
        return;
    }
    
    // 检查Menu按钮
    if (winAchievementMenuButton.getGlobalBounds().contains(mousePos)) {
        currentState = GameState::MAIN_MENU;
        winAchievementDialogShown = false;
        achievedWin = false;
        return;
    }
}

void Game::handleWinAchievementDialogKeyInput(sf::Keyboard::Key key) {
    if (key == sf::Keyboard::Y) {
        // Continue game
        winAchievementDialogShown = false;
        return;
    }
    
    if (key == sf::Keyboard::M) {
        // Go to main menu
        currentState = GameState::MAIN_MENU;
        winAchievementDialogShown = false;
        achievedWin = false;
        return;
    }
}

void Game::handleGameOverDialogClick(const sf::Vector2f& mousePos) {
    // 检查Restart按钮
    if (gameOverRestartButton.getGlobalBounds().contains(mousePos)) {
        resetGame();
        return;
    }
    
    // 检查Menu按钮
    if (gameOverMenuButton.getGlobalBounds().contains(mousePos)) {
        currentState = GameState::MAIN_MENU;
        return;
    }
}

void Game::handleGameOverDialogKeyInput(sf::Keyboard::Key key) {
    if (key == sf::Keyboard::R) {
        // Restart game
        resetGame();
        return;
    }
    
    if (key == sf::Keyboard::M) {
        // Go to main menu
        currentState = GameState::MAIN_MENU;
        return;
    }
}

void Game::setupWinSprites() {
    // 重新设置胜利界面的图片
    if (winTexture.getSize().x > 0 && winTexture.getSize().y > 0) {
        // 设置真正的胜利界面图片
        winSprite.setTexture(winTexture);
        float spriteScale = 150.0f / std::max(winTexture.getSize().x, winTexture.getSize().y);
        winSprite.setScale(spriteScale, spriteScale);
        float spriteWidth = winTexture.getSize().x * spriteScale;
        float spriteHeight = winTexture.getSize().y * spriteScale;
        winSprite.setPosition(WINDOW_WIDTH/2 - spriteWidth/2, WINDOW_HEIGHT/2 - 160);
        
        // 设置胜利达成界面图片
        winAchievementSprite.setTexture(winTexture);
        winAchievementSprite.setScale(spriteScale, spriteScale);
        winAchievementSprite.setPosition(WINDOW_WIDTH/2 - spriteWidth/2, WINDOW_HEIGHT/2 - 160);
        
        // 暂停界面不需要图片
        
        std::cout << "✓ 胜利图片设置完成: 位置(" << (WINDOW_WIDTH/2 - spriteWidth/2) << ", " << (WINDOW_HEIGHT/2 - 180) << "), 大小(" << spriteWidth << "x" << spriteHeight << ")" << std::endl;
    }
    
    // 重新设置游戏结束界面的图片
    if (loseTexture.getSize().x > 0 && loseTexture.getSize().y > 0) {
        gameOverSprite.setTexture(loseTexture);
        float spriteScale = 150.0f / std::max(loseTexture.getSize().x, loseTexture.getSize().y);
        gameOverSprite.setScale(spriteScale, spriteScale);
        float spriteWidth = loseTexture.getSize().x * spriteScale;
        float spriteHeight = loseTexture.getSize().y * spriteScale;
        gameOverSprite.setPosition(WINDOW_WIDTH/2 - spriteWidth/2, WINDOW_HEIGHT/2 - 160);
        
        std::cout << "✓ 失败图片设置完成: 位置(" << (WINDOW_WIDTH/2 - spriteWidth/2) << ", " << (WINDOW_HEIGHT/2 - 180) << "), 大小(" << spriteWidth << "x" << spriteHeight << ")" << std::endl;
    }
}

void Game::drawRoundedRectangle(sf::RenderWindow& window, const sf::Vector2f& position, const sf::Vector2f& size, const sf::Color& color, float cornerRadius) {
    // 用SFML的CircleShape做真正的圆角 - 简单有效
    
    float radius = std::min(cornerRadius, std::min(size.x, size.y) / 2.0f);
    
    // 主体矩形（中间部分）
    sf::RectangleShape mainRect(sf::Vector2f(size.x, size.y - 2 * radius));
    mainRect.setPosition(position.x, position.y + radius);
    mainRect.setFillColor(color);
    window.draw(mainRect);
    
    // 上下横条
    sf::RectangleShape topRect(sf::Vector2f(size.x - 2 * radius, radius));
    topRect.setPosition(position.x + radius, position.y);
    topRect.setFillColor(color);
    window.draw(topRect);
    
    sf::RectangleShape bottomRect(sf::Vector2f(size.x - 2 * radius, radius));
    bottomRect.setPosition(position.x + radius, position.y + size.y - radius);
    bottomRect.setFillColor(color);
    window.draw(bottomRect);
    
    // 四个真正的圆角
    sf::CircleShape corner(radius);
    corner.setFillColor(color);
    
    // 左上角
    corner.setPosition(position.x, position.y);
    window.draw(corner);
    
    // 右上角
    corner.setPosition(position.x + size.x - 2 * radius, position.y);
    window.draw(corner);
    
    // 左下角
    corner.setPosition(position.x, position.y + size.y - 2 * radius);
    window.draw(corner);
    
    // 右下角
    corner.setPosition(position.x + size.x - 2 * radius, position.y + size.y - 2 * radius);
    window.draw(corner);
}

void Game::setupPauseUI() {
    // Semi-transparent background
    pauseBackground.setSize(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
    pauseBackground.setFillColor(sf::Color(0, 0, 0, 150));
    
    // Pause dialog box with rounded corners effect
    pauseBox.setSize(sf::Vector2f(600, 400));
    pauseBox.setPosition(WINDOW_WIDTH/2 - 300, WINDOW_HEIGHT/2 - 200);
    pauseBox.setFillColor(sf::Color(250, 248, 239));
    
    // 暂停界面不需要图片
    
    // "Game Paused" text - 继续往上移动
    pauseText.setFont(font);
    pauseText.setString(toUTF8String("游戏暂停"));
    pauseText.setCharacterSize(36);
    pauseText.setFillColor(sf::Color::Blue);
    // 只左右居中，不上下居中
    sf::FloatRect pauseTextRect = pauseText.getLocalBounds();
    pauseText.setOrigin(pauseTextRect.left + pauseTextRect.width/2.0f, 0);
    pauseText.setPosition(WINDOW_WIDTH/2, WINDOW_HEIGHT/2 - 120); // 从-60改为-120，再往上移动60像素
    
    // Continue button (绿色圆角按钮) - 上方，继续往上移动
    pauseContinueButton.setSize(sf::Vector2f(200, 60));
    pauseContinueButton.setPosition(WINDOW_WIDTH/2 - 100, WINDOW_HEIGHT/2 - 40); // 从+20改为-40，往上移动60像素
    pauseContinueButton.setFillColor(sf::Color(50, 180, 50)); // 绿色
    
    // Menu button (红色圆角按钮) - 下方，继续往上移动
    pauseMenuButton.setSize(sf::Vector2f(200, 60));
    pauseMenuButton.setPosition(WINDOW_WIDTH/2 - 100, WINDOW_HEIGHT/2 + 60); // 从+120改为+60，往上移动60像素
    pauseMenuButton.setFillColor(sf::Color(220, 50, 50)); // 红色
    
    // Button texts
    pauseContinueText.setFont(font);
    pauseContinueText.setString(toUTF8String("继续游戏(Y)"));
    pauseContinueText.setCharacterSize(20);
    pauseContinueText.setFillColor(sf::Color::White);
    // 居中对齐
    sf::FloatRect continueTextRect = pauseContinueText.getLocalBounds();
    pauseContinueText.setOrigin(continueTextRect.left + continueTextRect.width/2.0f,
                               continueTextRect.top + continueTextRect.height/2.0f);
    pauseContinueText.setPosition(WINDOW_WIDTH/2, WINDOW_HEIGHT/2 - 10); // 从+50改为-10，往上移动60像素
    
    pauseMenuText.setFont(font);
    pauseMenuText.setString(toUTF8String("返回主菜单(M)"));
    pauseMenuText.setCharacterSize(20);
    pauseMenuText.setFillColor(sf::Color::White);
    // 居中对齐
    sf::FloatRect pauseMenuTextRect = pauseMenuText.getLocalBounds();
    pauseMenuText.setOrigin(pauseMenuTextRect.left + pauseMenuTextRect.width/2.0f,
                           pauseMenuTextRect.top + pauseMenuTextRect.height/2.0f);
    pauseMenuText.setPosition(WINDOW_WIDTH/2, WINDOW_HEIGHT/2 + 90); // 从+150改为+90，往上移动60像素
    
    // 右上角暂停按钮
    pauseButton.setSize(sf::Vector2f(80, 40));
    pauseButton.setPosition(WINDOW_WIDTH - 100, 20); // 右上角位置
    pauseButton.setFillColor(sf::Color(100, 100, 100, 200)); // 半透明灰色
    
    pauseButtonText.setFont(font);
    pauseButtonText.setString(toUTF8String("暂停(P)"));
    pauseButtonText.setCharacterSize(16);
    pauseButtonText.setFillColor(sf::Color::White);
    // 居中对齐
    sf::FloatRect pauseButtonTextRect = pauseButtonText.getLocalBounds();
    pauseButtonText.setOrigin(pauseButtonTextRect.left + pauseButtonTextRect.width/2.0f,
                             pauseButtonTextRect.top + pauseButtonTextRect.height/2.0f);
    pauseButtonText.setPosition(WINDOW_WIDTH - 60, 40); // 按钮中心位置
}

void Game::handlePauseDialogClick(const sf::Vector2f& mousePos) {
    // 检查Continue按钮
    if (pauseContinueButton.getGlobalBounds().contains(mousePos)) {
        isPaused = false;
        return;
    }
    
    // 检查Menu按钮
    if (pauseMenuButton.getGlobalBounds().contains(mousePos)) {
        currentState = GameState::MAIN_MENU;
        isPaused = false;
        return;
    }
}

void Game::handlePauseDialogKeyInput(sf::Keyboard::Key key) {
    if (key == sf::Keyboard::Y) {
        // Continue game
        isPaused = false;
        return;
    }
    
    if (key == sf::Keyboard::M) {
        // Go to main menu
        currentState = GameState::MAIN_MENU;
        isPaused = false;
        return;
    }
}