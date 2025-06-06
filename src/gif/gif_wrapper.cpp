#include "gif_wrapper.h"
#include <iostream>
#include <cstring>
#include <algorithm>
#include <vector>
#include <unordered_map>

// GIF文件格式常量
const uint8_t GIF_MAGIC[] = {'G', 'I', 'F', '8', '9', 'a'};
const uint8_t IMAGE_SEPARATOR = 0x2C;
const uint8_t EXTENSION_INTRODUCER = 0x21;
const uint8_t GRAPHIC_CONTROL_LABEL = 0xF9;
const uint8_t TRAILER = 0x3B;

struct GifWrapper::GifData {
    std::vector<uint8_t> buffer;
    size_t position = 0;
    int width = 0;
    int height = 0;
    std::vector<sf::Color> globalColorTable;
    float defaultDelay = 0.1f; // 默认帧延迟（秒）
};

GifWrapper::GifWrapper() : currentFrame(0), looping(true), animated(false), 
                         backgroundColorOverride(sf::Color::Transparent), useBackgroundOverride(false) {
    gifData = std::make_unique<GifData>();
}

GifWrapper::~GifWrapper() {
    clearGifData();
}

GifWrapper::GifWrapper(GifWrapper&& other) noexcept
    : frames(std::move(other.frames)),
      currentFrame(other.currentFrame),
      looping(other.looping),
      frameClock(other.frameClock),
      animated(other.animated),
      backgroundColorOverride(other.backgroundColorOverride),
      useBackgroundOverride(other.useBackgroundOverride),
      gifData(std::move(other.gifData)) {
}

GifWrapper& GifWrapper::operator=(GifWrapper&& other) noexcept {
    if (this != &other) {
        frames = std::move(other.frames);
        currentFrame = other.currentFrame;
        looping = other.looping;
        frameClock = other.frameClock;
        animated = other.animated;
        backgroundColorOverride = other.backgroundColorOverride;
        useBackgroundOverride = other.useBackgroundOverride;
        gifData = std::move(other.gifData);
    }
    return *this;
}

void GifWrapper::clearGifData() {
    if (gifData) {
        gifData->buffer.clear();
        gifData->position = 0;
        gifData->globalColorTable.clear();
    }
}

// LZW解码器
class LZWDecoder {
public:
    LZWDecoder(int minCodeSize) : minCodeSize(minCodeSize) {
        clearCode = 1 << minCodeSize;
        endCode = clearCode + 1;
        nextCode = endCode + 1;
        codeSize = minCodeSize + 1;
        maxCode = (1 << codeSize) - 1;
        initDictionary();
    }

    void initDictionary() {
        dictionary.clear();
        for (int i = 0; i < clearCode; i++) {
            dictionary[i] = std::vector<uint8_t>{static_cast<uint8_t>(i)};
        }
        nextCode = endCode + 1;
        codeSize = minCodeSize + 1;
        maxCode = (1 << codeSize) - 1;
    }

    std::vector<uint8_t> decode(const std::vector<uint8_t>& input) {
        std::vector<uint8_t> output;
        BitReader reader(input);
        
        int oldCode = -1;
        std::vector<uint8_t> str;

        while (true) {
            int code = reader.readBits(codeSize);
            if (code == endCode) break;
            
            if (code == clearCode) {
                initDictionary();
                oldCode = -1;
                continue;
            }

            if (oldCode == -1) {
                output.insert(output.end(), dictionary[code].begin(), dictionary[code].end());
                oldCode = code;
                continue;
            }

            if (code < nextCode) {
                str = dictionary[code];
            } else {
                str = dictionary[oldCode];
                str.push_back(str[0]);
            }

            output.insert(output.end(), str.begin(), str.end());

            if (nextCode <= maxCode) {
                auto newEntry = dictionary[oldCode];
                newEntry.push_back(str[0]);
                dictionary[nextCode++] = newEntry;

                if (nextCode > maxCode && codeSize < 12) {
                    codeSize++;
                    maxCode = (1 << codeSize) - 1;
                }
            }

            oldCode = code;
        }

        return output;
    }

private:
    class BitReader {
    public:
        BitReader(const std::vector<uint8_t>& data) : data(data), pos(0), bitPos(0) {}

        int readBits(int count) {
            int result = 0;
            int bitsRead = 0;

            while (bitsRead < count) {
                if (pos >= data.size()) return -1;

                int bitsAvailable = 8 - bitPos;
                int bitsNeeded = count - bitsRead;
                int bitsToRead = std::min(bitsAvailable, bitsNeeded);

                int mask = ((1 << bitsToRead) - 1) << bitPos;
                int bits = (data[pos] & mask) >> bitPos;

                result |= bits << bitsRead;
                bitsRead += bitsToRead;
                bitPos += bitsToRead;

                if (bitPos >= 8) {
                    pos++;
                    bitPos = 0;
                }
            }

            return result;
        }

    private:
        const std::vector<uint8_t>& data;
        size_t pos;
        int bitPos;
    };

    int minCodeSize;
    int clearCode;
    int endCode;
    int nextCode;
    int codeSize;
    int maxCode;
    std::unordered_map<int, std::vector<uint8_t>> dictionary;
};

bool GifWrapper::loadFromFile(const std::string& filename) {
    return loadFromFile(filename, sf::Color::Transparent);
}

bool GifWrapper::loadFromFile(const std::string& filename, const sf::Color& backgroundColor) {
    // 设置背景色覆盖
    backgroundColorOverride = backgroundColor;
    useBackgroundOverride = (backgroundColor != sf::Color::Transparent);
    
    // 清除现有数据
    frames.clear();
    currentFrame = 0;
    animated = false;
    frameClock.restart();

    // 打开文件
    FILE* file = fopen(filename.c_str(), "rb");
    if (!file) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return false;
    }

    // 读取GIF头部
    uint8_t header[6];
    if (fread(header, 1, 6, file) != 6 ||
        memcmp(header, GIF_MAGIC, 6) != 0) {
        fclose(file);
        return false;
    }

    // 读取逻辑屏幕描述符
    uint8_t lsd[7];
    if (fread(lsd, 1, 7, file) != 7) {
        fclose(file);
        return false;
    }

    // 获取宽度和高度
    int width = lsd[0] | (lsd[1] << 8);
    int height = lsd[2] | (lsd[3] << 8);

    // 读取全局颜色表
    uint8_t packed = lsd[4];
    bool hasGlobalColorTable = (packed & 0x80) != 0;
    int colorTableSize = hasGlobalColorTable ? 1 << ((packed & 0x07) + 1) : 0;
    
    std::vector<sf::Color> globalColorTable;
    if (hasGlobalColorTable) {
        globalColorTable.resize(colorTableSize);
        for (int i = 0; i < colorTableSize; ++i) {
            uint8_t rgb[3];
            if (fread(rgb, 1, 3, file) != 3) {
                fclose(file);
                return false;
            }
            globalColorTable[i] = sf::Color(rgb[0], rgb[1], rgb[2]);
        }
    }

    // 创建基础图像
    sf::Image baseImage;
    baseImage.create(width, height, sf::Color::Transparent);

    float frameDelay = gifData->defaultDelay;
    bool firstFrame = true;

    // 读取数据块
    while (true) {
        uint8_t blockType;
        if (fread(&blockType, 1, 1, file) != 1) break;

        if (blockType == IMAGE_SEPARATOR) {
            // 图像块
            sf::Image frameImage = baseImage;

            // 读取图像描述符
            uint8_t descriptor[9];
            if (fread(descriptor, 1, 9, file) != 9) {
                if (firstFrame) {
                    fclose(file);
                    return false;
                }
                break;
            }

            // 获取图像位置和大小
            int left = descriptor[0] | (descriptor[1] << 8);
            int top = descriptor[2] | (descriptor[3] << 8);
            int imageWidth = descriptor[4] | (descriptor[5] << 8);
            int imageHeight = descriptor[6] | (descriptor[7] << 8);

            // 检查局部颜色表
            bool hasLocalColorTable = (descriptor[8] & 0x80) != 0;
            std::vector<sf::Color> localColorTable;
            if (hasLocalColorTable) {
                int localColorTableSize = 1 << ((descriptor[8] & 0x07) + 1);
                localColorTable.resize(localColorTableSize);
                for (int i = 0; i < localColorTableSize; ++i) {
                    uint8_t rgb[3];
                    if (fread(rgb, 1, 3, file) != 3) {
                        if (firstFrame) {
                            fclose(file);
                            return false;
                        }
                        break;
                    }
                    localColorTable[i] = sf::Color(rgb[0], rgb[1], rgb[2]);
                }
            }

            // 读取LZW最小码长度
            uint8_t lzwMinCodeSize;
            if (fread(&lzwMinCodeSize, 1, 1, file) != 1) {
                if (firstFrame) {
                    fclose(file);
                    return false;
                }
                break;
            }

            // 读取图像数据块
            std::vector<uint8_t> imageData;
            uint8_t blockSize;
            while (fread(&blockSize, 1, 1, file) == 1 && blockSize > 0) {
                size_t currentSize = imageData.size();
                imageData.resize(currentSize + blockSize);
                if (fread(imageData.data() + currentSize, 1, blockSize, file) != blockSize) {
                    if (firstFrame) {
                        fclose(file);
                        return false;
                    }
                    break;
                }
            }

            // 解码LZW数据并创建图像
            if (!imageData.empty()) {
                LZWDecoder decoder(lzwMinCodeSize);
                std::vector<uint8_t> decodedData = decoder.decode(imageData);
                
                // 选择使用的颜色表
                const std::vector<sf::Color>& colorTable = hasLocalColorTable ? localColorTable : globalColorTable;
                
                // 将解码的数据转换为像素
                if (!decodedData.empty() && !colorTable.empty()) {
                    for (int py = 0; py < imageHeight && py + top < height; ++py) {
                        for (int px = 0; px < imageWidth && px + left < width; ++px) {
                            int dataIndex = py * imageWidth + px;
                            if (dataIndex < static_cast<int>(decodedData.size())) {
                                uint8_t colorIndex = decodedData[dataIndex];
                                if (colorIndex < colorTable.size()) {
                                    sf::Color pixelColor = colorTable[colorIndex];
                                    
                                    // 如果启用背景色覆盖，替换透明或接近黑色的像素
                                    if (useBackgroundOverride) {
                                        // 检查是否为透明或接近黑色的像素
                                        bool isTransparentOrDark = (pixelColor.a < 128) || 
                                                                 (pixelColor.r < 50 && pixelColor.g < 50 && pixelColor.b < 50);
                                        if (isTransparentOrDark) {
                                            pixelColor = backgroundColorOverride;
                                        }
                                    }
                                    
                                    frameImage.setPixel(px + left, py + top, pixelColor);
                                }
                            }
                        }
                    }
                }
            }

            // 创建帧
            GifFrame frame;
            frame.texture.create(width, height);
            frame.texture.update(frameImage);
            frame.delay = frameDelay;
            frames.push_back(frame);

            firstFrame = false;
            animated = frames.size() > 1;
        }
        else if (blockType == EXTENSION_INTRODUCER) {
            // 扩展块
            uint8_t label;
            if (fread(&label, 1, 1, file) != 1) break;

            if (label == GRAPHIC_CONTROL_LABEL) {
                // 图形控制扩展
                uint8_t blockSize;
                if (fread(&blockSize, 1, 1, file) != 1 || blockSize != 4) break;

                uint8_t packed;
                if (fread(&packed, 1, 1, file) != 1) break;

                // 读取延迟时间
                uint16_t delay;
                if (fread(&delay, 2, 1, file) != 1) break;
                frameDelay = delay / 100.0f; // 转换为秒

                // 跳过透明色索引
                uint8_t transparentColorIndex;
                if (fread(&transparentColorIndex, 1, 1, file) != 1) break;

                // 跳过块终结符
                uint8_t terminator;
                if (fread(&terminator, 1, 1, file) != 1) break;
            }
            else {
                // 跳过其他扩展块
                uint8_t blockSize;
                while (fread(&blockSize, 1, 1, file) == 1 && blockSize > 0) {
                    fseek(file, blockSize, SEEK_CUR);
                }
            }
        }
        else if (blockType == TRAILER) {
            break;
        }
    }

    fclose(file);

    // 如果没有帧，创建一个默认帧
    if (frames.empty()) {
        GifFrame frame;
        frame.texture.create(width, height);
        frame.texture.update(baseImage);
        frame.delay = gifData->defaultDelay;
        frames.push_back(frame);
    }

    return true;
}

void GifWrapper::updateFrame() {
    if (frames.empty() || !animated) return;

    float elapsed = frameClock.getElapsedTime().asSeconds();
    if (elapsed >= frames[currentFrame].delay) {
        frameClock.restart();
        if (looping || currentFrame + 1 < frames.size()) {
            currentFrame = (currentFrame + 1) % frames.size();
        }
    }
}

sf::Texture& GifWrapper::getCurrentFrame() {
    if (frames.empty()) {
        static sf::Texture emptyTexture;
        if (emptyTexture.getSize().x == 0) {
            sf::Image img;
            img.create(64, 64, sf::Color(128, 128, 128, 255));
            emptyTexture.loadFromImage(img);
        }
        return emptyTexture;
    }
    return frames[currentFrame].texture;
}

void GifWrapper::setLooping(bool loop) {
    looping = loop;
}

bool GifWrapper::isLooping() const {
    return looping;
}

void GifWrapper::reset() {
    currentFrame = 0;
    frameClock.restart();
}

bool GifWrapper::isAnimated() const {
    return animated;
}

float GifWrapper::getFrameDelay() const {
    if (frames.empty()) return 0.0f;
    return frames[currentFrame].delay;
}

bool loadGif(const std::string& filename, sf::Texture& texture) {
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