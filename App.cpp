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
    switch (type)
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
    }
    case NAB:
    {
        if (pageStack.size() > 1)
        {
            pageStack.pop();
        }
        pageStack.top()->create();
        processDestroyed();
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
    SetConsoleTitle(name.c_str());
    this->app = app;
    this->outputHandle = outputHandle;
    this->inputHandle = inputHandle;
    initComponents();
}

View::PageBase::~PageBase()
{
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
    clear();
    isListen = false;
}

void View::PageBase::clear()
{
    //system("cls");
    COORD c = { 0,0 };
    SetConsoleCursorPosition(outputHandle, c);
    for (size_t i = 0; i < app->getHeight(); i++)
    {
        c.Y = i;
        SetConsoleCursorPosition(outputHandle, c);
        for (size_t j = 0; j < app->getWidth(); j++)
        {
            std::wcout << " ";
        }
    }
}

void View::PageBase::eventListener()
{
    INPUT_RECORD	record;
    DWORD			res;

    View::MouseEvent mouE;
    View::KeyEvent keyE;
    ComponentBase* p = NULL, * tmp = NULL;
    while (isListen)
    {
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
            for (auto i : mouseComponents)
                p = (i->onMouseEvent(mouE)) == NULL ? p : tmp;
        }
        else if (isListen)
        {
            for (auto i : keyComponents)
                p = (i->onKeyEvent(keyE)) == NULL ? p : tmp;
        }
        setFocus(p);
        if (isListen)
            for (auto i : allComponents)
                i->update();
    }

}