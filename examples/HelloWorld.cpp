#include "../include/viewterminal/pch.h"

// 第三个页面
class ThirdPage : public View::PageBase {
    public:
        ThirdPage() {}
        
        void initComponents() override {
            // 创建标题文本
            View::Text* title = new View::Text(outputHandle, L"这是第三个页面", 5, 2, 30, 3, View::Color::LIGHTGREEN);
            
            // 创建描述文本
            View::Text* description = new View::Text(outputHandle, L"你已经成功从上一页导航到这个页面！", 5, 6, 40, 3);
            
            // 创建返回按钮
            View::Button* backButton = new View::Button(outputHandle, L"返回上一页, 这是一个很长的测试", 5, 10, 20, 3, View::Color::WHITE, View::Color::BLUE);
            backButton->onClickFun = [this](View::MouseEvent& e, View::ComponentBase* com) -> int {
                app->navigateBack();
                return 1;
            };

            
            // 创建返回按钮
            View::Button* homeButton = new View::Button(outputHandle, L"返回主页", 5, 20, 20, 3, View::Color::WHITE, View::Color::BLUE);
            homeButton->onClickFun = [this](View::MouseEvent& e, View::ComponentBase* com) -> int {
                app->navigateTo(L"hello");
                return 1;
            };
            
            // 注册组件
            // addComponent(title);
            // addComponent(description);
            addComponent(backButton);
            addComponent(homeButton);
        }
    };
    

// 第二个页面
class SecondPage : public View::PageBase {
public:
    SecondPage() {}
    
    void initComponents() override {
        // 创建标题文本
        View::Text* title = new View::Text(outputHandle, L"这是第二个页面", 5, 2, 30, 3, View::Color::LIGHTGREEN);
        
        // 创建描述文本
        View::Text* description = new View::Text(outputHandle, L"你已经成功从主页导航到这个页面！", 5, 6, 40, 3);
        
        // 创建返回按钮
        View::Button* backButton = new View::Button(outputHandle, L"返回上一页", 5, 10, 20, 3, View::Color::WHITE, View::Color::BLUE);
        backButton->onClickFun = [this](View::MouseEvent& e, View::ComponentBase* com) -> int {
            app->navigateBack();
            return 1;
        };

        
        // 创建返回按钮
        View::Button* nextButton = new View::Button(outputHandle, L"到达第三页", 5, 20, 20, 3, View::Color::WHITE, View::Color::BLUE);
        nextButton->onClickFun = [this](View::MouseEvent& e, View::ComponentBase* com) -> int {
            app->navigateTo(L"third");
            return 1;
        };
        
        // 注册组件
        // addComponent(title);
        // addComponent(description);
        addComponent(backButton);
        addComponent(nextButton);
    }
};

// 主页
class HelloWorldPage : public View::PageBase {
public:
    HelloWorldPage() {}
    
    void initComponents() override {
        // 创建标题文本
        View::Text* title = new View::Text(outputHandle, L"Hello World!", 10, 2, 40, 3, View::Color::LIGHTYELLOW);
        
        // 创建描述文本
        View::Text* description = new View::Text(outputHandle, L"这是使用ViewTerminal框架的简单示例", 10, 6, 40, 3);
        
        // 创建导航按钮
        View::Button* nextButton = new View::Button(outputHandle, L"前往第二页", 10, 10, 20, 3, View::Color::WHITE, View::Color::GREEN);
        nextButton->onClickFun = [this](View::MouseEvent& e, View::ComponentBase* com) -> int {
            app->navigateTo(L"second");
            return 1;
        };
        
        // 注册组件
        addComponent(title);
        addComponent(description);
        addComponent(nextButton);
    }
};

int main() {
    // 创建应用实例
    View::App app(80, 25);  // 宽度和高度参数在新版中不起作用
    std::map<std::wstring, View::PageBase*> route = {
        {L"hello", new HelloWorldPage()},
        {L"second", new SecondPage()},
        {L"third", new ThirdPage()}
    };
    // 注册页面
    app.registerPages(route);
    
    // 导航到主页
    app.navigateTo(L"hello");
    
    return 0;
}
