#include "App.h"
#include <gtest/gtest.h>

// 用于测试的Renderer派生类，重写与控制台交互的函数

class RendererTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 每个测试前执行
    }

    void TearDown() override {
        // 每个测试后执行
    }
};

TEST_F(RendererTest, InitializeTest) {
    // 测试初始化功能
    const COOD_INT width = 80;
    const COOD_INT height = 25;
    View::Renderer renderer(width, height);
    
    // 验证缓冲区尺寸
    ASSERT_EQ(renderer.getCurrentBuffer().size(), height);
    ASSERT_EQ(renderer.getNewBuffer().size(), height);
    
    for (COOD_INT y = 0; y < height; y++) {
        ASSERT_EQ(renderer.getCurrentBuffer()[y].size(), width);
        ASSERT_EQ(renderer.getNewBuffer()[y].size(), width);
    }
    
    // 验证缓冲区是否被清空(全是空格)
    for (COOD_INT y = 0; y < height; y++) {
        for (COOD_INT x = 0; x < width; x++) {
            ASSERT_EQ(renderer.getCurrentBuffer()[y][x].character, L' ');
            ASSERT_EQ(renderer.getNewBuffer()[y][x].character, L' ');
        }
    }
}

TEST_F(RendererTest, WriteCharTest) {
    View::Renderer renderer(80, 25);
    
    // 测试写入单个字符
    View::Point poCOOD_INT(10, 5);
    renderer.writeChar(poCOOD_INT, L'X', View::Color::RED, View::Color::BLACK);
    
    // 验证新缓冲区状态
    ASSERT_EQ(renderer.getNewBuffer()[5][10].character, L'X');
    ASSERT_EQ(renderer.getNewBuffer()[5][10].attributes, 
              renderer.getColorAttribute(View::Color::RED, View::Color::BLACK));
    
    // 验证脏区域标记
    ASSERT_EQ(renderer.getDirtyRegions().size(), 1);
    ASSERT_EQ(renderer.getDirtyRegions()[0].left, 10);
    ASSERT_EQ(renderer.getDirtyRegions()[0].top, 5);
    ASSERT_EQ(renderer.getDirtyRegions()[0].right, 10);
    ASSERT_EQ(renderer.getDirtyRegions()[0].bottom, 5);
    
    // 测试边界条件
    renderer.writeChar(View::Point(-1, -1), L'Y', View::Color::RED, View::Color::BLACK);
    // 无效坐标不应该改变脏区域数量
    ASSERT_EQ(renderer.getDirtyRegions().size(), 1);
}

TEST_F(RendererTest, WriteStringTest) {
    View::Renderer renderer(80, 25);
    
    // 测试写入字符串
    View::Point poCOOD_INT(5, 10);
    std::wstring str = L"Hello, World!";
    renderer.writeString(poCOOD_INT, str, View::Color::GREEN, View::Color::BLACK);
    
    // 验证新缓冲区状态
    for (size_t i = 0; i < str.length(); i++) {
        ASSERT_EQ(renderer.getNewBuffer()[10][5 + i].character, str[i]);
        ASSERT_EQ(renderer.getNewBuffer()[10][5 + i].attributes, 
                 renderer.getColorAttribute(View::Color::GREEN, View::Color::BLACK));
    }
    
    // 验证脏区域标记
    ASSERT_EQ(renderer.getDirtyRegions().size(), 1);
    ASSERT_EQ(renderer.getDirtyRegions()[0].left, 5);
    ASSERT_EQ(renderer.getDirtyRegions()[0].top, 10);
    ASSERT_EQ(renderer.getDirtyRegions()[0].right, 5 + str.length() - 1);
    ASSERT_EQ(renderer.getDirtyRegions()[0].bottom, 10);
}

TEST_F(RendererTest, FillRectTest) {
    View::Renderer renderer(80, 25);
    
    // 测试填充矩形
    View::Point topLeft(5, 5);
    COOD_INT rectWidth = 10;
    COOD_INT rectHeight = 5;
    renderer.fillRect(topLeft, rectWidth, rectHeight, L'#', View::Color::BLUE, View::Color::BLACK);
    
    // 验证新缓冲区状态
    for (COOD_INT y = 5; y < 5 + rectHeight; y++) {
        for (COOD_INT x = 5; x < 5 + rectWidth; x++) {
            ASSERT_EQ(renderer.getNewBuffer()[y][x].character, L'#');
            ASSERT_EQ(renderer.getNewBuffer()[y][x].attributes, 
                     renderer.getColorAttribute(View::Color::BLUE, View::Color::BLACK));
        }
    }
    
    // 验证脏区域标记
    ASSERT_EQ(renderer.getDirtyRegions().size(), 1);
    ASSERT_EQ(renderer.getDirtyRegions()[0].left, 5);
    ASSERT_EQ(renderer.getDirtyRegions()[0].top, 5);
    ASSERT_EQ(renderer.getDirtyRegions()[0].right, 5 + rectWidth - 1);
    ASSERT_EQ(renderer.getDirtyRegions()[0].bottom, 5 + rectHeight - 1);
}

TEST_F(RendererTest, ClearRectTest) {
    View::Renderer renderer(80, 25);
    
    // 先填充一个区域
    View::Point topLeft(5, 5);
    COOD_INT rectWidth = 10;
    COOD_INT rectHeight = 5;
    renderer.fillRect(topLeft, rectWidth, rectHeight, L'#', View::Color::BLUE, View::Color::BLACK);
    
    // 渲染并清除脏区域标记
    renderer.render();
    
    // 然后清除该区域
    renderer.clearRect(topLeft, rectWidth, rectHeight, View::Color::BLACK);
    
    // 验证新缓冲区状态
    for (COOD_INT y = 5; y < 5 + rectHeight; y++) {
        for (COOD_INT x = 5; x < 5 + rectWidth; x++) {
            ASSERT_EQ(renderer.getNewBuffer()[y][x].character, L' ');
            ASSERT_EQ(renderer.getNewBuffer()[y][x].attributes, 
                     renderer.getColorAttribute(View::Color::WHITE, View::Color::BLACK));
        }
    }
}

TEST_F(RendererTest, DrawBorderTest) {
    View::Renderer renderer(80, 25);
    
    // 测试绘制边框
    View::Point topLeft(5, 5);
    COOD_INT rectWidth = 10;
    COOD_INT rectHeight = 5;
    renderer.drawBorder(topLeft, rectWidth, rectHeight, View::Color::YELLOW, View::Color::BLACK);
    
    // 验证边框字符
    // 左上角
    ASSERT_EQ(renderer.getNewBuffer()[5][5].character, L'╭');
    // 右上角
    ASSERT_EQ(renderer.getNewBuffer()[5][14].character, L'╮');
    // 左下角
    ASSERT_EQ(renderer.getNewBuffer()[9][5].character, L'╰');
    // 右下角
    ASSERT_EQ(renderer.getNewBuffer()[9][14].character, L'╯');
    
    // 验证水平边框
    for (COOD_INT x = 6; x < 14; x++) {
        ASSERT_EQ(renderer.getNewBuffer()[5][x].character, L'─');
        ASSERT_EQ(renderer.getNewBuffer()[9][x].character, L'─');
    }
    
    // 验证垂直边框
    for (COOD_INT y = 6; y < 9; y++) {
        ASSERT_EQ(renderer.getNewBuffer()[y][5].character, L'│');
        ASSERT_EQ(renderer.getNewBuffer()[y][14].character, L'│');
    }
}

TEST_F(RendererTest, MarkDirtyRegionsTest) {
    View::Renderer renderer(80, 25);
    
    // 标记两个相邻的脏区域
    renderer.markDirty(View::Point(5, 5), 10, 5);
    renderer.markDirty(View::Point(10, 5), 10, 5);
    
    // 验证脏区域合并
    ASSERT_EQ(renderer.getDirtyRegions().size(), 1);
    ASSERT_EQ(renderer.getDirtyRegions()[0].left, 5);
    ASSERT_EQ(renderer.getDirtyRegions()[0].top, 5);
    ASSERT_EQ(renderer.getDirtyRegions()[0].right, 19);
    ASSERT_EQ(renderer.getDirtyRegions()[0].bottom, 9);
    
    // 标记一个不重叠的区域
    renderer.markDirty(View::Point(30, 15), 5, 5);
    
    // 验证现在有两个脏区域
    ASSERT_EQ(renderer.getDirtyRegions().size(), 2);
}

TEST_F(RendererTest, RenderTest) {
    View::Renderer renderer(80, 25);
    
    // 执行几个绘制操作
    renderer.writeChar(View::Point(5, 5), L'A', View::Color::RED, View::Color::BLACK);
    renderer.writeString(View::Point(10, 10), L"Test", View::Color::GREEN, View::Color::BLACK);
    renderer.fillRect(View::Point(20, 15), 5, 3, L'#', View::Color::BLUE, View::Color::BLACK);
    
    // 脏区域数量应该是3
    ASSERT_EQ(renderer.getDirtyRegions().size(), 3);
    
    // 重置计数器
    // renderer.writeCharCount = 0;
    // renderer.writeAttrCount = 0;
    
    // 执行渲染
    renderer.render();
    
    // 验证写入次数与脏区域数量匹配
    // ASSERT_EQ(renderer.writeCharCount, 3); // 每个脏区域一次写入
    // ASSERT_EQ(renderer.writeAttrCount, 3);
    
    // 渲染后脏区域应该被清空
    ASSERT_EQ(renderer.getDirtyRegions().size(), 0);
}

// 主函数
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
