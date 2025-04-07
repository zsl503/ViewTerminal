#include "pch.h"

using namespace View;
App::App(int width, int height) :width(width), height(height)
{
    outputHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    inputHandle = GetStdHandle(STD_INPUT_HANDLE);
    setlocale(LC_CTYPE, "");
    std::wstring t;
    DWORD mode = ENABLE_INSERT_MODE | ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT | ENABLE_MOUSE_INPUT | ENABLE_PROCESSED_INPUT;
    SetConsoleMode(inputHandle, mode);

    CONSOLE_CURSOR_INFO CursorInfo;
    GetConsoleCursorInfo(outputHandle, &CursorInfo);
    CursorInfo.bVisible = false; //隐藏控制台光标
    SetConsoleCursorInfo(outputHandle, &CursorInfo);//设置控制台光标状态
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

int View::App::getWidth()
{
    return width;
}

int View::App::getHeight()
{
    return height;
}

void View::App::navigateBack()
{
    type = Type::NAB;
    pageStack.top()->destroy();
}

void App::registerPages(std::map<std::wstring, View::PageBase*>& route)
{
    this->route = route;
}

View::PageBase* App::openPage(std::wstring name)
{
    std::map<std::wstring, View::PageBase*>::iterator iter = route.find(name);
    if (iter != route.end()) {
        pageStack.push(route[name]);
        route[name]->initPage(this, name, outputHandle, inputHandle);
        return route[name];
    }
    else {
        throw;
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
    App* app, std::wstring name, HANDLE outputHandle, HANDLE inputHandle
)
{
    this->name = name;
    this->app = app;
    this->outputHandle = outputHandle;
    this->inputHandle = inputHandle;
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

void View::PageBase::addComponent(ComponentBase* com)
{
    allComponents.insert(com);
    mouseComponents.insert(com);
    keyComponents.insert(com);
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
    for (auto i : allComponents)
    {
        i->draw();
    }
    mounted();
    isListen = true;
    eventListener();
}

void View::PageBase::setFocus(View::ComponentBase* com)
{
    if (com != NULL)
        com->setFocus();
}

void View::PageBase::update()
{
    clear();
    create();
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
    // 使用更高效的控制台清屏方法
    HANDLE hConsole = outputHandle;
    COORD coordScreen = { 0, 0 };
    DWORD charsWritten;
    WORD wd = FOREGROUND_INTENSITY; 
    // 填充指定宽度和高度范围内的矩阵为空格
    for (int y = 0; y < app->getHeight(); ++y) {
        COORD coord = { 0, static_cast<SHORT>(y) };
        FillConsoleOutputAttribute(hConsole, wd, app->getWidth(), coord, &charsWritten);
        FillConsoleOutputCharacterW(hConsole, L' ', app->getWidth(), coord, &charsWritten);
    }

    // 重置光标位置
    SetConsoleCursorPosition(hConsole, coordScreen);
}

// void View::PageBase::clear()
// {
//     // 使用更高效的控制台清屏方法
//     HANDLE hConsole = outputHandle;
//     COORD coordScreen = { 0, 0 };
//     DWORD cCharsWritten;
//     CONSOLE_SCREEN_BUFFER_INFO csbi;
    
//     // 获取控制台信息
//     GetConsoleScreenBufferInfo(hConsole, &csbi);
//     DWORD dwConSize = csbi.dwSize.X * csbi.dwSize.Y;
    
//     // 填充整个屏幕为空格
//     FillConsoleOutputCharacter(hConsole, ' ', dwConSize, coordScreen, &cCharsWritten);
    
//     // 恢复默认属性
//     FillConsoleOutputAttribute(hConsole, csbi.wAttributes, dwConSize, coordScreen, &cCharsWritten);
    
//     // 重置光标位置
//     SetConsoleCursorPosition(hConsole, coordScreen);
// }

void View::PageBase::eventListener()
{
    INPUT_RECORD record;
    DWORD res;
    View::MouseEvent mouE;
    View::KeyEvent keyE;
    ComponentBase* p = NULL;
    
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
                    
                    p = NULL;
                    for (auto i : mouseComponents) {
                        ComponentBase* result = i->onMouseEvent(mouE);
                        if (result != NULL) {
                            p = result;
                        }
                    }
                }
                
                setFocus(p);
            }
        }
        else {
            // 没有事件时休眠一小段时间，减少CPU占用
            Sleep(1);
        }
        
        // 更新所有组件，但限制更新频率
        static DWORD lastUpdateTime = GetTickCount();
        DWORD currentTime = GetTickCount();
        
        if (isListen && (currentTime - lastUpdateTime >= 16)) { // 约60fps
            for (auto i : allComponents) {
                i->update();
            }
            lastUpdateTime = currentTime;
        }
    }
}