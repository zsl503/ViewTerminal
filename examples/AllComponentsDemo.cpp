#include "../include/viewterminal/pch.h"
#include <memory> // 包含 <memory>

class WelcomePage : public View::PageBase {
public:
    WelcomePage() {}

    void initComponents() override {
        // Title Component
        auto title = new View::Text(outputHandle, L"🌟 ViewTerminal Demo 🌟", 5, 2, 50, 3, View::Color::YELLOW);

        // Description Component
        std::wstring description = L"🚀 ViewTerminal 是一个基于 C++ 的终端 UI 库，\n"
                                    L"✨ 提供丰富的组件如按钮、文本、布局等，\n"
                                    L"🔧 使用现代 C++ 技术构建，支持灵活的扩展。";
        auto desc = new View::Text(outputHandle, description, 5, 6, 50, 5, View::Color::WHITE);

        // Button to navigate to demo page
        auto demoButton = new View::Button(outputHandle, L"进入 Demo 页面", 5, 12, 20, 3, View::Color::WHITE, View::Color::BLUE);
        
        // 使用新的事件系统
        demoButton->addEventListener(View::EventType::CLICK, [this](View::EventArgs* args) {
            app->navigateTo(L"allComponents");
        });

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
        auto button = std::make_shared<View::Button>(outputHandle, L"返回首页", View::Point{0, 0}, 15, 3, View::Color::WHITE, View::Color::BLUE);
        
        // 使用新的事件系统
        button->addEventListener(View::EventType::CLICK, [this](View::EventArgs* args) {
            app->navigateBack();
        });

        // InputText Component
        auto inputText = std::make_shared<View::InputText>(outputHandle, L"Enter Text", View::Point{5, 0}, 30, 3, View::Color::BRIGHTWHITE);
        
        // 使用新的事件系统监听值变化
        inputText->addEventListener(View::EventType::VALUE_CHANGE, [](View::EventArgs* args) {
            auto valueArgs = static_cast<View::ValueChangedEventArgs*>(args);
            auto oldText = std::any_cast<std::wstring>(valueArgs->getOldValue());
            auto newText = std::any_cast<std::wstring>(valueArgs->getNewValue());
            // 处理文本变化
        });

        // List Component
        std::vector<std::wstring> items = {L"Item 1", L"Item 2", L"Item 3"};
        auto list = new View::List(outputHandle, items, View::Point{40, 0}, 30, 3);

        // Line Component
        auto line = std::make_shared<View::Line>(outputHandle, 5, 15, true, 30, View::Color::WHITE);

        // Span Layout
        auto span = std::make_shared<View::Span>(outputHandle, View::Point{0, 0}, 40, 5, 2);
        span->addComponent(std::make_shared<View::Text>(outputHandle, L"Span Child 1", View::Point{0,0}));
        span->addComponent(std::make_shared<View::Text>(outputHandle, L"Span Child 2", View::Point{0,0}));

        // Div Layout
        auto div1 = std::make_shared<View::Div>(outputHandle, View::Point{0, 0}, 40, 10);
        div1->addComponent(std::make_shared<View::Text>(outputHandle, L"Div Child  1", View::Point{0,0}));
        div1->addComponent(std::make_shared<View::Text>(outputHandle, L"Div Child  2", View::Point{0,0}));

        // Div Layout
        auto div0 = new View::Div(outputHandle, View::Point{0, 0}, 160, 80);
        div0->addComponent(std::make_shared<View::Text>(outputHandle, L"This is a Text Component", View::Point{0,0}, 30));
        div0->addComponent(inputText);
        div0->addComponent(line);
        div0->addComponent(span);
        div0->addComponent(div1);
        div0->addComponent(button);

        // 将 list 和 div0 添加到页面
        addComponent(list);
        addComponent(div0);
    }
};

int main() {
    // Create application instance
    View::App app(200, 120);
    // 使用 unique_ptr 管理页面生命周期
    std::map<std::wstring, std::unique_ptr<View::PageBase>> route;
    route[L"welcome"] = std::make_unique<WelcomePage>();
    route[L"allComponents"] = std::make_unique<AllComponentsPage>();

    // Register pages
    app.registerPages(route);

    // Navigate to the WelcomePage
    app.navigateTo(L"welcome");

    return 0;
}