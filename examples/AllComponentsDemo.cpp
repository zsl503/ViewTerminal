#include "../include/viewterminal/pch.h"

class WelcomePage : public View::PageBase {
public:
    WelcomePage() {}

    void initComponents() override {
        // Title Component
        View::Text* title = new View::Text(outputHandle, L"🌟 ViewTerminal Demo 🌟", 5, 2, 50, 3, View::Color::YELLOW);

        // Description Component
        std::wstring description = L"🚀 ViewTerminal 是一个基于 C++ 的终端 UI 库，\n"
                                    L"✨ 提供丰富的组件如按钮、文本、布局等，\n"
                                    L"🔧 使用现代 C++ 技术构建，支持灵活的扩展。";
        View::Text* desc = new View::Text(outputHandle, description, 5, 6, 50, 5, View::Color::WHITE);

        // Button to navigate to demo page
        View::Button* demoButton = new View::Button(outputHandle, L"进入 Demo 页面", 5, 12, 20, 3, View::Color::WHITE, View::Color::BLUE);
        demoButton->onClickFun = [this](View::MouseEvent& e, View::ComponentBase* com) -> int {
            app->navigateTo(L"allComponents");
            return 1;
        };

        // Register components
        addComponent(title);
        addComponent(desc);
        addComponent(demoButton);
    }
};

class AllComponentsPage : public View::PageBase {
public:
    AllComponentsPage() {}

    void initComponents() override {
        // Button Component
        View::Button* button = new View::Button(outputHandle, L"返回首页", {0, 0}, 15, 3, View::Color::WHITE, View::Color::BLUE);
        button->onClickFun = [this](View::MouseEvent& e, View::ComponentBase* com) -> int {
            app->navigateBack();
            return 1;
        };

        // InputText Component
        View::InputText* inputText = new View::InputText(outputHandle, L"Enter Text", {5, 0}, 30, 3, View::Color::BRIGHTWHITE);
        // List Component
        std::vector<std::wstring> items = {L"Item 1", L"Item 2", L"Item 3"};
        View::List* list = new View::List(outputHandle, items, {40, 0}, 30, 3);

        // Line Component
        View::Line* line = new View::Line(outputHandle, 5, 15, true, 30, View::Color::WHITE);

        // Span Layout
        View::Span* span = new View::Span(outputHandle, {0, 0}, 40, 5, 2);
        span->addComponent(new View::Text(outputHandle, L"Span Child 1", {0,0}));
        span->addComponent(new View::Text(outputHandle, L"Span Child 2", {0,0}));

        // Div Layout
        View::Div* div1 = new View::Div(outputHandle, {0, 0}, 40, 5);
        div1->addComponent(new View::Text(outputHandle, L"Div Child  1", {0,0}));
        div1->addComponent(new View::Text(outputHandle, L"Div Child  2", {0,0}));

        // Div Layout
        View::Div* div0 = new View::Div(outputHandle, {0, 0}, 160, 80);
        div0->addComponent(new View::Text(outputHandle, L"This is a Text Component", {0,0}, 30));
        // div0->addComponent(line);
        div0->addComponent(inputText);
        // div0->addComponent(line);
        // div0->addComponent(list);
        div0->addComponent(line);
        div0->addComponent(span);
        div0->addComponent(div1);
        div0->addComponent(button);

        addComponent(list);
        addComponent(div0);
    }
};

int main() {
    // Create application instance
    View::App app(200, 120);
    std::map<std::wstring, View::PageBase*> route = {
        {L"welcome", new WelcomePage()},
        {L"allComponents", new AllComponentsPage()}
    };

    // Register pages
    app.registerPages(route);

    // Navigate to the WelcomePage
    app.navigateTo(L"welcome");

    return 0;
}