# ViewTerminal

## 描述
这是一个控制台下的页面显示框架，可以参考示例：https://github.com/zsl503/CommodityManagement，其中LoginPage、MainPage等几个Page部分

## 使用方法
```C++
// #include <App.h>

// 编写单个页面
class LoginPage :
    public View::PageBase
{
public:
  LoginPage(){}
  void initComponents();
  // 你的自定义页面函数和参数
private:
  View::Text* boxTitle;
  // 你的自定义成员
}

// 初始化总框架,新版terminal中，这俩参数不起作用
App app(width,height);

// 登记全部页面
app.registerPages({
    {L"页面名称",new Page1()},{L"login",new LoginPage()},{L"main",new MainPage()},
});

// 跳转到Page1
app.navigateTo(L"Page1");
//		void redirectTo(std::wstring);	// 关闭当前页面，跳转到指定页面//
//		void reLaunch(std::wstring);	// 关闭所有页面，跳转到指定页面
//		void navigateTo(std::wstring);	// 直接跳转到指定页面
//		void navigateBack();	// 返回上一个页面


```
