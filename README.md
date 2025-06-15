# 合成耄孩子

<div align="center">

![Game Logo](assets/picture/win.jpg)

**一个基于SFML的现代化2048游戏实现**

[![C++](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![SFML](https://img.shields.io/badge/SFML-2.5+-green.svg)](https://www.sfml-dev.org/)
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-Linux-lightgrey.svg)]()

[功能特性](#功能特性) • [快速开始](#快速开始) • [游戏玩法](#🕹️-游戏玩法) • [编译指南](#编译指南) • [贡献指南](#贡献指南) • [后续规划](#后续规划)

</div>

## 🎮 游戏介绍

哈！耄耋出世！

在遥远的耄耋星上，有一群可爱的小耄耋，几亿年来逍遥自在地生活在这颗璀璨的星球上。

在公元2048年的一天，万恶的人类发现了耄耋星球，并对他们展开了强大的赛博攻势。可爱的小耄耋不想被人类消灭，于是它们展现出了空前的团结！

可爱的小耄耋们有一项特异功能，两个能力值相等的小耄耋可以合体成为一个大耄耋，大耄耋的能力值是小耄耋能力值之和。如果在耄耋星球上，有更多能力值较大的耄耋，那么他们更容易召唤出强大的 "哈气" 之力，从而抵挡住罪恶人类的攻势，赢得耄耋星的保卫战！

耄耋星上的耄耋们有着相当严格的移动规则。由于在耄耋星的不同地区，耄耋们有不同的移动规则。在首都 $M$ 市，耄耋单次只能往前后左右一个方向移动，且不能越过自己的同伴。而在耄耋星的另一边的 $D$ 市，耄耋们只能斜向移动。

耄耋们受过严格的军事训练，所以在长官的一声命令下，所有的耄耋都会尽可能地移动到最远的地方。亲爱的耄耋长官，请问你能否带领耄耋们完成合体大业，以召唤出神秘的 "哈气" 之力吗？

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

- **操作系统**: Ubuntu 18.04+ 或其他Linux发行版
- **编译器**: GCC 7+
- **依赖库**: SFML 2.5+
- **CMake**: 3.10+

### 安装依赖

```bash
sudo apt update
sudo apt install build-essential cmake libsfml-dev
```

### 编译运行

```bash
# 克隆项目
git clone https://github.com/yourusername/Mix_MAO_Kids.git
cd Mix_MAO_Kids

# 创建构建目录
mkdir build && cd build

# 配置项目
cmake ..

# 编译
cmake --build . --config Release

# 运行游戏
./startGame
```

## 🕹️ 游戏玩法

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
│   ├── picture/        # 游戏素材
│   └── music/          # 音乐文件（暂未启用）
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

## 📅 后续规划

### 音频支持（暂缓开发）
由于WSL对音频支持的限制，游戏的音频功能目前处于暂缓开发状态。虽然音乐文件已经准备就绪（存放在`assets/music`目录下），但为了确保在WSL环境下的良好体验，我们决定将音频支持功能推迟到后续版本中实现。

> 音频功能将在以下情况下开启：
> - Windows原生支持完成后
> - 或WSL音频支持更加完善后
> - 或用户在原生Linux环境下运行时

### Windows 平台支持（计划中）
作为后续版本的重要更新内容，我们计划添加完整的Windows平台支持：
- Visual Studio 项目支持
- Windows 特定的构建说明
- SFML在Windows下的配置指南
- 中文字体的自动检测和加载
- 更好的Windows环境适配

> 目前如果您使用Windows系统，建议通过WSL（Windows Subsystem for Linux）来运行游戏。

### 其他计划功能
- 存档功能：保存游戏进度
- 排行榜系统：记录最高分
- 更多动画效果：方块合并特效
- 自定义主题：支持更换背景和方块样式
- 更多游戏模式：计时模式、挑战模式等
- 手机版本适配：触摸屏支持
- 在线对战功能：多人竞技模式

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

- 项目链接: [https://github.com/SunglassmanAlex/Mix_MAO_Kids](https://github.com/SunglassmanAlex/Mix_MAO_Kids)
- 问题反馈: [Issues](https://github.com/SunglassmanAlex/Mix_MAO_Kids/issues)

---

<div align="center">

**如果这个项目对您有帮助，请给它一个 ⭐️**

Made with ❤️ by [Sunglassman (Alex Pan) ]

</div> 