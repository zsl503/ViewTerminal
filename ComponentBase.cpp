#include "pch.h"
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
View::Point View::Text::drawLine(std::wstring text, Point point, bool& isFinish)
{
    SetConsoleCursorPosition(outputHandle, point.getCOORD());
    size_t outLineLen = 0, line = 0;
    for (int i = 0; i < text.size(); i++)
    {
        int len = UnicodeToANSI(text.substr(i, 1)).length();
        if (width - outLineLen < len)
        {
            outLineLen = 0;
            line++;
            SetConsoleCursorPosition(outputHandle, point.getCOORD(0, line));
        }
        if (point.getAbsY(line) > this->point.getAbsY(height - 1))
        {
            SetConsoleCursorPosition(outputHandle, this->point.getCOORD(width - 3, height - 1));
            std::wcout << L"...";
            return point.getAbsPoint(outLineLen, line);
        }
        std::wcout << text[i];
        outLineLen += len;
    }
    isFinish = true;
    return point.getAbsPoint(outLineLen, line);
}

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

View::Point View::Point::getAbsPoint(int  reX, int  reY)
{
    return Point(reX + x, reY + y);
}

View::Point View::Point::getAbsPoint(Point rePoint)
{
    return Point(rePoint.x + x, rePoint.y + y);
}

COORD View::Point::getCOORD(int  reX, int  reY)
{
    COORD c = { getAbsX(reX), getAbsY(reY) };
    return c;
}

unsigned short View::Point::getAbsX(int  reX)
{
    return reX + x;
}

unsigned short View::Point::getAbsY(int  reY)
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
    if (!isDrawed)return;
    SetConsoleTextAttribute(outputHandle, int(defaultForeColor) + int(defaultBackColor) * 10);
    for (int i = 0; i < height; i++)
    {
        SetConsoleCursorPosition(outputHandle, point.getCOORD(0, i));
        for (int j = 0; j < width; j++)
        {
            std::wcout << L" ";
        }
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

    COORD c = point.getCOORD();
    SetConsoleCursorPosition(outputHandle, c);
    std::wcout << L"◢";
    for (int i = 1; i < width - 1; i++)
        std::wcout << L"-";
    std::wcout << L"◣";
    c.X = point.getAbsX(0), c.Y = point.getAbsY(height - 1);
    SetConsoleCursorPosition(outputHandle, c);
    std::wcout << L"◥";
    for (int i = 1; i < width - 1; i++)
        std::wcout << L"-";
    std::wcout << L"◤";

    c.X = point.getAbsX(0);
    for (int i = 1; i < height - 1; i++)
    {
        c.Y = point.getAbsY(i);
        SetConsoleCursorPosition(outputHandle, c);
        std::wcout << L"|";
    }

    c.X = point.getAbsX(width - 1);
    for (int i = 1; i < height - 1; i++)
    {
        c.Y = point.getAbsY(i);
        SetConsoleCursorPosition(outputHandle, c);
        std::wcout << L"|";
    }
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
    for (int i = 0; i < height; i++)
    {
        SetConsoleCursorPosition(outputHandle, point.getCOORD(0, i));
        for (int j = 0; j < width; j++)
        {
            std::wcout << L" ";
        }
    }
}

/*-----------------Text-----------------*/

void View::Text::beginDraw()
{
    SetConsoleTextAttribute(outputHandle, int(nowForeColor) + int(nowBackColor) * 0x10);
    Point tmp(point);
    SetConsoleCursorPosition(outputHandle, point.getCOORD());
    std::vector<std::wstring> splits = split(text, L"\n");
    bool isFinish = false;
    for (auto i : splits)
    {
        isFinish = false;
        endPoint = drawLine(i, tmp, isFinish);
        tmp.y = endPoint.y + 1;
        if (!isFinish) break;
    }
    SetConsoleTextAttribute(outputHandle, int(defaultForeColor) + int(defaultBackColor) * 10);
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
    text.draw();
    SetConsoleTextAttribute(outputHandle, int(defaultForeColor) + int(defaultBackColor) * 10);
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
    int y = 0;
    for (auto i : items)
    {
        buttons.push_back(new View::Button(outputHandle, i, point.x, y, width, itemHeight));
        y += (itemHeight - 1);
    }
}

void View::List::beginDraw()
{
    COORD c = point.getCOORD();
    Point p;
    for (auto button : buttons)
        button->draw();
    for (size_t i = 0; i < buttons.size() - 1; i++)
    {
        p = buttons[i]->getRDPoint();
        c.X = p.x, c.Y = p.y;
        SetConsoleCursorPosition(outputHandle, c);
        std::wcout << L"|";
        c.X = point.x, c.Y = p.y;
        SetConsoleCursorPosition(outputHandle, c);
        std::wcout << L"|";
    }
}

View::List::~List()
{
    for (auto i : buttons)
    {
        delete i;
        i = NULL;
    }
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
            std::wcout << L"-";
        }
    }
    else
        for (size_t i = 0; i < length; i++)
        {
            SetConsoleCursorPosition(outputHandle, point.getCOORD(0, i));
            std::wcout << L"|";
        }
    SetConsoleTextAttribute(outputHandle, int(defaultForeColor) + int(defaultBackColor) * 10);
}

/*-----------------InputText-----------------*/

View::ComponentBase* View::InputText::onMouseEvent(MouseEvent& e)
{
    mouseEventFun = NULL;
    if (isPointIn(e.point))
    {
        if (e.type != MouseEventType::HOVER)
        {
            std::wstring oldStr = setText(L"");
            update();
            SetConsoleCursorPosition(outputHandle, point.getCOORD());
            CONSOLE_CURSOR_INFO CursorInfo;
            GetConsoleCursorInfo(outputHandle, &CursorInfo);
            CursorInfo.bVisible = true;
            SetConsoleCursorInfo(outputHandle, &CursorInfo);//设置控制台光标状态

            std::wstring s;
            std::getline(std::wcin, s);
            if (s.empty())
                s = oldStr;
            std::wcin.clear();
            setText(s);
            if (textChanged != NULL && s != oldStr)
                textChanged(this);
            CursorInfo.bVisible = false;
            SetConsoleCursorInfo(outputHandle, &CursorInfo);//设置控制台光标状态
            mouseEventFun = focusFun;
            unsetFocus();
        }
    }
    else
    {
        //if (focus)
        //{
        //    unsetFocus();
        //    CONSOLE_CURSOR_INFO CursorInfo;
        //    GetConsoleCursorInfo(outputHandle, &CursorInfo);
        //    CursorInfo.bVisible = false; //隐藏控制台光标
        //    SetConsoleCursorInfo(outputHandle, &CursorInfo);//设置控制台光标状态
        //}
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
        //update();
    }
    else
    {
        ComponentBase::setShow(show);
        //update();
    }
    //ComponentBase::setShow(show);
}

void View::LayoutBase::registerComponents(
    std::vector<ComponentBase*>& allComponents,
    std::vector<ComponentBase*>& mouseComponents,
    std::vector<ComponentBase*>& keyComponents
)
{
    for (auto com : allComponents)
        com->setPoint(point.getAbsPoint(com->getPoint()));
    this->allComponents = allComponents;
    this->mouseComponents = mouseComponents;
    this->keyComponents = keyComponents;
}

bool View::LayoutBase::addComponent(ComponentBase* com)
{
    allComponents.push_back(com);
    com->setPoint(point.getAbsPoint(com->getPoint()));
    return true;
}

bool View::LayoutBase::removeComponent(ComponentBase* com)
{
    std::vector<ComponentBase*>::iterator iter = std::find(allComponents.begin(), allComponents.end(), com);
    if (iter != allComponents.end())
        allComponents.erase(iter);
    iter = std::find(mouseComponents.begin(), mouseComponents.end(), com);
    if (iter != mouseComponents.end())
        mouseComponents.erase(iter);
    iter = std::find(keyComponents.begin(), keyComponents.end(), com);
    if (iter != keyComponents.end())
        keyComponents.erase(iter);
    //if(com != NULL)
    delete com;
    com = NULL;
    return true;
}

void View::LayoutBase::beginDraw()
{
    for (auto i : allComponents)
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
            //clearDraw();
            draw();
        }
        else
            clearDraw();
        isChange = false;
    }
    else
        for (auto i : allComponents)
        {
            i->update();
        }
}

void View::LayoutBase::setPoint(Point point)
{
    int dx = point.x - this->point.x, dy = point.y - this->point.y;
    for (auto i : allComponents)
    {
        i->setPoint(i->getPoint().getAbsPoint(dx, dy));
    }
    ComponentBase::setPoint(point);

}


void View::LayoutBase::clear()
{
    for (auto i : allComponents)
    {
        delete i;
        i = NULL;
    }
    allComponents.clear();
    mouseComponents.clear();
    keyComponents.clear();
}

View::LayoutBase::~LayoutBase()
{
    clear();
}

View::ComponentBase* View::LayoutBase::onMouseEvent(MouseEvent& e)
{
    if (!getShow())
        return NULL;
    ComponentBase* p = NULL, * tmp = NULL;
    for (auto i : mouseComponents)
        p = (i->onMouseEvent(e)) == NULL ? p : tmp;
    return p;
}

View::ComponentBase* View::LayoutBase::onKeyEvent(KeyEvent& e)
{
    if (!getShow())
        return NULL;
    ComponentBase* p = NULL, * tmp = NULL;
    for (auto i : keyComponents)
        p = (i->onKeyEvent(e)) == NULL ? p : tmp;
    return p;
}


/*-----------------Span-----------------*/

void View::Span::setPadding(unsigned short top, unsigned short right, unsigned short bottom, unsigned short left)
{
    padding[0] = top, padding[1] = right, padding[2] = bottom, padding[3] = left;
    currentWidth = padding[3];
    for (auto com : allComponents)
    {
        com->setPoint(point.getAbsPoint(currentWidth, padding[0]));
        currentWidth += (com->getWidth() + space);
    }
}

bool View::Span::addComponent(ComponentBase* com)
{
    if (currentWidth + com->getWidth() + space > width)
        return false;
    LayoutBase::addComponent(com);
    com->setPoint(point.getAbsPoint(currentWidth, padding[0]));
    currentWidth += (com->getWidth() + space);
    return true;
}

void View::Span::clear()
{
    LayoutBase::clear();
    currentWidth = 0;
}

bool View::Span::removeComponent(ComponentBase* com)
{
    currentWidth -= (com->getWidth() + space);
    return LayoutBase::removeComponent(com);
}

void View::Span::setSpace(size_t space)
{
    currentWidth = padding[3];
    for (auto com : allComponents)
    {
        com->setPoint(point.getAbsPoint(currentWidth, padding[0]));
        currentWidth += (com->getWidth() + space);
    }
}

void View::Span::registerComponents(
    std::vector<ComponentBase*>& allComponents,
    std::vector<ComponentBase*>& mouseComponents,
    std::vector<ComponentBase*>& keyComponents
)
{
    this->mouseComponents = mouseComponents;
    this->keyComponents = keyComponents;
    for (auto i : allComponents)
    {
        i->setShow(false);
        if (!addComponent(i))
            LayoutBase::removeComponent(i);
        else
            i->setShow(true);
    }
}

/*-----------------Div-----------------*/

bool View::Div::addComponent(ComponentBase* com)
{
    if (currentHeight + com->getHeight() + space > height)
        return false;
    LayoutBase::addComponent(com);
    com->setPoint(point.getAbsPoint(padding[3], currentHeight));
    currentHeight += (com->getHeight() + space);
    return true;
}

bool View::Div::removeComponent(ComponentBase* com)
{
    currentHeight -= (com->getHeight() + space);
    return LayoutBase::removeComponent(com);
}

void View::Div::setSpace(size_t space)
{
    currentHeight = padding[0];
    for (auto com : allComponents)
    {
        com->setPoint(point.getAbsPoint(padding[3], currentHeight));
        currentHeight += (com->getHeight() + space);
    }
}

void View::Div::clear()
{
    LayoutBase::clear();
    currentHeight = 0;
}

void View::Div::registerComponents(
    std::vector<ComponentBase*>& allComponents,
    std::vector<ComponentBase*>& mouseComponents,
    std::vector<ComponentBase*>& keyComponents
)
{
    this->mouseComponents = mouseComponents;
    this->keyComponents = keyComponents;
    for (auto i : allComponents)
    {
        if (!addComponent(i))
        {
            LayoutBase::removeComponent(i);
        }
    }
}

void View::Div::setPadding(unsigned short top, unsigned short right, unsigned short bottom, unsigned short left)
{
    padding[0] = top, padding[1] = right, padding[2] = bottom, padding[3] = left;
    currentHeight = padding[0];
    for (auto com : allComponents)
    {
        com->setPoint(point.getAbsPoint(padding[3], currentHeight));
        currentHeight += (com->getHeight() + space);
    }
}
