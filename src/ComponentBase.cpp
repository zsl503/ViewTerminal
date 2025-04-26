#include "pch.h"
#include "ComponentBase.h"
#include <memory> // 添加 <memory> 头文件
#include <algorithm> // 添加 <algorithm> 头文件
#pragma warning( disable : 4996)
// std::wstring版本
std::vector<std::wstring> View::split(const std::wstring& in, const std::wstring& delim) {
    std::wregex re{ delim };
    return std::vector<std::wstring> {
        std::wsregex_token_iterator(in.begin(), in.end(), re, -1),
            std::wsregex_token_iterator()
    };
}
std::string View::UnicodeToANSI(const std::wstring& wstr)
{
    std::string ret;
    mbstate_t state = {};
    const wchar_t* src = wstr.data();
    size_t len = wcsrtombs(nullptr, &src, 0, &state);
    if (static_cast<size_t>(-1) != len) {
        std::unique_ptr< char[] > buff(new char[len + 1]);
        len = wcsrtombs(buff.get(), &src, len, &state);
        if (static_cast<size_t>(-1) != len) {
            ret.assign(buff.get(), len);
        }
    }
    return ret;
}

/*-----------------Point-----------------*/

View::Point::Relation View::Point::getNewPointRelation(Point newPoint)
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

View::Point View::Point::getAbsPoint(COOD_INT reX, COOD_INT reY)
{
    return Point(reX + x, reY + y);
}

View::Point View::Point::getAbsPoint(Point rePoint)
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


void View::ComponentBase::draw()
{
    if (isShow)
    {
        beginDraw();
        isDrawed = true;
    }
}

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


bool View::ComponentBase::isPointIn(Point point)
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

bool View::ComponentBase::isPointIn(COORD point)
{
    return isPointIn(Point(point));
}

void View::ComponentBase::setPoint(Point point)
{
    this->point = point;
    this->setChange();
}

void View::ComponentBase::setSize(size_t width, size_t height)
{
    this->width = width;
    this->height = height;
    setChange();
}

void View::ComponentBase::setChange()
{
    if (isShow)
        isChange = true;
}

View::Point View::ComponentBase::getPoint()
{
    return point;
}

View::ComponentBase* View::ComponentBase::onMouseEvent(MouseEvent& e)
{
    if (mouseEventFun && mouseEventFun(e, this))
    {
        setChange();
        return this;
    }
    return NULL;
}

View::ComponentBase* View::ComponentBase::onKeyEvent(KeyEvent& e)
{
    if (keyEventFun && keyEventFun(e, this))
    {
        setChange();
        return this;
    }
    return NULL;
}

View::ComponentBase::~ComponentBase()
{
    clearDraw();
}

void View::ComponentBase::update()
{
    if (!isChange) return;
    clearDraw();
    draw();
    isChange = false;
}

void View::ComponentBase::clearDraw()
{
    if (!isDrawed) return;
    // SetConsoleTextAttribute(outputHandle, int(defaultForeColor) + int(defaultBackColor) * 10);
    WORD wd = int(defaultForeColor) + int(defaultBackColor) * 0x10; 
    DWORD charsWritten;

    for (int i = 0; i < height; i++)
    {
        FillConsoleOutputCharacterW(outputHandle, L' ', width, point.getCOORD(0, i), &charsWritten);
        FillConsoleOutputAttribute(outputHandle, wd, width, point.getCOORD(0, i), &charsWritten);
    }
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

void View::ComponentBase::drawBorder()
{
    //????

    // 预先准备好边框字符串
    std::wstring topBorder = L"╭" + std::wstring(width - 2, L'─') + L"╮";
    std::wstring bottomBorder = L"╰" + std::wstring(width - 2, L'─') + L"╯";
    
    // 绘制顶部边框(一次性写入)
    DWORD charsWritten;
    SetConsoleCursorPosition(outputHandle, point.getCOORD());
    WriteConsoleW(outputHandle, topBorder.c_str(), (DWORD)topBorder.length(), &charsWritten, NULL);
    
    // 绘制底部边框(一次性写入)
    SetConsoleCursorPosition(outputHandle, point.getCOORD(0, height - 1));
    WriteConsoleW(outputHandle, bottomBorder.c_str(), (DWORD)bottomBorder.length(), &charsWritten, NULL);
    
    // 绘制左右边框
    for (int i = 1; i < height - 1; i++) {
        SetConsoleCursorPosition(outputHandle, point.getCOORD(0, i));
        WriteConsoleW(outputHandle, L"│", 1, &charsWritten, NULL);
        
        SetConsoleCursorPosition(outputHandle, point.getCOORD(width - 1, i));
        WriteConsoleW(outputHandle, L"│", 1, &charsWritten, NULL);
    }

    // COORD c = point.getCOORD();
    
    // const wchar_t *s = std::wstring(width - 2, L'─').c_str();
    
    // SetConsoleCursorPosition(outputHandle, c);
    // wprintf(L"╭");
    // c.X += 1;
    // SetConsoleCursorPosition(outputHandle, c);
    // wprintf(s);
    // c.X += width - 2;
    // SetConsoleCursorPosition(outputHandle, c);
    // wprintf(L"╮");

    // c.X = point.getAbsX(0), c.Y = point.getAbsY(height - 1);
    // SetConsoleCursorPosition(outputHandle, c);
    // wprintf(L"╰");
    // c.X += 1;
    // SetConsoleCursorPosition(outputHandle, c);
    // wprintf(s);
    // c.X += width - 2;
    // SetConsoleCursorPosition(outputHandle, c);
    // wprintf(L"╯");

    // c.X = point.getAbsX(0);
    // for (int i = 1; i < height - 1; i++)
    // {
    //     c.Y = point.getAbsY(i);
    //     SetConsoleCursorPosition(outputHandle, c);
    //     wprintf(L"│");
    // }

    // c.X = point.getAbsX(width - 1);
    // for (int i = 1; i < height - 1; i++)
    // {
    //     c.Y = point.getAbsY(i);
    //     SetConsoleCursorPosition(outputHandle, c);
    //     wprintf(L"│");
    // }
}

void View::ComponentBase::setColorStyle(View::Color foreColor, View::Color backColor)
{
    this->nowForeColor = foreColor;
    this->nowBackColor = backColor;
    this->foreColor = foreColor;
    this->backColor = backColor;
    setChange();
}

void View::ComponentBase::unsetFocus()
{
    focus = false;
}

void View::ComponentBase::setFocus()
{
    focus = true;
}

void View::ComponentBase::drawBackground()
{
    WORD wd = int(defaultForeColor) + int(defaultBackColor) * 0x10; 
    DWORD charsWritten;
    const wchar_t *s = std::wstring(width, L' ').c_str();

    for (int i = 0; i < height; i++)
    {
        SetConsoleCursorPosition(outputHandle, point.getCOORD(0, i));
        // wprintf(s);
        WriteConsoleW(outputHandle, s, width, &charsWritten, NULL);
        // FillConsoleOutputCharacterW(outputHandle, L' ', width, point.getCOORD(0, i), &charsWritten);
        // FillConsoleOutputAttribute(outputHandle, wd, width, point.getCOORD(0, i), &charsWritten);
    }
}

/*-----------------Text-----------------*/


View::Point View::Text::drawLine(std::wstring text, Point point, bool& isFinish)
{
    SetConsoleCursorPosition(outputHandle, point.getCOORD());
    
    // 计算可显示的最大宽度
    size_t maxWidth = width;
    size_t outLineLen = 0;
    size_t line = 0;
    
    // 创建缓冲区存储待输出内容
    std::wstring buffer;
    buffer.reserve(maxWidth);
    
    for (size_t i = 0; i < text.size(); i++)
    {
        int charWidth = UnicodeToANSI(text.substr(i, 1)).length();
        
        if (outLineLen + charWidth > maxWidth)
        {
            // 输出当前缓冲区内容
            DWORD charsWritten;
            SetConsoleCursorPosition(outputHandle, point.getCOORD(0, line));
            WriteConsoleW(outputHandle, buffer.c_str(), (DWORD)buffer.length(), &charsWritten, NULL);
            
            buffer.clear();
            outLineLen = 0;
            line++;
            
            // 检查是否超出垂直边界
            if (point.getAbsY(line) > this->point.getAbsY(height - 1))
            {
                SetConsoleCursorPosition(outputHandle, this->point.getCOORD(width - 3, height - 1));
                WriteConsoleW(outputHandle, L"...", 3, &charsWritten, NULL);
                isFinish = false;
                return point.getAbsPoint(outLineLen, line);
            }
        }
        
        buffer += text[i];
        outLineLen += charWidth;
    }
    
    // 输出最后一行
    if (!buffer.empty()) {
        DWORD charsWritten;
        SetConsoleCursorPosition(outputHandle, point.getCOORD(0, line));
        WriteConsoleW(outputHandle, buffer.c_str(), (DWORD)buffer.length(), &charsWritten, NULL);
    }
    
    isFinish = true;
    return point.getAbsPoint(outLineLen, line);
}


void View::Text::beginDraw()
{
    SetConsoleTextAttribute(outputHandle, int(nowForeColor) + int(nowBackColor) * 0x10);
    // drawBackground();
    Point tmp(point);
    SetConsoleCursorPosition(outputHandle, point.getCOORD());
    std::vector<std::wstring> splits = split(text, L"\n");
    bool isFinish = false;
    for (auto& line : splits)
    {
        isFinish = false;
        endPoint = drawLine(line, tmp, isFinish);
        tmp.y = endPoint.y + 1;
        if (!isFinish) break;
    }
    SetConsoleTextAttribute(outputHandle, int(defaultForeColor) + int(defaultBackColor) * 0x10);
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

View::ComponentBase* View::Button::onMouseEvent(MouseEvent& e)
{
    if (!getShow())
        return NULL;
    if (isPointIn(e.point))
    {
        switch (e.type)
        {
        case MouseEventType::LCLICK:
        case MouseEventType::LDOUBLECLICK:
            isHover = false;
            if (!isOnclick)
            {
                isOnclick = true;
                text.setColorStyle(clickForeColor, clickBackColor);
                nowBackColor = clickBackColor;
                nowForeColor = clickForeColor;
                setChange();
            }
            mouseEventFun = NULL;
            break;
        default:
            if (!isHover)
            {
                text.setColorStyle(hoverForeColor, hoverBackColor);
                nowBackColor = hoverBackColor;
                nowForeColor = hoverForeColor;
                isHover = true;
                setChange();
            }
            if (isOnclick)
            {
                mouseEventFun = onClickFun;
                isOnclick = false;
            }
            else
                mouseEventFun = NULL;
        }
    }
    else
    {
        switch (e.type)
        {
        case MouseEventType::LCLICK:
        case MouseEventType::LDOUBLECLICK:
            break;
        default:
            if (isHover || isOnclick)
            {
                text.setColorStyle(foreColor, backColor);
                nowBackColor = backColor;
                nowForeColor = foreColor;
                isHover = false;
                isOnclick = false;
                setChange();
            }
        }
        mouseEventFun = NULL;
    }
    return ComponentBase::onMouseEvent(e);
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
    SetConsoleTextAttribute(outputHandle, int(nowForeColor) + int(nowBackColor) * 0x10);
    drawBackground();
    drawBorder();
    text.setPoint(point.getAbsPoint(textAbX, textAbY));
    text.setSize(width - textSubWidth, height - textSubHeight);
    text.setColorStyle(nowForeColor, nowBackColor);
    text.draw();
    SetConsoleTextAttribute(outputHandle, int(defaultForeColor) + int(defaultBackColor) * 0x10);
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

View::List::List(HANDLE outputHandle, std::vector<std::wstring> items, size_t abX, size_t abY, size_t width, size_t itemHeight, Color defaultForeColor, Color defaultBackColor)
    : List(outputHandle, items, Point(abX, abY), width, itemHeight) {}

View::List::List(HANDLE outputHandle, std::vector<std::wstring> items, Point point, size_t width, size_t itemHeight, Color defaultForeColor, Color defaultBackColor)
    : ComponentBase(outputHandle, point, width, itemHeight, defaultForeColor, defaultBackColor)
{
    height = items.size() * itemHeight;
    COOD_INT y = 0;
    for (const auto& i : items)
    {
        buttons.emplace_back(std::make_unique<View::Button>(outputHandle, i, point.getAbsPoint(0, y), width, itemHeight));
        y += (itemHeight-1);
    }
}

void View::List::setPoint(Point point)
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
    COORD c = point.getCOORD();
    DWORD charsWritten;
    Point p;
    for (auto& button : buttons){
        button->draw();
    }
    for (size_t i = 0; i < buttons.size() - 1; i++)
    {
        p = buttons[i]->getRDPoint();
        c.X = p.x, c.Y = p.y;
        SetConsoleCursorPosition(outputHandle, c);
        WriteConsoleW(outputHandle, L"│", 1, &charsWritten, NULL);
        c.X = point.x, c.Y = p.y;
        SetConsoleCursorPosition(outputHandle, c);
        WriteConsoleW(outputHandle, L"│", 1, &charsWritten, NULL);
    }
}

View::List::~List()
{
    ComponentBase::~ComponentBase();
}

/*-----------------Line-----------------*/

void View::Line::beginDraw()
{
    SetConsoleTextAttribute(outputHandle, int(nowForeColor) + int(nowBackColor) * 0x10);
    if (isHorizontal)
    {
        SetConsoleCursorPosition(outputHandle, point.getCOORD());
        for (size_t i = 0; i < length; i++)
        {
            std::wcout << L"─";
        }
    }
    else
        for (size_t i = 0; i < length; i++)
        {
            SetConsoleCursorPosition(outputHandle, point.getCOORD(0, i));
            std::wcout << L"│";
        }
    SetConsoleTextAttribute(outputHandle, int(defaultForeColor) + int(defaultBackColor) * 0x10);
}

/*-----------------InputText-----------------*/

// 将文本按组件宽度拆分为多行
std::vector<std::wstring> View::InputText::calculateTextLines(const std::wstring& text) {
    std::vector<std::wstring> textLines;
    std::wstring currentLine;
    int lineWidth = 0;
    
    for (size_t i = 0; i < text.length(); i++) {
        int charWidth = UnicodeToANSI(text.substr(i, 1)).length();
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

// 显示文本内容
void View::InputText::renderText(const std::vector<std::wstring>& textLines) {
    clearDraw();
    for (size_t i = 0; i < textLines.size(); i++) {
        SetConsoleCursorPosition(outputHandle, point.getCOORD(0, i));
        std::wcout << textLines[i];
    }
}

// 根据鼠标点击位置计算光标位置
int View::InputText::calculateCursorPosition(const std::vector<std::wstring>& textLines, int clickX, int clickY, int& currentLineIdx, int& accumulatedWidth) {
    int cursorPos = 0;
    
    // 先定位到正确的行
    for (int i = 0; i < clickY && i < textLines.size(); i++) {
        cursorPos += textLines[i].length();
    }
    
    // 如果点击的行超出了文本行数，光标放在最后一个位置
    currentLineIdx = clickY < textLines.size() ? clickY : (textLines.size() > 0 ? textLines.size() - 1 : 0);
    std::wstring clickedLine = currentLineIdx < textLines.size() ? textLines[currentLineIdx] : (textLines.size() > 0 ? textLines.back() : L"");
    
    // 在当前行中定位光标
    accumulatedWidth = 0;
    int linePos = 0;
    for (size_t i = 0; i < clickedLine.length(); i++) {
        int charWidth = UnicodeToANSI(clickedLine.substr(i, 1)).length();
        if (accumulatedWidth + charWidth/2 >= clickX) {
            break;
        }
        accumulatedWidth += charWidth;
        linePos++;
    }
    
    // 计算光标在全文中的位置
    cursorPos = 0;
    for (int i = 0; i < currentLineIdx && i < textLines.size(); i++) {
        cursorPos += textLines[i].length();
    }
    cursorPos += linePos;
    
    return cursorPos;
}

// 显示光标并定位
void View::InputText::showAndPositionCursor(int accumulatedWidth, int currentLineIdx) {
    CONSOLE_CURSOR_INFO CursorInfo;
    GetConsoleCursorInfo(outputHandle, &CursorInfo);
    CursorInfo.bVisible = true;
    SetConsoleCursorInfo(outputHandle, &CursorInfo);
    SetConsoleCursorPosition(outputHandle, point.getCOORD(accumulatedWidth, currentLineIdx));
}

// 处理插入字符
bool View::InputText::handleCharacterInput(KEY_EVENT_RECORD ker, std::wstring& newText, int& cursorPos, 
                                         std::vector<std::wstring>& textLines, int& currentLineIdx, int& linePos) {
    // 检查插入后是否会超过垂直限制
    std::wstring testText = newText;
    testText.insert(cursorPos, 1, ker.uChar.UnicodeChar);
    
    // 计算插入后的行数
    std::vector<std::wstring> testLines = calculateTextLines(testText);
    
    // 如果行数超过高度限制，拒绝输入
    if (testLines.size() > height) {
        return false;
    }
    
    newText.insert(cursorPos, 1, ker.uChar.UnicodeChar);
    cursorPos++;
    
    // 重新使用测试过的行数据
    textLines = testLines;
    
    // 重新绘制文本
    renderText(textLines);
    
    // 更新当前行和光标位置
    updateLineAndPosition(textLines, cursorPos, currentLineIdx, linePos);
    
    return true;
}

// 处理删除字符
void View::InputText::handleDelete(std::wstring& newText, int& cursorPos, std::vector<std::wstring>& textLines, 
                                 int& currentLineIdx, int& linePos, bool isBackspace) {
    if (isBackspace) {
        cursorPos--;
    }
    
    newText.erase(cursorPos, 1);
    
    // 重新计算文本行
    textLines = calculateTextLines(newText);
    
    // 重新绘制文本
    renderText(textLines);
    
    // 更新当前行和光标位置
    updateLineAndPosition(textLines, cursorPos, currentLineIdx, linePos);
}

// 更新当前行和光标在行中的位置
void View::InputText::updateLineAndPosition(const std::vector<std::wstring>& textLines, int cursorPos, 
                                          int& currentLineIdx, int& linePos) {
    currentLineIdx = 0;
    int tempPos = 0;
    for (int i = 0; i < textLines.size(); i++) {
        if (tempPos + textLines[i].length() >= cursorPos) {
            currentLineIdx = i;
            linePos = cursorPos - tempPos;
            break;
        }
        tempPos += textLines[i].length();
    }
}

// 计算光标显示的位置
int View::InputText::calculateCursorDisplayPosition(const std::vector<std::wstring>& textLines, int currentLineIdx, int linePos) {
    if (currentLineIdx >= textLines.size() || linePos > textLines[currentLineIdx].length()) {
        return 0;
    }
    
    int accumulatedWidth = 0;
    for (int i = 0; i < linePos; i++) {
        accumulatedWidth += UnicodeToANSI(textLines[currentLineIdx].substr(i, 1)).length();
    }
    
    return accumulatedWidth;
}

// 处理键盘输入事件
bool View::InputText::handleKeyEvent(KEY_EVENT_RECORD ker, std::wstring& newText, std::wstring& currentText, 
                                   int& cursorPos, std::vector<std::wstring>& textLines, 
                                   int& currentLineIdx, int& linePos, bool& editing) {
    // Enter键 - 结束编辑
    if (ker.wVirtualKeyCode == VK_RETURN){
        editing = false;
        return true;
    }
    // Escape键 - 取消编辑
    else if (ker.wVirtualKeyCode == VK_ESCAPE) {
        newText = currentText;
        editing = false;
        return true;
    }
    // 左箭头 - 光标左移
    else if (ker.wVirtualKeyCode == VK_LEFT && cursorPos > 0) {
        cursorPos--;
        updateLineAndPosition(textLines, cursorPos, currentLineIdx, linePos);
        int accumulatedWidth = calculateCursorDisplayPosition(textLines, currentLineIdx, linePos);
        showAndPositionCursor(accumulatedWidth, currentLineIdx);
        return true;
    }
    // 右箭头 - 光标右移
    else if (ker.wVirtualKeyCode == VK_RIGHT && cursorPos < newText.length()) {
        cursorPos++;
        updateLineAndPosition(textLines, cursorPos, currentLineIdx, linePos);
        int accumulatedWidth = calculateCursorDisplayPosition(textLines, currentLineIdx, linePos);
        showAndPositionCursor(accumulatedWidth, currentLineIdx);
        return true;
    }
    // Backspace键 - 删除光标前的字符
    else if (ker.wVirtualKeyCode == VK_BACK && cursorPos > 0) {
        handleDelete(newText, cursorPos, textLines, currentLineIdx, linePos, true);
        int accumulatedWidth = calculateCursorDisplayPosition(textLines, currentLineIdx, linePos);
        showAndPositionCursor(accumulatedWidth, currentLineIdx);
        return true;
    }
    // Delete键 - 删除光标后的字符
    else if (ker.wVirtualKeyCode == VK_DELETE && cursorPos < newText.length()) {
        handleDelete(newText, cursorPos, textLines, currentLineIdx, linePos, false);
        int accumulatedWidth = calculateCursorDisplayPosition(textLines, currentLineIdx, linePos);
        showAndPositionCursor(accumulatedWidth, currentLineIdx);
        return true;
    }
    // 普通字符 - 在光标位置插入
    else if (ker.uChar.UnicodeChar != 0 && ker.uChar.UnicodeChar != VK_RETURN) {
        if (handleCharacterInput(ker, newText, cursorPos, textLines, currentLineIdx, linePos)) {
            int accumulatedWidth = calculateCursorDisplayPosition(textLines, currentLineIdx, linePos);
            showAndPositionCursor(accumulatedWidth, currentLineIdx);
        }
        return true;
    }
    
    return false;
}

// 主函数
View::ComponentBase* View::InputText::onMouseEvent(MouseEvent& e)
{
    SetConsoleTextAttribute(outputHandle, int(defaultForeColor) + int(defaultBackColor) * 0x10);
    mouseEventFun = NULL;
    if (isPointIn(e.point))
    {
        if (e.type != MouseEventType::HOVER)
        {
            // 获取当前文本（如果显示的是占位符，则使用空字符串）
            std::wstring currentText = isPlaceholderVisible ? L"" : getText();
            
            // 计算并显示文本行
            std::vector<std::wstring> textLines = calculateTextLines(currentText);
            renderText(textLines);
            
            // 计算点击位置对应的文本索引
            int clickX = e.point.x - point.x;  // 相对于控件左上角的X坐标
            int clickY = e.point.y - point.y;  // 相对于控件左上角的Y坐标
            
            int currentLineIdx = 0;
            int accumulatedWidth = 0;
            int cursorPos = calculateCursorPosition(textLines, clickX, clickY, currentLineIdx, accumulatedWidth);
            int linePos = cursorPos - (currentLineIdx > 0 ? (cursorPos - accumulatedWidth) : 0);
            
            // 显示并定位光标
            showAndPositionCursor(accumulatedWidth, currentLineIdx);
            
            // 进入编辑模式
            INPUT_RECORD inputRecord;
            DWORD numEventsRead;
            std::wstring newText = currentText;
            bool editing = true;
            
            while (editing)
            {
                // 使用PeekConsoleInput而不是ReadConsoleInput，这样可以非阻塞检查输入
                DWORD numEvents = 0;
                PeekConsoleInputW(GetStdHandle(STD_INPUT_HANDLE), &inputRecord, 1, &numEvents);
                
                if (numEvents > 0) {
                    ReadConsoleInputW(GetStdHandle(STD_INPUT_HANDLE), &inputRecord, 1, &numEventsRead);
                    
                    // 处理鼠标事件
                    if (inputRecord.EventType == MOUSE_EVENT && 
                        inputRecord.Event.MouseEvent.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED) {
                        COORD mousePos = inputRecord.Event.MouseEvent.dwMousePosition;
                        Point clickPoint(mousePos.X, mousePos.Y);
                        
                        if (!isPointIn(clickPoint)) {
                            // 点击在组件外部，结束编辑
                            editing = false;
                        } else {
                            // 点击在组件内部，重新定位光标
                            int newClickX = clickPoint.x - point.x;
                            int newClickY = clickPoint.y - point.y;
                            
                            // 重新计算光标位置
                            cursorPos = calculateCursorPosition(textLines, newClickX, newClickY, currentLineIdx, accumulatedWidth);
                            linePos = cursorPos - (currentLineIdx > 0 ? (cursorPos - accumulatedWidth) : 0);
                            
                            // 重新定位光标
                            showAndPositionCursor(accumulatedWidth, currentLineIdx);
                        }
                    }
                    
                    // 处理键盘事件
                    else if (inputRecord.EventType == KEY_EVENT && inputRecord.Event.KeyEvent.bKeyDown) {
                        handleKeyEvent(inputRecord.Event.KeyEvent, newText, currentText, 
                                     cursorPos, textLines, currentLineIdx, linePos, editing);
                    }
                }
            }
            
            // 处理编辑后的文本
            if (!newText.empty()) {
                setText(newText);
                isPlaceholderVisible = false;
            } else {
                setText(placeholder);
                isPlaceholderVisible = true;
            }
            
            // 如果文本已更改，调用回调
            if (textChanged != NULL && newText != currentText)
                textChanged(this);
            
            // 隐藏光标
            CONSOLE_CURSOR_INFO CursorInfo;
            GetConsoleCursorInfo(outputHandle, &CursorInfo);
            CursorInfo.bVisible = false;
            SetConsoleCursorInfo(outputHandle, &CursorInfo);
            
            mouseEventFun = focusFun;
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
        // update();
    }
    else
    {
        ComponentBase::setShow(show);
        //update();
    }
    //ComponentBase::setShow(show);
}

// void View::LayoutBase::registerComponents(
//     std::vector<ComponentBase*>& allComponents,
//     std::vector<ComponentBase*>& mouseComponents,
//     std::vector<ComponentBase*>& keyComponents
// )
// {
//     for (auto com : allComponents)
//         com->setPoint(point.getAbsPoint(com->getPoint()));
//     this->allComponents = allComponents;
//     this->mouseComponents = mouseComponents;
//     this->keyComponents = keyComponents;
// }

bool View::LayoutBase::addComponent(std::shared_ptr<ComponentBase> com)
{
    if (!com) return false;
    ComponentBase* comPtr = com.get();
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
        allComponents.erase(iter);
        return true;
    }
    return false;
}

void View::LayoutBase::beginDraw()
{
    for (auto& i : allComponents)
    {
        i->draw();
    }
}

void View::LayoutBase::update()
{
    if (isChange)
    {
        if (getShow())
        {
            draw();
        }
        else
            clearDraw();
        isChange = false;
    }
    else
        for (auto& i : allComponents)
        {
            i->update();
        }
}

void View::LayoutBase::setPoint(Point point)
{
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

View::LayoutBase::~LayoutBase()
{
}

View::ComponentBase* View::LayoutBase::onMouseEvent(MouseEvent& e)
{
    if (!getShow())
        return NULL;
    ComponentBase* p = NULL;
    for (auto& i : allComponents) {
        ComponentBase* tmp = i->onMouseEvent(e);
        if (tmp != NULL) {
            p = tmp;
        }
    }
    return p;
}

View::ComponentBase* View::LayoutBase::onKeyEvent(KeyEvent& e)
{
    if (!getShow())
        return NULL;

    ComponentBase* p = NULL;
    for (auto& i : allComponents) {
        ComponentBase* tmp = i->onKeyEvent(e);
        if (tmp != NULL) {
            p = tmp;  // 保存非NULL的返回值
        }
    }
    return p;  // 返回最后一个响应的组件
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
    ComponentBase* comPtr = com.get();
    if (LayoutBase::addComponent(std::move(com)))
    {
        comPtr->setPoint(point.getAbsPoint(currentWidth, padding[0]));
        currentWidth += (comPtr->getWidth() + space);
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

// void View::Span::registerComponents(
//     std::vector<ComponentBase*>& allComponents,
//     std::vector<ComponentBase*>& mouseComponents,
//     std::vector<ComponentBase*>& keyComponents
// )
// {
//     this->mouseComponents = mouseComponents;
//     this->keyComponents = keyComponents;
//     for (auto i : allComponents)
//     {
//         i->setShow(false);
//         if (!addComponent(i))
//             LayoutBase::removeComponent(i);
//         else
//             i->setShow(true);
//     }
// }

/*-----------------Div-----------------*/

bool View::Div::addComponent(std::shared_ptr<ComponentBase> com)
{
    if (!com || currentHeight + com->getHeight() + space > height)
        return false;
    ComponentBase* comPtr = com.get();
    if (LayoutBase::addComponent(std::move(com)))
    {
        comPtr->setPoint(point.getAbsPoint(padding[3], currentHeight));
        currentHeight += (comPtr->getHeight() + space);
        return true;
    }
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
    this->space = space;
    currentHeight = padding[0];
    for (auto& com : allComponents)
    {
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
