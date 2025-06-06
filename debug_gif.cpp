#include <SFML/Graphics.hpp>
#include "src/gif/gif_wrapper.h"
#include <iostream>

int main() {
    // 测试加载一个GIF
    GifWrapper gif;
    if (!gif.loadFromFile("assets/picture/2.gif")) {
        std::cerr << "Failed to load GIF" << std::endl;
        return -1;
    }
    
    std::cout << "GIF loaded successfully!" << std::endl;
    std::cout << "Is animated: " << (gif.isAnimated() ? "Yes" : "No") << std::endl;
    
    // 获取纹理并检查尺寸
    sf::Texture& texture = gif.getCurrentFrame();
    sf::Vector2u size = texture.getSize();
    
    std::cout << "Texture size: " << size.x << "x" << size.y << std::endl;
    
    if (size.x > 0 && size.y > 0) {
        std::cout << "Texture has valid dimensions!" << std::endl;
        
        // 复制纹理到图像并检查几个像素
        sf::Image image = texture.copyToImage();
        sf::Color topLeft = image.getPixel(0, 0);
        sf::Color center = image.getPixel(size.x/2, size.y/2);
        
        std::cout << "Top-left pixel: RGBA(" << (int)topLeft.r << "," << (int)topLeft.g << "," << (int)topLeft.b << "," << (int)topLeft.a << ")" << std::endl;
        std::cout << "Center pixel: RGBA(" << (int)center.r << "," << (int)center.g << "," << (int)center.b << "," << (int)center.a << ")" << std::endl;
        
        // 检查是否所有像素都是透明的
        bool allTransparent = true;
        for (unsigned int y = 0; y < size.y && allTransparent; ++y) {
            for (unsigned int x = 0; x < size.x && allTransparent; ++x) {
                sf::Color pixel = image.getPixel(x, y);
                if (pixel.a > 0) {
                    allTransparent = false;
                }
            }
        }
        
        if (allTransparent) {
            std::cout << "WARNING: All pixels are transparent!" << std::endl;
        } else {
            std::cout << "SUCCESS: Found non-transparent pixels!" << std::endl;
        }
    } else {
        std::cout << "ERROR: Texture has invalid dimensions!" << std::endl;
    }
    
    return 0;
} 