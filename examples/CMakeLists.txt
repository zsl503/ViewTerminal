cmake_minimum_required(VERSION 3.10)

# HelloWorld示例
add_executable(AllComponentsDemo AllComponentsDemo.cpp)
# add_executable(HelloWorld HelloWorld.cpp)

# 链接ViewTerminal库
target_link_libraries(AllComponentsDemo PRIVATE ViewTerminal)

# 包含必要的头文件路径
target_include_directories(AllComponentsDemo PRIVATE 
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/viewterminal
)
