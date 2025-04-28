#include "pch.h"
#include "ComponentBase.h"
#include <memory> // 添加 <memory> 头文件
#include <algorithm> // 添加 <algorithm> 头文件
#include <Logger.h>
#pragma warning( disable : 4996)
// std::wstring版本
std::vector<std::wstring> View::split(const std::wstring& in, const std::wstring& delim) {
    std::wregex re{ delim };
    return std::vector<std::wstring> {
        std::wsregex_token_iterator(in.begin(), in.end(), re, -1),
            std::wsregex_token_iterator()
    };
}

// 添加一个简单的函数来判断字符在控制台中的宽度（以单元格为单位）
// 注意：这是一个简化的实现，不适用于所有Unicode字符和复杂的字体情况
int GetCharacterConsoleWidth(wchar_t ch) {
    // 常见的单宽字符范围
    if (ch >= 0x0020 && ch <= 0x007E) return 1; // Basic Latin (ASCII 可打印字符)
    if (ch >= 0x00A1 && ch <= 0x00FF) return 1; // Latin-1 Supplement 部分字符

    // 常见的双宽字符范围（例如 CJK Unified Ideographs）
    if (ch >= 0x4E00 && ch <= 0x9FFF) return 2; // CJK Unified Ideographs
    if (ch >= 0x3000 && ch <= 0x303F) return 2; // CJK Symbols and Punctuation
    if (ch >= 0xFF01 && ch <= 0xFF60) return 2; // Fullwidth Forms (全角字符)
    if (ch >= 0xFFE0 && ch <= 0xFFEF) return 2; // Halfwidth and Fullwidth Forms

    // 对于其他未判断的字符，可以根据需要返回默认值。
    // 这里我们默认返回1，但这可能导致某些宽字符显示不正确。
    // 如果主要处理中英文，这个范围基本够用。
    return 1;
}
/*-----------------Point-----------------*/

View::Point::Relation View::Point::getNewPointRelation(const Point& newPoint) const 
{
    if (newPoint.x == x && newPoint.y == y)
        return Relation::SAME;
    else if (newPoint.x > x && newPoint.y == y)
        return Relation::RIGHT;
    else if (newPoint.x < x && newPoint.y == y)
        return Relation::LEFT;
    else if (newPoint.x == x && newPoint.y < y)
        return Relation::UP;
    else if (newPoint.x == x && newPoint.y > y)
        return Relation::DOWN;
    else if (newPoint.x > x && newPoint.y > y)
        return Relation::RIGHTDOWN;
    else if (newPoint.x > x && newPoint.y < y)
        return Relation::RIGHTUP;
    else if (newPoint.x < x && newPoint.y > y)
        return Relation::LEFTDOWN;
    else if (newPoint.x < x && newPoint.y < y)
        return Relation::LEFTUP;
    else
        throw;
}

View::Point View::Point::getAbsPoint(COOD_INT reX, COOD_INT reY) const
{
    return Point(reX + x, reY + y);
}

View::Point View::Point::getAbsPoint(const Point& rePoint) const
{
    return Point(rePoint.x + x, rePoint.y + y);
}

COORD View::Point::getCOORD(COOD_INT reX, COOD_INT reY)
{
    COORD c = { getAbsX(reX), getAbsY(reY) };
    return c;
}

COOD_INT View::Point::getAbsX(COOD_INT reX)
{
    return reX + x;
}

COOD_INT View::Point::getAbsY(COOD_INT reY)
{
    return reY + y;
}

/*-----------------ComponentBase-----------------*/

void View::ComponentBase::setShow(bool isShow)
{
    if (isShow == this->isShow)
        return;
    this->isShow = isShow;
    if (isDrawed != isShow)
        this->isChange = true;
}

bool View::ComponentBase::getShow()
{
    return isShow;
}

bool View::ComponentBase::isPointIn(const Point& point)
{
    Point rightDown = this->point.getAbsPoint(width, height);
    switch (this->point.getNewPointRelation(point))
    {
    case Point::Relation::LEFTDOWN:
    case Point::Relation::LEFTUP:
    case Point::Relation::RIGHTUP:
    case Point::Relation::LEFT:
    case Point::Relation::UP:
        return false;
    }

    switch (rightDown.getNewPointRelation(point))
    {
    case Point::Relation::RIGHTDOWN:
    case Point::Relation::RIGHTUP:
    case Point::Relation::LEFTDOWN:
    case Point::Relation::RIGHT:
    case Point::Relation::DOWN:
        return false;
    }

    return true;
}

bool View::ComponentBase::isPointIn(const COORD& point)
{
    return isPointIn(Point(point));
}

void View::ComponentBase::setPoint(const Point& point)
{
    LOG_DEBUG(L"ComponentBase::setPoint: " + std::to_wstring(point.x) + L" " + std::to_wstring(point.y));
    if (this->point != point) {
        this->point = point;
        if (isDrawed) {
            setChange();
        }
    }
}

void View::ComponentBase::setSize(size_t width, size_t height)
{
    if (this->width != width || this->height != height) {
        this->width = width;
        this->height = height;
        setChange();
    }
}

void View::ComponentBase::setChange()
{
    isChange = true;
}

View::Point View::ComponentBase::getPoint()
{
    return point;
}

void View::ComponentBase::onMouseEvent(MouseEvent& e)
{
    if (!getShow())
        return;

    MouseEventArgs mouseArgs(this, e);
    if (isPointIn(e.point))
    {
        dispatchEvent(EventType::MOUSE_ENTER, &mouseArgs);
        
        switch (e.type)
        {
        case MouseEventType::RCLICK:
        case MouseEventType::RDOUBLECLICK:
        case MouseEventType::LCLICK:
        case MouseEventType::LDOUBLECLICK:
            LOG_DEBUG(L"ComponentBase::onMouseEvent: [Click]:" + std::to_wstring(e.point.x) + L" " + std::to_wstring(e.point.y));
            isHover = false;
            if (!isOnclick)
                dispatchEvent(EventType::CLICK, &mouseArgs);
            break;
        default:
            if (!isHover)
            {
                LOG_DEBUG(L"ComponentBase::onMouseEvent: [MOUSE_OVER]:" + std::to_wstring(e.point.x) + L" " + std::to_wstring(e.point.y));
                dispatchEvent(EventType::MOUSE_OVER, &mouseArgs);
                isHover = true;
            }
            if (isOnclick)
            {
                isOnclick = false;
            }
        }
    }
    else
    {
        if (isHover || isOnclick) {
            dispatchEvent(EventType::MOUSE_LEAVE, &mouseArgs);
        }
        isHover = false;
        isOnclick = false;
    }
}

void View::ComponentBase::onKeyEvent(KeyEvent& e)
{
    // if (keyEventFun && keyEventFun(e, this))
    // {
    //     setChange();
    //     return this;
    // }
    return;
}

View::ComponentBase::~ComponentBase()
{
    clearDraw();
}

void View::ComponentBase::update(bool force)
{
    if (isChange || force) {
        clearDraw(isShow);
        if (isShow) {
            LOG_DEBUG(L"ComponentBase::update: Drawed Component at (" + std::to_wstring(point.x) + L"," + std::to_wstring(point.y) + L")" +
                L" (" + std::to_wstring(width) + L"," + std::to_wstring(height) + L")");
            beginDraw();
            isDrawed = true;
        }
        isChange = false;
    }
}

void View::ComponentBase::clearDraw(bool useBackColor)
{
    if (!isDrawed) return;
    // 清除当前组件的绘制
    if(useBackColor)
        render.clearRect(point, width, height, nowBackColor);
    else
        render.clearRect(point, width, height);
    isDrawed = false;
}

size_t View::ComponentBase::getHeight()
{
    return height;
}

size_t View::ComponentBase::getWidth()
{
    return width;
}

View::Point View::ComponentBase::getRDPoint()
{
    return this->point.getAbsPoint(width - 1, height - 1);
}

void View::ComponentBase::drawBackground()
{
    render.fillRect(point, width, height, L' ', nowForeColor, nowBackColor);
}

void View::ComponentBase::drawBorder()
{
    render.drawBorder(point, width, height, nowForeColor, nowBackColor);
}

void View::ComponentBase::addEventListener(EventType type, const EventHandler& handler)
{
    eventManager.addEventListener(type, handler);
}

void View::ComponentBase::removeEventListeners(EventType type)
{
    eventManager.removeEventListeners(type);
}

void View::ComponentBase::dispatchEvent(EventType type, EventArgs* args)
{
    eventManager.dispatchEvent(type, args);
}

void View::ComponentBase::setColorStyle(View::Color foreColor, View::Color backColor)
{
    // 如果颜色没有变化，就不需要更新
    if (this->foreColor == foreColor && this->backColor == backColor)
        return;
        
    this->nowForeColor = foreColor;
    this->nowBackColor = backColor;
    this->foreColor = foreColor;
    this->backColor = backColor;
    
    // 标记组件需要更新
    setChange();
}

void View::ComponentBase::setFocus()
{
    if (!focus) {
        focus = true;
        
        // 发布获取焦点事件
        EventArgs args(this);
        dispatchEvent(EventType::FOCUS, &args);
        
        // 如果组件具有可视化焦点效果，可以在这里标记它需要更新
        setChange();
    }
}

void View::ComponentBase::unsetFocus()
{
    if (focus) {
        focus = false;
        
        // 发布失去焦点事件
        EventArgs args(this);
        dispatchEvent(EventType::BLUR, &args);
        
        // 如果组件具有可视化焦点效果，可以在这里标记它需要更新
        setChange();
    }
}

/*-----------------Text-----------------*/

View::Point View::Text::drawLineToBuffer(std::wstring text, Point point, bool& isFinish)
{
    size_t maxWidth = width;
    size_t outLineLen = 0;
    size_t line = 0;
    std::wstring buffer;
    buffer.reserve(maxWidth);
    for (size_t i = 0; i < text.size(); i++)
    {
        COOD_INT charWidth = GetCharacterConsoleWidth(text[i]); // 获取当前字符占用的单元格宽度
        
        if (outLineLen + charWidth > maxWidth)
        {
            // 输出当前缓冲区内容
            render.writeString(point.getCOORD(0, line), buffer, nowForeColor, nowBackColor);
            buffer.clear();
            outLineLen = 0;
            line++;
            
            // 检查是否超出垂直边界
            if (point.getAbsY(line) > this->point.getAbsY(height - 1))
            {
                render.writeString(this->point.getCOORD(width - 3, height - 1), L"...", nowForeColor, nowBackColor);
                isFinish = false;
                return point.getAbsPoint(outLineLen, line);
            }
        }
        buffer += text[i];
        outLineLen += charWidth;
    }

    if (!buffer.empty()) {
        render.writeString(point.getCOORD(0, line), buffer, nowForeColor, nowBackColor);
    }

    isFinish = true;
    return point.getAbsPoint(outLineLen, line);
}

void View::Text::beginDraw()
{
    WORD attr = (WORD)(int(nowForeColor) + int(nowBackColor) * 0x10);
    Point tmp(point);
    
    std::vector<std::wstring> splits = split(text, L"\n");
    bool isFinish = false;
    
    for (auto& line : splits)
    {
        isFinish = false;
        endPoint = drawLineToBuffer(line, tmp, isFinish);
        tmp.y = endPoint.y + 1;
        if (!isFinish) break;
    }
}

std::wstring View::Text::setText(std::wstring newText)
{
    std::wstring oldText = text;
    text = newText;
    setChange();
    return oldText;
}

std::wstring View::Text::setText(int newText)
{
    return setText(std::to_wstring(newText));
}

std::wstring View::Text::getText()
{
    return text;
}

/*-----------------Button-----------------*/

void View::Button::addDefaultEventListener() 
{
    addEventListener(EventType::MOUSE_OVER, [this](EventArgs* e) {
        LOG_DEBUG(L"Button::addDefaultEventListener [MOUSE_OVER]");
        if (nowBackColor == hoverBackColor && nowForeColor == hoverForeColor)
            return;
        text.setColorStyle(hoverForeColor, hoverBackColor);
        nowBackColor = hoverBackColor;
        nowForeColor = hoverForeColor;
        setChange();
    });

    addEventListener(EventType::CLICK, [this](EventArgs* e) {
        MouseEventArgs* mouseArgs = static_cast<MouseEventArgs*>(e);
        LOG_DEBUG(L"Button::addDefaultEventListener [CLICK]: " + std::to_wstring(mouseArgs->event.point.x) + L" " + std::to_wstring(mouseArgs->event.point.y));
        switch (mouseArgs->event.type)
        {
            case MouseEventType::LDOUBLECLICK:
            case MouseEventType::LCLICK:
            if (nowBackColor == clickBackColor && nowForeColor == clickForeColor)
                return;
            text.setColorStyle(foreColor, backColor);
            nowBackColor = backColor;
            nowForeColor = foreColor;
            // if (nowBackColor == clickBackColor && nowForeColor == clickForeColor)
            //     return;
            // text.setColorStyle(clickForeColor, clickBackColor);
            // nowBackColor = clickBackColor;
            // nowForeColor = clickForeColor;
            break;
        } 
    });

    addEventListener(EventType::MOUSE_LEAVE, [this](EventArgs* e) {
        LOG_DEBUG(L"Button::addDefaultEventListener [MOUSE_LEAVE]");
        if (nowBackColor == clickBackColor && nowForeColor == clickForeColor)
            return;
        text.setColorStyle(foreColor, backColor);
        nowBackColor = backColor;
        nowForeColor = foreColor;
        setChange();
    });
}

std::wstring View::Button::setText(std::wstring newText)
{
    if (newText.size() > width * height * 2)
        return text.getText();
    std::wstring oldText = text.getText();
    text.setText(newText);
    setChange();
    return oldText;
}

std::wstring View::Button::setText(int newText)
{
    return setText(std::to_wstring(newText));
}

std::wstring View::Button::getText()
{
    return text.getText();
}

void View::Button::beginDraw()
{    
    drawBackground();
    drawBorder();
    text.setPoint(point.getAbsPoint(textAbX, textAbY));
    text.setSize(width - textSubWidth, height - textSubHeight);
    text.setColorStyle(nowForeColor, nowBackColor);
    text.update();
}

void View::Button::setHoverStyle(View::Color foreColor, View::Color backColor)
{
    hoverBackColor = backColor;
    hoverForeColor = foreColor;
}

void View::Button::setClickStyle(View::Color foreColor, View::Color backColor)
{
    clickBackColor = backColor;
    clickForeColor = foreColor;
}

void View::Button::setColorStyle(View::Color foreColor, View::Color backColor)
{
    ComponentBase::setColorStyle(foreColor, backColor);
    text.setColorStyle(foreColor, backColor);
    setChange();
}

/*-----------------List-----------------*/

View::List::List(Renderer& render, const std::vector<std::wstring>& items, size_t abX, size_t abY, size_t width, size_t itemHeight, Color defaultForeColor, Color defaultBackColor)
    : List(render, items, Point(abX, abY), width, itemHeight) {}

View::List::List(Renderer& render, const std::vector<std::wstring>& items, const Point& point, size_t width, size_t itemHeight, Color defaultForeColor, Color defaultBackColor)
    : ComponentBase(render, point, width, itemHeight, defaultForeColor, defaultBackColor)
{
    height = items.size() * itemHeight;
    COOD_INT y = 0;
    for (const auto& i : items)
    {
        buttons.emplace_back(std::make_unique<View::Button>(render, i, point.getAbsPoint(0, y), width, itemHeight));
        y += (itemHeight-1);
    }
}

void View::List::setPoint(const Point& point)
{
    int dx = point.x - this->point.x, dy = point.y - this->point.y;
    for (auto& i : buttons)
    {
        i->setPoint(i->getPoint().getAbsPoint(dx, dy));
    }
    ComponentBase::setPoint(point);
}

void View::List::beginDraw()
{
    // clearDraw(true);
    drawBackground();
    COORD c = point.getCOORD();
    DWORD charsWritten;
    Point p;
    for (auto& button : buttons){
        button->update(true);
    }
    drawBorder();
    // 绘制分隔线 | 取代原来的渲染方式
    // render.fillRect(point, 1, height, L'|', nowForeColor, nowBackColor);
    // render.fillRect(point.getAbsPoint(width - 1, 0), 1, height, L'|', nowForeColor, nowBackColor);
}

/*-----------------Line-----------------*/

void View::Line::beginDraw()
{
    WORD attr = (WORD)(int(nowForeColor) + int(nowBackColor) * 0x10);
    
    if (isHorizontal)
    {
        render.fillRect(point, length, 1, L'─', nowForeColor, nowBackColor);
    }
    else
    {
        render.fillRect(point, 1, length, L'│', nowForeColor, nowBackColor);
    }
}

/*-----------------InputText-----------------*/

// 将文本按组件宽度拆分为多行
std::vector<std::wstring> View::InputText::calculateTextLines(const std::wstring& text) {
    std::vector<std::wstring> textLines;
    std::wstring currentLine;
    int lineWidth = 0;
    
    for (size_t i = 0; i < text.length(); i++) {
        int charWidth = GetCharacterConsoleWidth(text[i]);
        if (lineWidth + charWidth > width - 1) {  // 留出一些边距
            textLines.emplace_back(currentLine);
            currentLine.clear();
            lineWidth = 0;
        }
        currentLine += text[i];
        lineWidth += charWidth;
    }
    if (!currentLine.empty()) {
        textLines.emplace_back(currentLine);
    }
    
    return textLines;
}


// 处理插入字符
bool View::InputText::handleCharacterInput(KEY_EVENT_RECORD ker, std::wstring& newText, int& cursorPos, 
                                         std::vector<std::wstring>& textLines, int& currentLineIdx, int& linePos) {
    std::wstring testText = newText;
    testText.insert(cursorPos, 1, ker.uChar.UnicodeChar);
    
    std::vector<std::wstring> testLines = calculateTextLines(testText);
    
    if (testLines.size() > height) {
        return false;
    }
    
    newText.insert(cursorPos, 1, ker.uChar.UnicodeChar);
    cursorPos++;
    
    textLines = testLines;
    
    renderText(textLines);
    
    updateLineAndPosition(textLines, cursorPos, currentLineIdx, linePos);
    
    return true;
}

// 处理删除字符
void View::InputText::handleDelete(std::wstring& newText, int& cursorPos, std::vector<std::wstring>& textLines, 
                                 int& currentLineIdx, int& linePos, bool isBackspace) {
    if (isBackspace) {
        cursorPos--;
        cursorPos = std::max(0, cursorPos);
    }
    if(cursorPos > 0)
        newText.erase(cursorPos, 1);
    else if(cursorPos == 0)
        newText.clear();
    
    textLines = calculateTextLines(newText);
    
    renderText(textLines);
    
    updateLineAndPosition(textLines, cursorPos, currentLineIdx, linePos);
}


// 显示文本内容（修改：render只调用一次）
void View::InputText::renderText(const std::vector<std::wstring>& textLines) {
    drawBackground();
    for (size_t i = 0; i < textLines.size(); i++) {
        render.writeString(point.getAbsPoint(0, i), textLines[i], nowForeColor, nowBackColor);
    }
    render.render(); // 注意：不要在每行render了，只最后render一次
}

void View::InputText::showAndPositionCursor(int accumulatedWidth, int currentLineIdx) {
    CONSOLE_CURSOR_INFO CursorInfo;
    HANDLE outputHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleCursorInfo(outputHandle, &CursorInfo);
    CursorInfo.bVisible = true;
    SetConsoleCursorInfo(outputHandle, &CursorInfo);
    
    COORD pos = point.getCOORD(accumulatedWidth, currentLineIdx);
    SetConsoleCursorPosition(outputHandle, pos);
}

// 更新当前行和光标在行中的位置
void View::InputText::updateLineAndPosition(const std::vector<std::wstring>& textLines, int cursorPos,
                                             int& currentLineIdx, int& linePos) {
    // --- 添加的检查 ---
    if (textLines.empty()) {
        currentLineIdx = 0;
        linePos = 0;
        return; // 直接返回，不执行后续逻辑
    }
    // -----------------

    currentLineIdx = 0;
    int tempPos = 0;
    for (int i = 0; i < textLines.size(); i++) {
        if (tempPos + textLines[i].length() >= cursorPos) {
            currentLineIdx = i;
            linePos = cursorPos - tempPos;
            return;
        }
        tempPos += textLines[i].length();
    }
    // 如果没找到，放最后 (此处的逻辑在添加空检查后，只会在 textLines 非空的情况下执行，但光标位置超出文本总长度时触发)
    // 这种情况也需要正确处理，确保索引不越界
    currentLineIdx = textLines.size() - 1;
    linePos = textLines.back().length();
    // 额外检查：确保linePos不超过当前行的长度
    if (linePos > textLines[currentLineIdx].length()) {
        linePos = textLines[currentLineIdx].length();
    }
}

// 计算光标显示的列宽
int View::InputText::calculateCursorDisplayPosition(const std::vector<std::wstring>& textLines, int currentLineIdx, int linePos) {
    if (currentLineIdx >= textLines.size()) return 0;
    int accumulatedWidth = 0;
    const std::wstring& line = textLines[currentLineIdx];
    for (int i = 0; i < linePos && i < line.length(); i++) {
        accumulatedWidth += GetCharacterConsoleWidth(line[i]);
    }
    return accumulatedWidth;
}

// 鼠标点击光标位置改进版（修得更准）
int View::InputText::calculateCursorPosition(const std::vector<std::wstring>& textLines, int clickX, int clickY, int& currentLineIdx, int& accumulatedWidth) {
    currentLineIdx = std::clamp(clickY, 0, static_cast<int>(textLines.size()-1));
    const std::wstring& clickedLine = (currentLineIdx < textLines.size()) ? textLines[currentLineIdx] : L"";

    accumulatedWidth = 0;
    int linePos = 0;
    for (size_t i = 0; i < clickedLine.length(); i++) {
        int charWidth = GetCharacterConsoleWidth(clickedLine[i]);
        if (accumulatedWidth + charWidth/2 >= clickX) {
            break;
        }
        accumulatedWidth += charWidth;
        linePos++;
    }

    int cursorPos = 0;
    for (int i = 0; i < currentLineIdx; i++) {
        cursorPos += textLines[i].length();
    }
    cursorPos += linePos;

    return cursorPos;
}

// 处理键盘输入时，每次移动光标都刷新一次
bool View::InputText::handleKeyEvent(KEY_EVENT_RECORD ker, std::wstring& newText, std::wstring& currentText, 
                                   int& cursorPos, std::vector<std::wstring>& textLines, 
                                   int& currentLineIdx, int& linePos, bool& editing) {
    bool changed = false;
    if (ker.wVirtualKeyCode == VK_RETURN) {
        editing = false;
        return true;
    } 
    else if (ker.wVirtualKeyCode == VK_ESCAPE) {
        newText = currentText;
        editing = false;
        return true;
    } 
    else if (ker.wVirtualKeyCode == VK_LEFT && cursorPos > 0) {
        cursorPos--;
        changed = true;
    } 
    else if (ker.wVirtualKeyCode == VK_RIGHT && cursorPos < newText.length()) {
        cursorPos++;
        changed = true;
    }
    else if (ker.wVirtualKeyCode == VK_BACK && cursorPos > 0) {
        handleDelete(newText, cursorPos, textLines, currentLineIdx, linePos, true);
        changed = true;
    }
    else if (ker.wVirtualKeyCode == VK_DELETE && cursorPos < newText.length()) {
        handleDelete(newText, cursorPos, textLines, currentLineIdx, linePos, false);
        changed = true;
    }
    else if (ker.uChar.UnicodeChar != 0) {
        if (handleCharacterInput(ker, newText, cursorPos, textLines, currentLineIdx, linePos)) {
            changed = true;
        }
    }

    if (changed) {
        updateLineAndPosition(textLines, cursorPos, currentLineIdx, linePos);
        int accumulatedWidth = calculateCursorDisplayPosition(textLines, currentLineIdx, linePos);
        showAndPositionCursor(accumulatedWidth, currentLineIdx);
    }
    return changed;
}

// 主函数
void View::InputText::onMouseEvent(MouseEvent& e)
{
    HANDLE outputHandle = render.getOutputHandle();
    SetConsoleTextAttribute(outputHandle, int(defaultForeColor) + int(defaultBackColor) * 0x10);
    if (isPointIn(e.point))
    {
        if (e.type != MouseEventType::HOVER)
        {
            std::wstring currentText = isPlaceholderVisible ? L"" : getText();
            
            std::vector<std::wstring> textLines = calculateTextLines(currentText);
            renderText(textLines);
            
            int clickX = e.point.x - point.x;
            int clickY = e.point.y - point.y;
            
            int currentLineIdx = 0;
            int accumulatedWidth = 0;
            int cursorPos = calculateCursorPosition(textLines, clickX, clickY, currentLineIdx, accumulatedWidth);
            int linePos = cursorPos - (currentLineIdx > 0 ? (cursorPos - accumulatedWidth) : 0);
            
            showAndPositionCursor(accumulatedWidth, currentLineIdx);
            
            INPUT_RECORD inputRecord;
            DWORD numEventsRead;
            std::wstring newText = currentText;
            bool editing = true;
            
            while (editing)
            {
                DWORD numEvents = 0;
                PeekConsoleInputW(GetStdHandle(STD_INPUT_HANDLE), &inputRecord, 1, &numEvents);
                
                if (numEvents > 0) {
                    ReadConsoleInputW(GetStdHandle(STD_INPUT_HANDLE), &inputRecord, 1, &numEventsRead);
                    
                    if (inputRecord.EventType == MOUSE_EVENT && 
                        inputRecord.Event.MouseEvent.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED) {
                        COORD mousePos = inputRecord.Event.MouseEvent.dwMousePosition;
                        Point clickPoint(mousePos.X, mousePos.Y);
                        
                        if (!isPointIn(clickPoint)) {
                            editing = false;
                        } else {
                            int newClickX = clickPoint.x - point.x;
                            int newClickY = clickPoint.y - point.y;
                            
                            cursorPos = calculateCursorPosition(textLines, newClickX, newClickY, currentLineIdx, accumulatedWidth);
                            linePos = cursorPos - (currentLineIdx > 0 ? (cursorPos - accumulatedWidth) : 0);
                            
                            showAndPositionCursor(accumulatedWidth, currentLineIdx);
                        }
                    }
                    
                    else if (inputRecord.EventType == KEY_EVENT && inputRecord.Event.KeyEvent.bKeyDown) {
                        handleKeyEvent(inputRecord.Event.KeyEvent, newText, currentText, 
                                     cursorPos, textLines, currentLineIdx, linePos, editing);
                    }
                }
            }
            
            if (!newText.empty()) {
                setText(newText);
                isPlaceholderVisible = false;
            } else {
                setText(placeholder);
                isPlaceholderVisible = true;
            }
            
            if (textChanged != NULL && newText != currentText) {
                ValueChangedEventArgs valueArgs(this, currentText, newText);
                dispatchEvent(EventType::VALUE_CHANGE, &valueArgs);
                
                textChanged(this);
            }
            
            CONSOLE_CURSOR_INFO CursorInfo;
            GetConsoleCursorInfo(outputHandle, &CursorInfo);
            CursorInfo.bVisible = false;
            SetConsoleCursorInfo(outputHandle, &CursorInfo);
            
            unsetFocus();
            update();
        }
    }
    return ComponentBase::onMouseEvent(e);
}

/*-----------------LayoutBase-----------------*/

void View::LayoutBase::setShow(bool show)
{
    if (show == getShow())return;
    if (show)
    {
        ComponentBase::setShow(show);
    }
    else
    {
        ComponentBase::setShow(show);
    }
}

bool View::LayoutBase::addComponent(std::shared_ptr<ComponentBase> com)
{
    if (!com) return false;
    LOG_DEBUG(L"LayoutBase::addComponent: " + std::to_wstring(com->getWidth()) + L" " + std::to_wstring(com->getHeight()));
    com->setPoint(point.getAbsPoint(com->getPoint()));
    allComponents.emplace_back(com);
    return true;
}

bool View::LayoutBase::removeComponent(ComponentBase* comPtr)
{
    auto iter = std::find_if(allComponents.begin(), allComponents.end(),
                             [comPtr](const std::shared_ptr<ComponentBase>& p) {
                                 return p.get() == comPtr;
                             });

    if (iter != allComponents.end()) {
        iter->reset(); // 释放智能指针
        allComponents.erase(iter);
        return true;
    }
    return false;
}

void View::LayoutBase::beginDraw()
{
    for (auto& i : allComponents)
    {
        i->update();
    }
}

void View::LayoutBase::update(bool force)
{
    if (getShow())
    {
        for (auto& i : allComponents)
        {
            i->update(force);
        }
    }
    else
        clearDraw();
}

void View::LayoutBase::setPoint(const Point& point)
{
    LOG_DEBUG(L"LayoutBase::setPoint: " + std::to_wstring(point.x) + L" " + std::to_wstring(point.y));
    int dx = point.x - this->point.x, dy = point.y - this->point.y;
    for (auto& i : allComponents)
    {
        i->setPoint(i->getPoint().getAbsPoint(dx, dy));
    }
    ComponentBase::setPoint(point);
}

void View::LayoutBase::clear()
{
    allComponents.clear();
}


void View::LayoutBase::onMouseEvent(MouseEvent& e)
{
    if (!getShow())
        return;
    for (auto& i : allComponents) {
        i->onMouseEvent(e);
    }
}

void View::LayoutBase::onKeyEvent(KeyEvent& e)
{
    if (!getShow())
        return;

    ComponentBase* p = NULL;
    for (auto& i : allComponents) {
        i->onKeyEvent(e);
    }
}

/*-----------------Span-----------------*/

void View::Span::setPadding(unsigned short top, unsigned short right, unsigned short bottom, unsigned short left)
{
    padding[0] = top, padding[1] = right, padding[2] = bottom, padding[3] = left;
    currentWidth = padding[3];
    for (auto& com : allComponents)
    {
        com->setPoint(point.getAbsPoint(currentWidth, padding[0]));
        currentWidth += (com->getWidth() + space);
    }
}

bool View::Span::addComponent(std::shared_ptr<ComponentBase> com)
{
    if (!com || currentWidth + com->getWidth() + space > width)
        return false;
    if (LayoutBase::addComponent(com))
    {
        com->setPoint(point.getAbsPoint(currentWidth, padding[0]));
        currentWidth += (com->getWidth() + space);
        return true;
    }
    return false;
}

void View::Span::clear()
{
    LayoutBase::clear();
    currentWidth = padding[3];
}

bool View::Span::removeComponent(ComponentBase* com)
{   
    if (!com) return false;
    int tmp = com->getWidth();
    if(LayoutBase::removeComponent(com)){
        currentWidth -= (tmp + space);
        return true;
    }
    return false;
}

void View::Span::setSpace(size_t space)
{
    this->space = space;
    currentWidth = padding[3];
    for (auto& com : allComponents)
    {
        com->setPoint(point.getAbsPoint(currentWidth, padding[0]));
        currentWidth += (com->getWidth() + space);
    }
}

/*-----------------Div-----------------*/

bool View::Div::addComponent(std::shared_ptr<ComponentBase> com)
{
    if (!com || currentHeight + com->getHeight() + space > height)
        return false;
    LOG_DEBUG(L"Div::addComponent: " + std::to_wstring(currentHeight) + L" " + std::to_wstring(com->getHeight()) + L" " + std::to_wstring(space) + L" " + std::to_wstring(height));
    if (LayoutBase::addComponent(com))
    {
        LOG_DEBUG(L"Div::addComponent setPoint:" + std::to_wstring(point.getAbsPoint(padding[3], currentHeight).x) + L" " + std::to_wstring(point.getAbsPoint(padding[3], currentHeight).y));
        com->setPoint(point.getAbsPoint(padding[3], currentHeight));
        currentHeight += (com->getHeight() + space);
        return true;
    }
    LOG_DEBUG(L"Div::addComponent failed" );
    return false;
}

bool View::Div::removeComponent(ComponentBase* com)
{
    if (!com) return false;
    int tmp = com->getHeight();
    if (LayoutBase::removeComponent(com))
    {
        currentHeight -= (tmp + space);
        return true;
    }
    return false;
}

void View::Div::setSpace(size_t space)
{
    LOG_DEBUG(L"Div::setSpace: " + std::to_wstring(space));
    this->space = space;
    currentHeight = padding[0];
    int i = 0;
    for (auto& com : allComponents)
    {
        i++;
        com->setPoint(point.getAbsPoint(padding[3], currentHeight));
        currentHeight += (com->getHeight() + space);
    }
}

void View::Div::clear()
{
    LayoutBase::clear();
    currentHeight = padding[0];
}

void View::Div::setPadding(unsigned short top, unsigned short right, unsigned short bottom, unsigned short left)
{
    padding[0] = top, padding[1] = right, padding[2] = bottom, padding[3] = left;
    currentHeight = padding[0];
    for (auto& com : allComponents)
    {
        com->setPoint(point.getAbsPoint(padding[3], currentHeight));
        currentHeight += (com->getHeight() + space);
    }
}
