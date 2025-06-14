# 合成耄孩子 (My2048)

<div align="center">

![Game Logo](assets/picture/win.jpg)

**一个基于SFML的现代化2048游戏实现**

[![C++](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![SFML](https://img.shields.io/badge/SFML-2.5+-green.svg)](https://www.sfml-dev.org/)
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux-lightgrey.svg)]()

[功能特性](#功能特性) • [快速开始](#快速开始) • [游戏玩法](#游戏玩法) • [编译指南](#编译指南) • [贡献指南](#贡献指南)

</div>

## 📖 项目简介

**合成耄孩子**是一个使用C++和SFML图形库开发的现代化2048游戏。游戏不仅实现了经典的2048玩法，还加入了创新的对角线移动模式、精美的GIF动画效果和完整的中文界面支持。

### ✨ 功能特性

- 🎮 **双游戏模式**
  - 经典模式：传统的上下左右移动
  - 对角线模式：创新的对角线移动玩法
  
- 🎨 **精美视觉效果**
  - 动态GIF动画方块
  - 流畅的移动动画
  - 现代化圆角UI设计
  - 主菜单动态背景

- 🌐 **完整中文支持**
  - 全中文界面
  - 支持中文字体渲染
  - 本地化游戏提示

- ⚙️ **丰富功能**
  - 多种网格尺寸 (4×4, 5×5, 6×6)
  - 游戏暂停/继续
  - 胜利/失败界面
  - 分数统计系统

## 🚀 快速开始

### 系统要求

- **操作系统**: Windows 10+ 或 Ubuntu 18.04+
- **编译器**: GCC 7+ 或 Visual Studio 2017+
- **依赖库**: SFML 2.5+
- **CMake**: 3.10+

### 安装依赖

#### Ubuntu/Debian
```bash
sudo apt update
sudo apt install build-essential cmake libsfml-dev
```

#### Windows (使用vcpkg)
```bash
vcpkg install sfml
```

### 编译运行

```bash
# 克隆项目
git clone https://github.com/yourusername/my2048.git
cd my2048

# 创建构建目录
mkdir build && cd build

# 配置项目
cmake ..

# 编译
cmake --build . --config Release

# 运行游戏
./my2048  # Linux
# 或
my2048.exe  # Windows
```

## 🎮 游戏玩法

### 基本操作

#### 经典模式
- **方向键** ↑↓←→ : 移动方块
- **R键** : 重新开始游戏
- **P键** : 暂停/继续游戏
- **Y键** : 继续游戏（在对话框中）
- **M键** : 返回主菜单
- **Esc键** : 退出确认

#### 对角线模式
- **Q键** : 左上方向移动
- **E键** : 右上方向移动
- **Z键** : 左下方向移动
- **C键** : 右下方向移动

### 游戏目标

- **主要目标**: 合成数字16（可在代码中调整为2048）
- **次要目标**: 获得尽可能高的分数
- **挑战**: 在网格填满前达成目标

### 游戏规则

1. 每次移动后会在空位随机生成2或4
2. 相同数字的方块碰撞时会合并
3. 合并后的数字翻倍并计入分数
4. 无法移动且网格填满时游戏结束

## 🛠️ 编译指南

### 项目结构

```
my2048/
├── src/
│   ├── game/           # 游戏核心逻辑
│   │   ├── Game2048.h
│   │   └── Game2048.cpp
│   ├── gif/            # GIF处理模块
│   │   ├── gif_wrapper.h
│   │   └── gif_wrapper.cpp
│   └── main.cpp        # 程序入口
├── assets/
│   ├── fonts/          # 字体文件
│   └── picture/        # 游戏素材
├── CMakeLists.txt      # CMake配置
└── README.md
```

### 自定义配置

#### 修改胜利条件
```cpp
// 在 Game2048.cpp 中修改
constexpr int WIN_VALUE = 16; // 改为 2048 或其他值
```

#### 调整动画速度
```cpp
// 在 Game2048.cpp 中修改
constexpr float GIF_MOVE_SPEED = 100.0f; // 调整GIF移动速度
```

#### 添加新的网格尺寸
```cpp
// 在 setupMainMenu() 中添加新按钮
const std::array<std::string, 4> sizeLabels = {"4 x 4", "5 x 5", "6 x 6", "7 x 7"};
```

## 🎨 素材说明

### GIF动画文件
- `2.gif` - 主菜单背景动画
- `4.gif, 8.gif, 16.gif...` - 对应数字的方块动画
- `32768.jpg` - 高数值方块的静态图片

### 图片文件
- `win.jpg` - 胜利界面图片
- `lose.jpg` - 失败界面图片

### 字体支持
项目支持多种中文字体，优先级顺序：
1. 思源黑体 (SourceHanSansSC.otf)
2. 黑体 (simhei.ttf)
3. 微软雅黑 (msyh.ttc)
4. 宋体 (simsun.ttc)

## 🤝 贡献指南

我们欢迎所有形式的贡献！

### 如何贡献

1. **Fork** 本项目
2. 创建您的特性分支 (`git checkout -b feature/AmazingFeature`)
3. 提交您的更改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 打开一个 **Pull Request**

### 开发规范

- 遵循现有的代码风格
- 添加适当的注释
- 确保代码能够正常编译
- 测试新功能的稳定性

### 问题报告

如果您发现了bug或有功能建议，请：

1. 检查是否已有相关issue
2. 创建新的issue并详细描述问题
3. 提供复现步骤和环境信息

## 📄 许可证

本项目采用 MIT 许可证 - 查看 [LICENSE](LICENSE) 文件了解详情。

## 🙏 致谢

- [SFML](https://www.sfml-dev.org/) - 优秀的多媒体库
- [gif-h](https://github.com/charlietangora/gif-h) - GIF处理库
- 所有贡献者和测试者

## 📞 联系方式

- 项目链接: [https://github.com/yourusername/my2048](https://github.com/yourusername/my2048)
- 问题反馈: [Issues](https://github.com/yourusername/my2048/issues)

---

<div align="center">

**如果这个项目对您有帮助，请给它一个 ⭐️**

Made with ❤️ by [Your Name]

</div> 