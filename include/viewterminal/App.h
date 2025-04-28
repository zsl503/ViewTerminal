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
    // 表示控制台中单个字符单元的结构
    struct CharCell {
        wchar_t character;      // 字符
        WORD attributes;        // 颜色和其他属性
        
        // 比较操作符用于差异检测
        bool operator==(const CharCell& other) const {
            return character == other.character && attributes == other.attributes;
        }
        
        bool operator!=(const CharCell& other) const {
            return !(*this == other);
        }
    };
    
    // 表示组件整个区域的缓冲区
    using CharBuffer = std::vector<std::vector<CharCell>>;
    
    // 脏区域表示为矩形
    struct DirtyRect {
        COOD_INT left, top, right, bottom;
    };

    // 终端渲染引擎 - 负责统一缓冲区管理和差异渲染
    class Renderer {
    public:
        // 构造函数
        Renderer(COOD_INT width, COOD_INT height);
		// 不允许默认构造函数
		Renderer() = delete;
        
        // 初始化和清理
        void initialize();
        void clear();
        
        // 获取屏幕尺寸
        COOD_INT getWidth() const { return width; }
        COOD_INT getHeight() const { return height; }
        
        // 缓冲区操作
        void writeChar(const Point& position, wchar_t ch, Color foreColor, Color backColor);
        void writeString(const Point& position, const std::wstring& str, Color foreColor, Color backColor);
        void fillRect(const Point& topLeft, COOD_INT width, COOD_INT height, wchar_t ch, Color foreColor, Color backColor);
        void clearRect(const Point& topLeft, COOD_INT width, COOD_INT height, Color backColor = Color::BLACK);
        void drawBorder(const Point& topLeft, COOD_INT width, COOD_INT height, Color foreColor, Color backColor);
        
        // 渲染控制
        void markDirty(const Point& topLeft, COOD_INT width, COOD_INT height);
        void render(); // 执行实际的差异渲染
        
        // 辅助方法
        WORD getColorAttribute(Color foreColor, Color backColor) const;
        static COORD pointToCoord(const Point& point);
        static Point coordToPoint(const COORD& coord);

		CharBuffer& getCurrentBuffer() { return currentBuffer; } // 获取当前缓冲区
		CharBuffer& getNewBuffer() { return newBuffer; } // 获取新缓冲区
		std::vector<DirtyRect>& getDirtyRegions() { return dirtyRegions; } // 获取脏区域
		HANDLE getOutputHandle() { return outputHandle; } // 获取输出句柄
        
    private:
        HANDLE outputHandle;
        const COOD_INT width;
        const COOD_INT height;
        CharBuffer currentBuffer; // 当前显示的缓冲区
        CharBuffer newBuffer;     // 即将显示的缓冲区
        std::vector<DirtyRect> dirtyRegions;
        
        void initBuffers();
        void renderDifferences();
        bool isValidPosition(COOD_INT x, COOD_INT y) const;
    };

	class PageBase;
	class App
	{
	public:
		App(COOD_INT width, COOD_INT height);
		void redirectTo(std::wstring);	// 关闭当前页面，跳转到指定页面
		void reLaunch(std::wstring);	// 关闭所有页面，跳转到指定页面
		void navigateTo(std::wstring);	// 直接跳转到指定页面
		void navigateBack();	// 返回上一个页面
		void registerPages(std::map<std::wstring, std::unique_ptr<View::PageBase>>& route);	// 注册页面
		COOD_INT getWidth();
		COOD_INT getHeight();
		Renderer& getRenderer();	// 获取渲染引擎
	private:
		void processDestroyed();	// 销毁页面后执行的操作，通常为打开下一个页面
		std::stack<View::PageBase*> pageStack;	// 页面栈
		std::map<std::wstring, std::unique_ptr<View::PageBase>> route;	// 路由表

		HANDLE outputHandle;	// 输入句柄
		HANDLE inputHandle;		// 输出句柄
		Renderer render;	// 渲染引擎
		std::wstring newPage;
		std::unique_ptr<View::PageBase>& openPage(std::wstring name);	// 打开新页面
		COOD_INT width;
		COOD_INT height;
		enum Type {
			RED, REL, NAT, NAB, None
		}type;	// 操作类型
	};

	class PageBase
	{
	public:
		PageBase(Renderer& render):render(render) {}
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
		void removeComponent(ComponentBase*);	// 移除并销毁组件
		void detachComponent(ComponentBase*);	// 移除但不销毁组件
		// 初始化页面
		void initPage(App* app, std::wstring name, HANDLE inputHandle);
		virtual void initComponents() = 0;	// 初始化组件
		HANDLE outputHandle;	// 输出句柄
		HANDLE inputHandle;		//输入句柄
	
		// 重载addComponent以支持组合器
		PageBase& addComponent(ComponentBase*);

	protected:
		App* app = NULL;	// App指针，管理全局信息
		Renderer& render; // 渲染引擎指针
	private:
		bool isListen = true;	// 是否监听事件中
		std::set<ComponentBase*> allComponents;	// 全部注册组件，只有加入该数组的组件才会被绘制
		std::set<ComponentBase*> mouseComponents;	// 响应鼠标事件的组件
		std::set<ComponentBase*> keyComponents;	// 响应键盘事件的组件
		std::wstring name;	// 页面名称
		void eventListener();	// 开始监听
	};

}