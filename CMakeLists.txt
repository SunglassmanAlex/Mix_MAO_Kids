cmake_minimum_required(VERSION 3.10)
project(My2048)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置UTF-8编码支持
add_compile_definitions(UNICODE _UNICODE)
if(MSVC)
    add_compile_options(/utf-8)
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -finput-charset=UTF-8 -fexec-charset=UTF-8")
endif()

# 查找SFML库
find_package(SFML 2.6 COMPONENTS graphics window system REQUIRED)

# 添加gif-h目录到包含路径
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/gif-h
    ${SFML_INCLUDE_DIR}
)

# 手动列出所有源文件
add_executable(startGame
    src/game/Game2048.cpp
    src/gif/gif_wrapper.cpp
    src/main/main.cpp
)

# 链接SFML库
target_link_libraries(startGame 
    sfml-graphics 
    sfml-window 
    sfml-system
)

# 复制资源文件到构建目录
file(COPY assets DESTINATION ${CMAKE_BINARY_DIR})