# tests/CMakeLists.txt

cmake_minimum_required(VERSION 3.10)
project(ViewTerminalTests)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 引入GoogleTest
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/heads/main.zip
)
FetchContent_MakeAvailable(googletest)

enable_testing()

# 加载主项目 (ViewTerminal) 的库
# 注意：这里假设tests的上一级目录就是主CMakeLists.txt所在目录
add_subdirectory(.. ViewTerminal_build)

# 添加测试可执行文件
add_executable(renderer_test RendererTest.cpp)

# 链接 ViewTerminal库 和 GoogleTest库
target_link_libraries(renderer_test
  ViewTerminal
  gtest
  gtest_main
  pthread
)

# 包含头文件路径
target_include_directories(renderer_test PRIVATE
  ${CMAKE_SOURCE_DIR}/../include
  ${CMAKE_SOURCE_DIR}/../include/viewterminal
)
