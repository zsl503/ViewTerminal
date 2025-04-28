#pragma once
#include <string>
#include <Windows.h>
#include <functional>
#include <vector>
#include <map>
#include <memory>
#include <any>
#include <algorithm>

#ifndef __COOD_INT
#define __COOD_INT
//   typedef int COOD_INT;
  typedef int COOD_INT;
//   定义最大值和最小值
  #define COOD_INT_MAX INT_MAX
  #define COOD_INT_MIN INT_MIN
#endif
int GetCharacterConsoleWidth(wchar_t ch);
namespace View
{
	class ComponentBase;
	class Renderer;
	class Point
	{
	public:
		enum class Relation
		{
			SAME, UP, DOWN, LEFT, RIGHT, LEFTUP, LEFTDOWN, RIGHTUP, RIGHTDOWN
		};

		Point(COOD_INT x = 0, COOD_INT y = 0) :x(x), y(y) {}
		Point(const COORD& point) :x(point.X), y(point.Y) {}
		Point(const Point& point) :x(point.x), y(point.y) {}
		Relation getNewPointRelation(const Point& rePoint) const;
		Point getAbsPoint(COOD_INT reX, COOD_INT reY) const;
		Point getAbsPoint(const Point& rePoint) const;
		COORD getCOORD(COOD_INT reX = 0, COOD_INT reY = 0);
		COOD_INT getAbsX(COOD_INT reX = 0);
		COOD_INT getAbsY(COOD_INT reX = 0);
		COOD_INT x, y;
		// 相等比较
		bool operator==(const Point& other) const { return x == other.x && y == other.y; }
		bool operator!=(const Point& other) const {
			return !(*this == other);
		}
	};

    // 事件类型枚举
    enum class EventType
    {
        // 鼠标事件
        CLICK,            // 点击事件
		CLICK_CLICK,     // 双击事件
        MOUSE_ENTER,      // 鼠标进入
        MOUSE_LEAVE,      // 鼠标离开
		MOUSE_OVER,       // 鼠标悬停
        
        // 键盘事件
        KEY_PRESS,        // 按键按下
        KEY_RELEASE,      // 按键释放
        
        // 组件事件
        FOCUS,            // 获得焦点
        BLUR,             // 失去焦点
        VALUE_CHANGE,     // 值改变
        
        // 自定义事件类型
        CUSTOM            // 自定义事件
    };


	enum class MouseEventType
	{
		RCLICK,
		RDOUBLECLICK,
		LCLICK,
		LDOUBLECLICK,
		HOVER
	};

	typedef struct MouseEvent
	{
		Point point;
		MouseEventType type;
	}MouseEvent;

	typedef struct KeyEvent
	{
		Point point;
		MouseEventType type;
	}KeyEvent;


    // 事件数据基类
    class EventArgs
    {
    public:
        EventArgs(ComponentBase* sender) : sender(sender) {}
        virtual ~EventArgs() = default;
        ComponentBase* getSender() { return sender; }
    
    private:
        ComponentBase* sender;
    };

    // 鼠标事件数据
    class MouseEventArgs : public EventArgs
    {
    public:
        MouseEventArgs(ComponentBase* sender, const MouseEvent& event) 
            : EventArgs(sender), event(event) {}
        const MouseEvent& getEvent() const { return event; }
    
        MouseEvent event;
    };

    // 键盘事件数据
    class KeyEventArgs : public EventArgs
    {
    public:
        KeyEventArgs(ComponentBase* sender, const KeyEvent& event) 
            : EventArgs(sender), event(event) {}
        const KeyEvent& getEvent() const { return event; }
    
        KeyEvent event;
    };

    // 值改变事件数据
    class ValueChangedEventArgs : public EventArgs
    {
    public:
        ValueChangedEventArgs(ComponentBase* sender, const std::any& oldValue, const std::any& newValue) 
            : EventArgs(sender), oldValue(oldValue), newValue(newValue) {}
        const std::any& getOldValue() const { return oldValue; }
        const std::any& getNewValue() const { return newValue; }
    
    private:
        std::any oldValue;
        std::any newValue;
    };

    // 事件委托类型定义
    using EventHandler = std::function<void(EventArgs*)>;

    // 事件管理器类
    class EventManager
    {
    public:
        // 添加事件监听器
        void addEventListener(EventType type, const EventHandler& handler)
        {
            listeners[type].emplace_back(handler);
        }
        
        // 移除事件监听器（根据类型清除所有）
        void removeEventListeners(EventType type)
        {
            listeners[type].clear();
        }
        
        // 触发事件
        void dispatchEvent(EventType type, EventArgs* args)
        {
            for (const auto& handler : listeners[type])
            {
                handler(args);
            }
        }
        
    private:
        std::map<EventType, std::vector<EventHandler>> listeners;
    };

	enum class Color
	{
		BLACK, BLUE, GREEN, LIGHTGREEN, RED, PURPLE, YELLOW, WHITE,
		GREY, LIGHTBLUE, PALEGREEN, PALELIGHTGREEN, LIGHTRED,
		LIGHTPURPLE, LIGHTYELLOW, BRIGHTWHITE,
	};

	class ComponentBase
	{
	public:
		virtual ~ComponentBase();	//	销毁
		virtual void update(bool force=false);	// 更新图案
		virtual void onMouseEvent(MouseEvent&);	// 鼠标事件
		virtual void onKeyEvent(KeyEvent&);	// 键盘事件
		virtual void setColorStyle(Color, Color);	// 设置颜色
		virtual void setFocus();	//	设置为焦点
		virtual void unsetFocus();	// 取消焦点
		bool isPointIn(const Point&);	// 判断坐标点是在组件内
		bool isPointIn(const COORD&);	// 判断坐标点是在组件内
		Point getPoint();	// 获取左上角坐标
		Point getRDPoint();	// 获取右下角坐标
		size_t getWidth();	//获取宽度
		size_t getHeight();	//获取高度
		virtual void setPoint(const Point&);	// 重置基准坐标
		virtual void setSize(size_t, size_t);	// 重置大小
		virtual void setShow(bool);	// 设置是否在页面中显示
		bool getShow();	// 获取页面显示设置
        
        // 事件系统方法
        void addEventListener(EventType type, const EventHandler& handler);
        void removeEventListeners(EventType type);
        void dispatchEvent(EventType type, EventArgs* args);
		bool needChange() { return isChange; }	// 获取组件是否需要更新
        
	protected:
		void setChange();	// 设置页面更新状态，调用函数后，组件会响应下一次更新
		virtual void beginDraw() = 0;	// 绘制前执行
		virtual void drawBackground();	// 绘制背景
		virtual void drawBorder();	// 绘制边框
		virtual void clearDraw(bool useBackColor=false);	//清除页面绘制
		ComponentBase(Renderer& render, Point point, size_t width = 0, size_t height = 0
			, Color defaultForeColor = Color::WHITE, Color defaultBackColor = Color::BLACK)
			: point(point), width(width), height(height),
			defaultForeColor(defaultForeColor), defaultBackColor(defaultBackColor),
			nowForeColor(defaultForeColor), nowBackColor(defaultBackColor),
			foreColor(defaultForeColor), backColor(defaultBackColor), render(render) {}
        
		size_t width, height;	// 宽度、高度
		Point point;	// 基准坐标点，为左上方坐标
		Renderer& render;	// 输出句柄，保存控制台句柄
		Color nowForeColor;	// 当前显示的文本色
		Color nowBackColor;	//当前显示的背景色
		Color backColor;	// 当前设置的背景色
		Color foreColor;	// 当前设置的文本色
		Color defaultForeColor;	// 默认文本色
		Color defaultBackColor;	// 默认背景色
		bool isChange = true;	// 是否响应更新
		bool focus;	// 是否为焦点

		bool isHover = false;
		bool isOnclick = false;
        EventManager eventManager;  // 事件管理器
        
	private:
		bool isDrawed = false;	// 当前组件是否已在屏幕上绘制
		bool isShow = true;	// 是否显示组件
	};

	class Text : public ComponentBase
	{
	public:
		Text(Renderer& render, std::wstring text, size_t abX, size_t abY, size_t width = 10, size_t height = 3
			, Color defaultForeColor = Color::WHITE, Color defaultBackColor = Color::BLACK)
			: ComponentBase(render, Point(abX, abY), width, height, defaultForeColor, defaultBackColor),
			text(text), endPoint(Point(abX, abY)) {}

		Text(Renderer& render, std::wstring text, Point point, size_t width = 10, size_t height = 3
			, Color defaultForeColor = Color::WHITE, Color defaultBackColor = Color::BLACK)
			: Text(render, text, point.x, point.y, width, height, defaultForeColor, defaultBackColor) {}

		Text(Renderer& render, int text, Point point, size_t width = 10, size_t height = 3
			, Color defaultForeColor = Color::WHITE, Color defaultBackColor = Color::BLACK)
			: Text(render, std::to_wstring(text), point.x, point.y, width, height, defaultForeColor, defaultBackColor) {}
		virtual std::wstring setText(std::wstring);
		std::wstring setText(int);
		std::wstring getText();
	protected:
		void beginDraw();
		Point endPoint;
	private:
		Point drawLineToBuffer(std::wstring text, Point relPoint, bool& isFinish); // 修改为写入缓冲区而非直接绘制
		std::wstring text;
	};

	class Button : public ComponentBase
	{
	public:
		Button(Renderer& render, std::wstring text, size_t abX, size_t abY, size_t width = 10, size_t height = 3
			, Color defaultForeColor = Color::WHITE, Color defaultBackColor = Color::BLACK)
			: ComponentBase(render, Point(abX, abY), width, height, defaultForeColor, defaultBackColor),
			text(render, text, 0, 0) {
			hoverBackColor = defaultBackColor;
			this->addDefaultEventListener();
		}

		Button(Renderer& render, std::wstring text, Point point, size_t width = 10, size_t height = 3
			, Color defaultForeColor = Color::WHITE, Color defaultBackColor = Color::BLACK)
			: Button(render, text, point.x, point.y, width, height, defaultForeColor, defaultBackColor) {}
		void setColorStyle(Color, Color);
		void setHoverStyle(Color, Color);
		void setClickStyle(Color, Color);
		void addDefaultEventListener();
		std::wstring setText(std::wstring);
		std::wstring setText(int);
		std::wstring getText();
	protected:
		void beginDraw();
		Text text;
		size_t textAbX = 1;
		size_t textAbY = 1;
		size_t textSubHeight = 2;
		size_t textSubWidth = 2;
	private:
		Color hoverBackColor;
		Color hoverForeColor = Color::LIGHTYELLOW;
		Color clickBackColor = Color::WHITE;
		Color clickForeColor = Color::LIGHTRED;
		bool isHover = false;
		bool isOnclick = false;
	};

	class List : public ComponentBase
	{
	public:
		List(Renderer& render, const std::vector<std::wstring>& items, size_t abX, size_t abY, size_t width = 10, size_t itemHeight = 3
			, Color defaultForeColor = Color::WHITE, Color defaultBackColor = Color::BLACK);
		List(Renderer& render, const std::vector<std::wstring>& items, const Point& point, size_t width = 10, size_t itemHeight = 3
			, Color defaultForeColor = Color::WHITE, Color defaultBackColor = Color::BLACK);
		void setPoint(const Point&); 
	protected:
		void beginDraw();
	private:
		std::vector<std::shared_ptr<View::Button>> buttons;
	};

	class Line : public ComponentBase
	{
	public:
		Line(Renderer& render, size_t abX, size_t abY, bool isHorizontal, size_t length
			, Color defaultForeColor = Color::WHITE, Color defaultBackColor = Color::BLACK)
			: ComponentBase(render, Point(abX, abY), isHorizontal ? length : 1, isHorizontal ? 1 : length, defaultForeColor, defaultBackColor),
			isHorizontal(isHorizontal), length(length) {}

		Line(Renderer& render, Point point, bool isHorizontal, size_t length = 10
			, Color defaultForeColor = Color::WHITE, Color defaultBackColor = Color::BLACK)
			: Line(render, point.x, point.y, isHorizontal, length, defaultForeColor, defaultBackColor) {}
	protected:
		void beginDraw();
	private:
		bool isHorizontal;
		size_t length;
	};

	class InputText :public Text
	{
	public:
		InputText(Renderer& render, std::wstring placeholder, size_t abX, size_t abY, size_t width = 10, size_t height = 3
			, Color defaultForeColor = Color::WHITE, Color defaultBackColor = Color::BLACK)
			: Text(render, placeholder, abX, abY, width, height, defaultForeColor, defaultBackColor), placeholder(placeholder), isPlaceholderVisible(true) {}

		InputText(Renderer& render, std::wstring placeholder, Point point, size_t width = 10, size_t height = 3
			, Color defaultForeColor = Color::WHITE, Color defaultBackColor = Color::BLACK)
			: InputText(render, placeholder, point.x, point.y, width, height, defaultForeColor, defaultBackColor) {}

		std::function<int(InputText*)> textChanged;
		void onMouseEvent(MouseEvent&);	// 鼠标事件

	private:
		std::wstring placeholder;
		bool isPlaceholderVisible;
        std::vector<std::wstring> calculateTextLines(const std::wstring &text);
        void renderText(const std::vector<std::wstring> &textLines);
        int calculateCursorPosition(const std::vector<std::wstring> &textLines, int clickX, int clickY, int &currentLineIdx, int &accumulatedWidth);
        void showAndPositionCursor(int accumulatedWidth, int currentLineIdx);
        void handleDelete(std::wstring &newText, int &cursorPos, std::vector<std::wstring> &textLines, int &currentLineIdx, int &linePos, bool isBackspace);
        void updateLineAndPosition(const std::vector<std::wstring> &textLines, int cursorPos, int &currentLineIdx, int &linePos);
        int calculateCursorDisplayPosition(const std::vector<std::wstring> &textLines, int currentLineIdx, int linePos);
        bool handleKeyEvent(KEY_EVENT_RECORD ker, std::wstring &newText, std::wstring &currentText, int &cursorPos, std::vector<std::wstring> &textLines, int &currentLineIdx, int &linePos, bool &editing);
        bool handleCharacterInput(KEY_EVENT_RECORD ker, std::wstring &newText, int &cursorPos, std::vector<std::wstring> &textLines, int &currentLineIdx, int &linePos);
                                  
    };

    class LayoutBase : public ComponentBase
	{
	public:
		LayoutBase(Renderer& render, size_t abX, size_t abY, size_t width, size_t height
			, Color defaultForeColor = Color::WHITE, Color defaultBackColor = Color::BLACK)
			: ComponentBase(render, Point(abX, abY), width, height, defaultForeColor, defaultBackColor) {}

		LayoutBase(Renderer& render, Point point, size_t width, size_t height
			, Color defaultForeColor = Color::WHITE, Color defaultBackColor = Color::BLACK)
			: LayoutBase(render, point.x, point.y, width, height, defaultForeColor, defaultBackColor) {}
		// virtual void registerComponents(std::vector<ComponentBase*>&, std::vector<ComponentBase*>&, std::vector<ComponentBase*>&);
		virtual bool addComponent(std::shared_ptr<ComponentBase>);
		virtual bool removeComponent(ComponentBase*);
		virtual void clear();
		void setShow(bool);
		void update(bool force=false);
		void onMouseEvent(MouseEvent&);
		void onKeyEvent(KeyEvent&);
		void setPoint(const Point&);

	protected:
		void beginDraw();
		std::vector<std::shared_ptr<ComponentBase>> allComponents;
		// std::vector<ComponentBase*> mouseComponents;
		// std::vector<ComponentBase*> keyComponents;
	};

	class Span :public LayoutBase
	{
	public:
		Span(Renderer& render, size_t abX, size_t abY, size_t width, size_t height, size_t space = 0
			, Color defaultForeColor = Color::WHITE, Color defaultBackColor = Color::BLACK)
			: LayoutBase(render, Point(abX, abY), width, height, defaultForeColor, defaultBackColor),
			space(space) {}
		Span(Renderer& render, Point point, size_t width, size_t height, size_t space = 0
			, Color defaultForeColor = Color::WHITE, Color defaultBackColor = Color::BLACK)
			: Span(render, point.x, point.y, width, height, space, defaultForeColor, defaultBackColor) {}
		void setPadding(unsigned short top = 0, unsigned short right = 0, unsigned short bottom = 0, unsigned short left = 0);
		// void registerComponents(std::vector<ComponentBase*>&, std::vector<ComponentBase*>&, std::vector<ComponentBase*>&);
		bool addComponent(std::shared_ptr<ComponentBase>);
		bool removeComponent(ComponentBase*);
		void setSpace(size_t space);
		void clear();
	protected:
		unsigned short padding[4] = { 0,0,0,0 };
		size_t currentWidth = 0;
		size_t space;
	};

	class Div : public LayoutBase
	{
	public:
		Div(Renderer& render, size_t abX, size_t abY, size_t width, size_t height, size_t space = 0
			, Color defaultForeColor = Color::WHITE, Color defaultBackColor = Color::BLACK)
			: LayoutBase(render, Point(abX, abY), width, height, defaultForeColor, defaultBackColor),
			space(space) {}

		Div(Renderer& render, Point point, size_t width, size_t height, size_t space = 0
			, Color defaultForeColor = Color::WHITE, Color defaultBackColor = Color::BLACK)
			: Div(render, point.x, point.y, width, height, space, defaultForeColor, defaultBackColor) {}
		// void registerComponents(std::vector<ComponentBase*>&, std::vector<ComponentBase*>&, std::vector<ComponentBase*>&);
		bool addComponent(std::shared_ptr<ComponentBase>);
		bool removeComponent(ComponentBase*);
		void setSpace(size_t space);
		void setPadding(unsigned short top = 0, unsigned short right = 0, unsigned short bottom = 0, unsigned short left = 0);
		void clear();

	protected:
		unsigned short padding[4] = { 0,0,0,0 };
		size_t currentHeight = 0;
		size_t space;
	};

	std::vector<std::wstring> split(const std::wstring& in, const std::wstring& delim);
	std::string UnicodeToANSI(const std::wstring& wstr);
}
