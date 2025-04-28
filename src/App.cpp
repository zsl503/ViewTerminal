#include "pch.h"
#include "App.h"
#include <Logger.h>
#include <wchar.h> // For wcwidth

using namespace View;


// Renderer 实现
Renderer::Renderer(COOD_INT width, COOD_INT height)
    : width(width), height(height)
{
    outputHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    // setlocale(LC_ALL, "");
    CONSOLE_CURSOR_INFO CursorInfo;
    GetConsoleCursorInfo(outputHandle, &CursorInfo);
    CursorInfo.bVisible = false; //隐藏控制台光标
    SetConsoleCursorInfo(outputHandle, &CursorInfo);//设置控制台光标状态
    initialize();
}

void Renderer::initialize()
{
    initBuffers();
    clear();
}

void Renderer::initBuffers()
{
    // 初始化缓冲区
    currentBuffer.resize(height);
    newBuffer.resize(height);
    
    for (COOD_INT y = 0; y < height; y++) {
        currentBuffer[y].resize(width, {L' ', (WORD)(int(Color::WHITE) + int(Color::BLACK) * 0x10)});
        newBuffer[y].resize(width, {L' ', (WORD)(int(Color::WHITE) + int(Color::BLACK) * 0x10)});
    }
}

void Renderer::clear()
{
    // 清空整个缓冲区
    WORD attr = getColorAttribute(Color::WHITE, Color::BLACK);
    for (COOD_INT y = 0; y < height; y++) {
        for (COOD_INT x = 0; x < width; x++) {
            newBuffer[y][x] = {L' ', attr};
        }
    }
    
    // 标记整个屏幕为脏区域
    markDirty(Point(0, 0), width, height);
    
    // 渲染
    render();
}

WORD Renderer::getColorAttribute(Color foreColor, Color backColor) const
{
    return (WORD)(int(foreColor) + int(backColor) * 0x10);
}

void Renderer::writeChar(const Point& position, wchar_t ch, Color foreColor, Color backColor)
{
    COOD_INT x = position.x;
    COOD_INT y = position.y;
    
    if (!isValidPosition(x, y)) {
        LOG_DEBUG(L"Renderer::writeChar: Invalid position: " + std::to_wstring(x) + L" " + std::to_wstring(y));
        return;
    }
    int charWidth = GetCharacterConsoleWidth(ch); // 获取当前字符占用的单元格宽度

    WORD attr = getColorAttribute(foreColor, backColor);
    newBuffer[y][x] = {ch, attr};
    if(charWidth == 1 && isValidPosition(x, y)) {
        newBuffer[y][x] = {ch, attr};
    }
    else if (charWidth == 2 && (isValidPosition(x, y) && isValidPosition(x + 1, y))) {
        newBuffer[y][x] = {ch, attr};
        newBuffer[y][x + 1] = {L'\0', attr}; // 将第二个单元格设置为 L'\0' 并继承属性
    }
    else {
        LOG_ERROR(L"writeString: Invalid position for character: " + std::to_wstring(x) + L" " + std::to_wstring(y) + L" " + ch);
        return;
    }
    // 标记脏区域
    markDirty(position, charWidth, 1);
}

// 修正后的 Renderer::writeString 实现
void Renderer::writeString(const Point& position, const std::wstring& str, Color foreColor, Color backColor)
{
    COOD_INT cellX = position.x; // 当前写入的单元格X坐标
    COOD_INT cellY = position.y; // 当前写入的单元格Y坐标

    // 检查起始位置是否有效
    if (!isValidPosition(cellX, cellY)) {
        return;
    }

    WORD attr = getColorAttribute(foreColor, backColor);
    COOD_INT startCellX = cellX; // 记录字符串开始写入的单元格X坐标

    // 遍历字符串中的每一个 wchar_t 字符
    for (size_t i = 0; i < str.length() && cellX < width; ++i) {
        wchar_t ch = str[i];
        int charWidth = GetCharacterConsoleWidth(ch); // 获取当前字符占用的单元格宽度

        
        if(charWidth == 1 && isValidPosition(cellX, cellY)) {
            newBuffer[cellY][cellX] = {ch, attr};
        }
        else if (charWidth == 2 && (isValidPosition(cellX, cellY) && isValidPosition(cellX + 1, cellY))) {
            newBuffer[cellY][cellX] = {ch, attr};
            newBuffer[cellY][cellX + 1] = {L'\0', attr}; // 将第二个单元格设置为 L'\0' 并继承属性
        }
        else {
            LOG_ERROR(L"writeString: Invalid position for character: " + std::to_wstring(cellX) + L" " + std::to_wstring(cellY) + L" " + ch);
            return;
        }
        // 根据字符宽度，移动到下一个字符应该开始写入的单元格位置
        cellX += charWidth;
    }

    // 标记脏区域：从起始位置开始，宽度是字符串实际占用的单元格总数
    // totalCellsCovered 是字符串写入结束后，当前的 cellX 与起始 startCellX 的差值
    COOD_INT totalCellsCovered = cellX - startCellX;
    if (totalCellsCovered > 0) {
        markDirty(position, totalCellsCovered, 1);
    }
}


void Renderer::fillRect(const Point& topLeft, COOD_INT rectWidth, COOD_INT rectHeight, wchar_t ch, Color foreColor, Color backColor)
{
    COOD_INT startX = topLeft.x;
    COOD_INT startY = topLeft.y;
    
    // 使用 std::max/std::min 来处理负坐标和超出屏幕的情况
    COOD_INT clampedStartX = std::max<COOD_INT>(0, startX);
    COOD_INT clampedStartY = std::max<COOD_INT>(0, startY);
    COOD_INT clampedEndX = std::min<COOD_INT>(startX + rectWidth, width);
    COOD_INT clampedEndY = std::min<COOD_INT>(startY + rectHeight, height);
    
    if (clampedStartX >= clampedEndX || clampedStartY >= clampedEndY) {
        return;
    }

    WORD attr = getColorAttribute(foreColor, backColor);
    int fillCharWidth = GetCharacterConsoleWidth(ch); // 获取填充字符的单元格宽度

    for (COOD_INT y = clampedStartY; y < clampedEndY; y++) {
        for (COOD_INT x = clampedStartX; x < clampedEndX; x++) {
            // 根据填充字符的宽度，决定当前单元格应该存储什么

            if (fillCharWidth == 1) {
                // 单宽字符：每个单元格都存储字符本身
                newBuffer[y][x] = {ch, attr};
            } else { 
                if ((x - clampedStartX) % 2 == 0) {
                    newBuffer[y][x] = {ch, attr};
                } else {
                    newBuffer[y][x] = {L'\0', attr}; // 使用 L'\0' 作为占位符
                }
            }
        }
    }

    markDirty(topLeft, rectWidth, rectHeight);
}

void Renderer::clearRect(const Point& topLeft, COOD_INT rectWidth, COOD_INT rectHeight, Color backColor)
{
    fillRect(topLeft, rectWidth, rectHeight, L' ', Color::WHITE, backColor);
}

void Renderer::drawBorder(const Point& topLeft, COOD_INT rectWidth, COOD_INT rectHeight, Color foreColor, Color backColor)
{
    COOD_INT startX = topLeft.x;
    COOD_INT startY = topLeft.y;
    
    // 确保矩形在屏幕内
    if (startX < 0) startX = 0;
    if (startY < 0) startY = 0;
    if (startX >= width || startY >= height) return;
    
    COOD_INT endX = std::min<COOD_INT>(startX + rectWidth - 1, width - 1);
    COOD_INT endY = std::min<COOD_INT>(startY + rectHeight - 1, height - 1);
    WORD attr = getColorAttribute(foreColor, backColor);
    
    // 只需要绘制矩形四边
    // 如果区域足够大，绘制边框角和边
    if (rectWidth >= 2 && rectHeight >= 2) {
        // 左上角
        if (isValidPosition(startX, startY))
            newBuffer[startY][startX] = {L'╭', attr};
        
        // 右上角
        if (isValidPosition(endX, startY))
            newBuffer[startY][endX] = {L'╮', attr};
        
        // 左下角
        if (isValidPosition(startX, endY))
            newBuffer[endY][startX] = {L'╰', attr};
        
        // 右下角
        if (isValidPosition(endX, endY))
            newBuffer[endY][endX] = {L'╯', attr};
        
        // 上边
        for (COOD_INT x = startX + 1; x < endX; x++) {
            if (isValidPosition(x, startY))
                newBuffer[startY][x] = {L'─', attr};
        }
        
        // 下边
        for (COOD_INT x = startX + 1; x < endX; x++) {
            if (isValidPosition(x, endY))
                newBuffer[endY][x] = {L'─', attr};
        }
        
        // 左边
        for (COOD_INT y = startY + 1; y < endY; y++) {
            if (isValidPosition(startX, y))
                newBuffer[y][startX] = {L'│', attr};
        }
        
        // 右边
        for (COOD_INT y = startY + 1; y < endY; y++) {
            if (isValidPosition(endX, y))
                newBuffer[y][endX] = {L'│', attr};
        }
    }
    
    // 标记脏区域
    markDirty(topLeft, rectWidth, rectHeight);
}

void Renderer::markDirty(const Point& topLeft, COOD_INT rectWidth, COOD_INT rectHeight)
{
    COOD_INT left = std::max<COOD_INT>(0, std::min<COOD_INT>(topLeft.x, width - 1));
    COOD_INT top = std::max<COOD_INT>(0, std::min<COOD_INT>(topLeft.y, height - 1));
    COOD_INT right = std::max<COOD_INT>(0, std::min(topLeft.x + rectWidth - 1, width - 1));
    COOD_INT bottom = std::max<COOD_INT>(0, std::min(topLeft.y + rectHeight - 1, height - 1));
    LOG_DEBUG(L"Renderer::markDirty: (" + std::to_wstring(left) + L", " + std::to_wstring(top) + L") - (" +
        std::to_wstring(right) + L", " + std::to_wstring(bottom) + L")");
    if (left <= right && top <= bottom) {
        // 合并重叠的脏区域以优化性能
        for (auto& region : dirtyRegions) {
            // 检查两个矩形是否重叠
            if (!(region.right < left || region.left > right || 
                  region.bottom < top || region.top > bottom)) {
                // 合并矩形
                region.left = std::min<COOD_INT>(region.left, left);
                region.top = std::min<COOD_INT>(region.top, top);
                region.right = std::max<COOD_INT>(region.right, right);
                region.bottom = std::max<COOD_INT>(region.bottom, bottom);
                return;
            }
        }
        
        // 如果没有找到重叠区域，添加新的脏区域
        dirtyRegions.push_back({left, top, right, bottom});
    }
}

void Renderer::render()
{
    if (dirtyRegions.empty()) {
        return;
    }
    
    // 渲染差异区域
    renderDifferences();
    
    // 清除脏区域标记
    dirtyRegions.clear();
}

void Renderer::renderDifferences()
{

    // 遍历所有脏区域
    for (const auto& region : dirtyRegions) {

        for (COOD_INT y = region.top; y <= region.bottom; y++) {
            COOD_INT startX = region.left;

            while (startX <= region.right) {
                // 1. 查找连续的与当前缓冲区相同的区域，跳过它们
                // 比较字符和属性
                while (startX <= region.right &&
                       currentBuffer[y][startX].character == newBuffer[y][startX].character &&
                       currentBuffer[y][startX].attributes == newBuffer[y][startX].attributes)
                {
                    startX++;
                }

                if (startX > region.right) break; // 当前行剩余部分都相同

                // 2. 从第一个不同单元格 startX 开始，找到连续的、**新属性相同**且与当前缓冲区**不同**的段
                WORD segmentAttr = newBuffer[y][startX].attributes; // 获取该段的目标属性

                COOD_INT endX = startX;
                // 找到段的结束点 endX（不包含），即第一个不符合条件的单元格索引
                // 条件：endX 在区域内 并且 新缓冲区的属性与段属性相同 并且 新缓冲区的内容与当前缓冲区不同
                while (endX <= region.right &&
                       newBuffer[y][endX].attributes == segmentAttr &&
                       (currentBuffer[y][endX].character != newBuffer[y][endX].character ||
                        currentBuffer[y][endX].attributes != newBuffer[y][endX].attributes))
                {
                     endX++;
                }

                // 段的范围是 [startX, endX - 1]，单元格数量为 endX - startX
                COOD_INT numCells = endX - startX;

                // 3. 渲染这个段
                if (numCells > 0) {
                    COORD writePos = {(SHORT)startX, (SHORT)y};

                    // 设置整个段的文本属性
                    SetConsoleTextAttribute(outputHandle, segmentAttr);

                    // 准备需要写入的字符数据
                    std::vector<wchar_t> chars(numCells);
                    for (int i = 0; i < numCells; ++i) {
                        chars[i] = newBuffer[y][startX + i].character;
                    }

                    // 写入字符：使用 WriteConsoleW 批量写入该段的字符
                    SetConsoleCursorPosition(outputHandle, writePos); // 移动光标到段的起始位置
                    DWORD writtenCount;
                    // WriteConsoleW 写入 numCells 个 wchar_t 字符
                    // 在设置了属性后，WriteConsoleW 应该会使用当前属性来写入字符
                    WriteConsoleW(outputHandle, chars.data(), numCells, &writtenCount, NULL);
                }

                // 4. 更新 currentBuffer 中刚刚渲染的区域
                for (int i = 0; i < numCells; ++i) {
                    currentBuffer[y][startX + i] = newBuffer[y][startX + i];
                }

                // 将 startX 移动到当前段的结束之后，继续查找下一个不同的段
                startX = endX;
            }
        }
    }
}

COORD Renderer::pointToCoord(const Point& point)
{
    COORD coord = {(SHORT)point.x, (SHORT)point.y};
    return coord;
}

Point Renderer::coordToPoint(const COORD& coord)
{
    return Point(coord.X, coord.Y);
}

bool Renderer::isValidPosition(COOD_INT x, COOD_INT y) const
{
    return x >= 0 && y >= 0 && x < width && y < height;
}

// 修改 App 的构造函数，初始化 renderer
App::App(COOD_INT width, COOD_INT height):width(width), height(height), render(width, height)
{
    inputHandle = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode = ENABLE_INSERT_MODE | ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT | ENABLE_MOUSE_INPUT | ENABLE_PROCESSED_INPUT;
    SetConsoleMode(inputHandle, mode);
}

void App::redirectTo(std::wstring newPage)
{
    if (pageStack.empty())
    {
        openPage(newPage)->create();
        processDestroyed();
    }
    else
    {
        type = Type::RED;
        this->newPage = newPage;
        pageStack.top()->destroy();
    }
}

void View::App::reLaunch(std::wstring newPage)
{
    if (pageStack.empty())
    {
        openPage(newPage)->create();
        processDestroyed();
    }
    else
    {
        type = Type::REL;
        this->newPage = newPage;
        pageStack.top()->destroy();
    }

}

void View::App::navigateTo(std::wstring newPage)
{
    if (pageStack.empty())
    {
        LOG_DEBUG(L"App::navigateTo: " + newPage);
        openPage(newPage)->create();
        processDestroyed();
    }
    else
    {
        type = Type::NAT;
        this->newPage = newPage;
        pageStack.top()->destroy();
    }
}

COOD_INT View::App::getWidth()
{
    return width;
}

COOD_INT View::App::getHeight()
{
    return height;
}

Renderer& View::App::getRenderer()
{
    return render;
}

void View::App::navigateBack()
{
    type = Type::NAB;
    pageStack.top()->destroy();
}

void App::registerPages(std::map<std::wstring, std::unique_ptr<View::PageBase>>& route)
{
    this->route = std::move(route);
}

// 修改 App::openPage 方法，传递 renderer 给 PageBase
std::unique_ptr<View::PageBase>& App::openPage(std::wstring name)
{
    LOG_DEBUG(L"App::openPage: " + name);
    auto iter = route.find(name);
    if (iter != route.end()) {
        pageStack.push(iter->second.get());
        iter->second->initPage(this, name, inputHandle);
        return iter->second;
    }
    else {
        throw std::runtime_error("Page not found: " + std::string(name.begin(), name.end()));
    }
}

void App::processDestroyed()
{
    auto tmpType = type;
    switch (tmpType)
    {
    case RED:
    {
        if (pageStack.size() > 0)
        {
            pageStack.pop();
        }
        openPage(newPage)->create();
        processDestroyed();
        break;
    }
    case REL:
    {
        while (!pageStack.empty())
        {
            pageStack.pop();
        }
        openPage(newPage)->create();
        processDestroyed();
        break;
    }
    case NAT:
    {
        openPage(newPage)->create();
        processDestroyed();
        break;
    }
    case NAB:
    {
        if (pageStack.size() > 1)
        {
            pageStack.pop();
            pageStack.top()->create();
            processDestroyed();
        }
        break;
    }
    default:
        break;
    }
}

/*-----------------PageBase-----------------*/
void View::PageBase::initPage(
    App* app, std::wstring name, HANDLE inputHandle
)
{
    this->name = name;
    this->app = app;
    this->inputHandle = inputHandle;
    LOG_DEBUG(L"PageBase::initPage: " + name);
    initComponents();
}

View::PageBase::~PageBase()
{
    // 确保监听已停止
    isListen = false;

    // 清空事件队列
    FlushConsoleInputBuffer(inputHandle);
    
    for (auto i : allComponents)
        if (i != NULL)
            delete i;
    clear();
}

void View::PageBase::registerComponents(
    std::set<ComponentBase*>& allComponents,
    std::set<ComponentBase*>& mouseComponents,
    std::set<ComponentBase*>& keyComponents
)
{
    this->allComponents = allComponents;
    this->mouseComponents = mouseComponents;
    this->keyComponents = keyComponents;
}

PageBase &View::PageBase::addComponent(ComponentBase* com)
{
    if (!com) return *this;
    allComponents.insert(com); 
    mouseComponents.insert(com); 
    keyComponents.insert(com); 
    return *this;
}
void View::PageBase::removeComponent(ComponentBase* com)
{
    allComponents.erase(com);
    mouseComponents.erase(com);
    keyComponents.erase(com);
    if (com != NULL)
        delete com;
}

void View::PageBase::detachComponent(ComponentBase* com)
{
    allComponents.erase(com);
    mouseComponents.erase(com);
    keyComponents.erase(com);
}

void View::PageBase::create()
{
    created();
    SetConsoleTitleW((name.c_str()));    
    // 绘制所有组件前先清屏
    render.clear();
    LOG_DEBUG(L"PageBase::create: " + name + L" and clear screen");
    // 绘制所有组件
    for (auto& i : allComponents)
    {
        LOG_DEBUG(L"PageBase::create component: Point(" + std::to_wstring(i->getPoint().x) + L", " + std::to_wstring(i->getPoint().y) + L")");
        i->update(true);
    }
    LOG_DEBUG(L"PageBase::create: all components updated");
    // 渲染所有更改
    render.render();
    LOG_DEBUG(L"PageBase::create: render all changes");
    
    mounted();
    isListen = true;
    eventListener();
}

void View::PageBase::update()
{
    // 只更新需要更新的组件，而不是整个页面
    bool needsRender = false;
    
    for (auto i : allComponents)
    {
        if (i->needChange())
        {
            i->update();
            needsRender = true;
        }
    }
    
    // 如果有组件被更新，执行一次渲染
    if (needsRender)
    {
        render.render();
    }
}

void View::PageBase::destroy()
{
    // 先停止监听，防止事件循环继续处理事件
    isListen = false;
    
    // 清空控制台输入缓冲区
    FlushConsoleInputBuffer(inputHandle);
    // 清屏
    clear();
}

void View::PageBase::clear()
{
    // 使用 Renderer 清屏
    render.clear();
}

void View::PageBase::eventListener()
{
    INPUT_RECORD record;
    DWORD res;
    View::MouseEvent mouE;
    View::KeyEvent keyE;
    ComponentBase* p = NULL;
    
    // 使用帧率限制渲染
    DWORD lastFrameTime = GetTickCount();
    const DWORD frameDelay = 16; // 约60fps
    
    while (isListen)
    {
        // 非阻塞方式检查是否有输入事件
        DWORD numEvents = 0;
        GetNumberOfConsoleInputEvents(inputHandle, &numEvents);
        
        if (numEvents > 0) {
            // 有事件要处理
            if (PeekConsoleInput(inputHandle, &record, 1, &res) && res > 0) {
                // 读取事件但不从队列中移除
                ReadConsoleInput(inputHandle, &record, 1, &res);
                
                if (record.EventType == MOUSE_EVENT)
                {
                    mouE.point = View::Point(record.Event.MouseEvent.dwMousePosition);
                    switch (record.Event.MouseEvent.dwButtonState)
                    {
                    case FROM_LEFT_1ST_BUTTON_PRESSED:
                        mouE.type = record.Event.MouseEvent.dwEventFlags == DOUBLE_CLICK ?
                            View::MouseEventType::LDOUBLECLICK : View::MouseEventType::LCLICK;
                        break;
                    case RIGHTMOST_BUTTON_PRESSED:
                        mouE.type = record.Event.MouseEvent.dwEventFlags == DOUBLE_CLICK ?
                            View::MouseEventType::RDOUBLECLICK : View::MouseEventType::RCLICK;
                        break;
                    default:
                        mouE.type = View::MouseEventType::HOVER;
                        break;
                    }
                    
                    for (auto i : mouseComponents) {
                        i->onMouseEvent(mouE);
                    }
                }
                
            }
        }
        else {
            // 没有事件时休眠一小段时间，减少CPU占用
            Sleep(frameDelay / 2);
        }
        
        // 更新所有标记为需要更新的组件
        DWORD currentTime = GetTickCount();
        if (isListen && (currentTime - lastFrameTime >= frameDelay)) {
            update(); // 调用更新方法，只更新脏组件
            lastFrameTime = currentTime;
        }
    }
}