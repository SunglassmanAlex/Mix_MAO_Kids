#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "字体测试");
    sf::Font font;
    
    // 尝试加载字体
    std::vector<std::string> fontPaths = {
        "assets/fonts/SourceHanSansSC.otf",
        "assets/fonts/msyh.ttc",
        "assets/fonts/simsun.ttc",
        "assets/fonts/arial.ttf"
    };
    
    bool fontLoaded = false;
    std::string loadedFont;
    
    for (const auto& path : fontPaths) {
        if (font.loadFromFile(path)) {
            std::cout << "成功加载字体: " << path << std::endl;
            fontLoaded = true;
            loadedFont = path;
            break;
        } else {
            std::cout << "字体加载失败: " << path << std::endl;
        }
    }
    
    if (!fontLoaded) {
        std::cout << "无法加载任何字体!" << std::endl;
        return 1;
    }
    
    // 测试不同的中文字符串
    std::vector<std::string> testStrings = {
        "测试", // 简单汉字
        "合成耄孩子", // 游戏标题
        "恭喜获胜!!", // 胜利文字
        "继续游戏", // 按钮文字
        "ABCD1234", // 英文数字
        "这是中文字符显示测试" // 完整句子
    };
    
    std::vector<sf::Text> texts;
    
    for (size_t i = 0; i < testStrings.size(); ++i) {
        sf::Text text;
        text.setFont(font);
        text.setString(testStrings[i]);
        text.setCharacterSize(32);
        text.setFillColor(sf::Color::White);
        text.setPosition(50, 50 + i * 60);
        texts.push_back(text);
        
        std::cout << "设置文字: " << testStrings[i] << std::endl;
    }
    
    // 背景
    sf::RectangleShape background(sf::Vector2f(800, 600));
    background.setFillColor(sf::Color::Black);
    
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)
                window.close();
        }
        
        window.clear();
        window.draw(background);
        
        for (const auto& text : texts) {
            window.draw(text);
        }
        
        window.display();
    }
    
    return 0;
} 