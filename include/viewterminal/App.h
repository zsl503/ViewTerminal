#pragma once
#define WIN32_LEAN_AND_MEAN             // 从 Windows 头文件中排除极少使用的内容
#include <windows.h>
#include <stack>
#include <regex>
#include <set>
#include <map>
#include "ComponentBase.h"

namespace View
{
	class PageBase;
	class App
	{
	public:
		App(int width, int height);
		void redirectTo(std::wstring);	// 关闭当前页面，跳转到指定页面
		void reLaunch(std::wstring);	// 关闭所有页面，跳转到指定页面
		void navigateTo(std::wstring);	// 直接跳转到指定页面
		void navigateBack();	// 返回上一个页面
		void registerPages(std::map<std::wstring, PageBase*>&);	// 注册页面
		int getWidth();
		int getHeight();
	private:
		void processDestroyed();	// 销毁页面后执行的操作，通常为打开下一个页面
		std::stack<PageBase*> pageStack;	// 页面栈
		std::map<std::wstring, PageBase*> route;	// 路由表
		HANDLE outputHandle;	// 输入句柄
		HANDLE inputHandle;		// 输出句柄
		std::wstring newPage;
		PageBase* openPage(std::wstring name);	// 打开新页面
		int width;
		int height;
		enum Type {
			RED, REL, NAT, NAB, None
		}type;	// 操作类型
	};

	class PageBase
	{
	public:
		PageBase() {}
		~PageBase();
		// 注册组件
		void registerComponents(std::set<ComponentBase*>&, std::set<ComponentBase*>&, std::set<ComponentBase*>&);
		void create();	// 生成并显示页面
		void update();	// 刷新页面
		void destroy();	// 销毁页面
		void clear();	// 清屏
		virtual void created() {}	// 绘制前执行
		virtual void mounted() {}	// 绘制后执行
		virtual void destroyed() {}	// 销毁后执行
		void setFocus(ComponentBase* com);	// 设置焦点组件
		void addComponent(ComponentBase*);	// 添加组件
		void removeComponent(ComponentBase*);	// 移除并销毁组件
		void detachComponent(ComponentBase*);	// 移除但不销毁组件
		// 初始化页面
		void initPage(App* app, std::wstring name, HANDLE outputHandle, HANDLE inputHandle);
		virtual void initComponents() = 0;	// 初始化组件
		HANDLE outputHandle;	// 输出句柄
		HANDLE inputHandle;		//输入句柄
	protected:
		App* app = NULL;	// App指针，管理全局信息
	private:
		bool isListen = true;	// 是否监听事件中
		std::set<ComponentBase*> allComponents;	// 全部注册组件，只有加入该数组的组件才会被绘制
		std::set<ComponentBase*> mouseComponents;	// 响应鼠标事件的组件
		std::set<ComponentBase*> keyComponents;	// 响应键盘事件的组件
		std::wstring name;	// 页面名称
		void eventListener();	// 开始监听
	};
}