cmake_minimum_required(VERSION 3.10)
project(ViewTerminal VERSION 1.0)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 定义源文件
set(SOURCES 
    src/pch.cpp
    src/App.cpp
    src/ComponentBase.cpp
    src/Logger.cpp
)

# 定义头文件
set(HEADERS
    include/viewterminal/pch.h
    include/viewterminal/App.h
    include/viewterminal/ComponentBase.h
    include/viewterminal/Logger.h
)

# 创建库
add_library(ViewTerminal STATIC ${SOURCES})

# 设置头文件路径
target_include_directories(ViewTerminal PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/viewterminal
)

# 添加示例目录
add_subdirectory(examples)
