#ifndef GIF_WRAPPER_H
#define GIF_WRAPPER_H

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <memory>

struct GifFrame {
    sf::Texture texture;
    float delay;  // 帧延迟（秒）
};

class GifWrapper {
public:
    GifWrapper();
    ~GifWrapper();
    
    // 移动语义
    GifWrapper(GifWrapper&& other) noexcept;
    GifWrapper& operator=(GifWrapper&& other) noexcept;
    
    // 禁用拷贝
    GifWrapper(const GifWrapper&) = delete;
    GifWrapper& operator=(const GifWrapper&) = delete;

    bool loadFromFile(const std::string& filename);
    void updateFrame();
    sf::Texture& getCurrentFrame();
    void setLooping(bool loop);
    bool isLooping() const;
    void reset();
    bool isAnimated() const;
    float getFrameDelay() const;

private:
    std::vector<GifFrame> frames;
    size_t currentFrame;
    bool looping;
    sf::Clock frameClock;
    bool animated;
    
    // GIF解码相关
    struct GifData;
    std::unique_ptr<GifData> gifData;
    bool decodeGifData(const std::string& filename);
    bool readGifHeader(FILE* file, int& width, int& height);
    bool readColorTable(FILE* file, int size, std::vector<sf::Color>& colorTable);
    bool readImageData(FILE* file, sf::Image& image, const std::vector<sf::Color>& colorTable);
    void clearGifData();
};

bool loadGif(const std::string& filename, sf::Texture& texture);

#endif // GIF_WRAPPER_H