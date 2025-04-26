#pragma once
#include <string>
#include <Windows.h>
#include <functional>
#include <vector>
#include <map>
#include <memory>
#include <any>

#ifndef __COOD_INT
#define __COOD_INT
  typedef short COOD_INT;
#endif

namespace View
{
	class ComponentBase;


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
		Relation getNewPointRelation(Point newPoint);
		Point getAbsPoint(COOD_INT reX, COOD_INT reY);
		Point getAbsPoint(Point rePoint);
		COORD getCOORD(COOD_INT reX = 0, COOD_INT reY = 0);
		COOD_INT getAbsX(COOD_INT reX = 0);
		COOD_INT getAbsY(COOD_INT reX = 0);
		COOD_INT x, y;
	};

    // 事件类型枚举
    enum class EventType
    {
        // 鼠标事件
        CLICK,            // 点击事件
		CLICK_CLICK,     // 双击事件
        HOVER,            // 悬停事件
        MOUSE_ENTER,      // 鼠标进入
        MOUSE_LEAVE,      // 鼠标离开
        
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
    
    private:
        MouseEvent event;
    };

    // 键盘事件数据
    class KeyEventArgs : public EventArgs
    {
    public:
        KeyEventArgs(ComponentBase* sender, const KeyEvent& event) 
            : EventArgs(sender), event(event) {}
        const KeyEvent& getEvent() const { return event; }
    
    private:
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
		void draw();	// 在控制台绘制图案
		virtual ~ComponentBase();	//	销毁
		virtual void update();	// 更新图案
		virtual ComponentBase* onMouseEvent(MouseEvent&);	// 鼠标事件
		virtual ComponentBase* onKeyEvent(KeyEvent&);	// 键盘事件
		virtual void setColorStyle(Color, Color);	// 设置颜色
		virtual void setFocus();	//	设置为焦点
		virtual void unsetFocus();	// 取消焦点
		bool isPointIn(Point);	// 判断坐标点是在组件内
		bool isPointIn(COORD);	// 判断坐标点是在组件内
		Point getPoint();	// 获取左上角坐标
		Point getRDPoint();	// 获取右下角坐标
		size_t getWidth();	//获取宽度
		size_t getHeight();	//获取高度
		virtual void setPoint(Point);	// 重置基准坐标
		virtual void setSize(size_t, size_t);	// 重置大小
		virtual void setShow(bool);	// 设置是否在页面中显示
		bool getShow();	// 获取页面显示设置
        
        // 事件系统方法
        void addEventListener(EventType type, const EventHandler& handler);
        void removeEventListeners(EventType type);
        
        void dispatchEvent(EventType type, EventArgs* args);
        
	protected:
		void setChange();	// 设置页面更新状态，调用函数后，组件会响应下一次更新
		virtual void beginDraw() = 0;	// 绘制前执行
		virtual void drawBackground();	// 绘制背景
		virtual void drawBorder();	// 绘制边框
		virtual void clearDraw();	//清除页面绘制
		ComponentBase(HANDLE outputHandle, Point point, size_t width = 0, size_t height = 0
			, Color defaultForeColor = Color::WHITE, Color defaultBackColor = Color::BLACK)
			: point(point), width(width), height(height), outputHandle(outputHandle),
			defaultForeColor(defaultForeColor), defaultBackColor(defaultBackColor),
			nowForeColor(defaultForeColor), nowBackColor(defaultBackColor),
			foreColor(defaultForeColor), backColor(defaultBackColor) {}
		size_t width, height;	// 宽度、高度
		Point point;	// 基准坐标点，为左上方坐标
		HANDLE outputHandle;	// 输出句柄，保存控制台句柄
		Color nowForeColor;	// 当前显示的文本色
		Color nowBackColor;	//当前显示的背景色
		Color backColor;	// 当前设置的背景色
		Color foreColor;	// 当前设置的文本色
		Color defaultForeColor;	// 默认文本色
		Color defaultBackColor;	// 默认背景色
		std::function<int(MouseEvent&, ComponentBase*)> mouseEventFun;	// 鼠标事件回调 (将被废弃)
		std::function<int(KeyEvent&, ComponentBase*)> keyEventFun;	// 键盘事件回调 (将被废弃)
		bool isChange = false;	// 是否响应更新
		bool focus;	// 是否为焦点
        EventManager eventManager;  // 事件管理器
	private:
		bool isDrawed = false;	// 当前组件是否已在屏幕上绘制
		bool isShow = true;	// 是否显示组件
	};

	class Text : public ComponentBase
	{
	public:
		Text(HANDLE outputHandle, std::wstring text, size_t abX, size_t abY, size_t width = 10, size_t height = 3
			, Color defaultForeColor = Color::WHITE, Color defaultBackColor = Color::BLACK)
			: ComponentBase(outputHandle, Point(abX, abY), width, height, defaultForeColor, defaultBackColor),
			text(text), endPoint(Point(abX, abY)) {}

		Text(HANDLE outputHandle, std::wstring text, Point point, size_t width = 10, size_t height = 3
			, Color defaultForeColor = Color::WHITE, Color defaultBackColor = Color::BLACK)
			: Text(outputHandle, text, point.x, point.y, width, height, defaultForeColor, defaultBackColor) {}

		Text(HANDLE outputHandle, int text, Point point, size_t width = 10, size_t height = 3
			, Color defaultForeColor = Color::WHITE, Color defaultBackColor = Color::BLACK)
			: Text(outputHandle, std::to_wstring(text), point.x, point.y, width, height, defaultForeColor, defaultBackColor) {}
		virtual std::wstring setText(std::wstring);
		std::wstring setText(int);
		std::wstring getText();
	protected:
		void beginDraw();
		Point endPoint;
	private:
		Point drawLine(std::wstring, Point, bool&);
		std::wstring text;
	};

	class Button : public ComponentBase
	{
	public:
		Button(HANDLE outputHandle, std::wstring text, size_t abX, size_t abY, size_t width = 10, size_t height = 3
			, Color defaultForeColor = Color::WHITE, Color defaultBackColor = Color::BLACK)
			: ComponentBase(outputHandle, Point(abX, abY), width, height, defaultForeColor, defaultBackColor),
			text(outputHandle, text, 0, 0) {
			hoverBackColor = defaultBackColor;
		}

		Button(HANDLE outputHandle, std::wstring text, Point point, size_t width = 10, size_t height = 3
			, Color defaultForeColor = Color::WHITE, Color defaultBackColor = Color::BLACK)
			: Button(outputHandle, text, point.x, point.y, width, height, defaultForeColor, defaultBackColor) {}
		void setColorStyle(Color, Color);
		void setHoverStyle(Color, Color);
		void setClickStyle(Color, Color);
		ComponentBase* onMouseEvent(MouseEvent&);
		std::wstring setText(std::wstring);
		std::wstring setText(int);
		std::wstring getText();
		
		// 老的事件回调将被保留一段时间以保持向后兼容性，但标记为过时
		std::function<int(MouseEvent&, ComponentBase*)> onClickFun;
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
		~List();
		List(HANDLE outputHandle, std::vector<std::wstring> items, size_t abX, size_t abY, size_t width = 10, size_t itemHeight = 3
			, Color defaultForeColor = Color::WHITE, Color defaultBackColor = Color::BLACK);
		List(HANDLE outputHandle, std::vector<std::wstring> items, Point point, size_t width = 10, size_t itemHeight = 3
			, Color defaultForeColor = Color::WHITE, Color defaultBackColor = Color::BLACK);
		void setPoint(Point); 
	protected:
		void beginDraw();
	private:
		std::vector<std::shared_ptr<View::Button>> buttons;
	};

	class Line : public ComponentBase
	{
	public:
		Line(HANDLE outputHandle, size_t abX, size_t abY, bool isHorizontal, size_t length
			, Color defaultForeColor = Color::WHITE, Color defaultBackColor = Color::BLACK)
			: ComponentBase(outputHandle, Point(abX, abY), isHorizontal ? length : 1, isHorizontal ? 1 : length, defaultForeColor, defaultBackColor),
			isHorizontal(isHorizontal), length(length) {}

		Line(HANDLE outputHandle, Point point, bool isHorizontal, size_t length = 10
			, Color defaultForeColor = Color::WHITE, Color defaultBackColor = Color::BLACK)
			: Line(outputHandle, point.x, point.y, isHorizontal, length, defaultForeColor, defaultBackColor) {}
	protected:
		void beginDraw();
	private:
		bool isHorizontal;
		size_t length;
	};

	class InputText :public Text
	{
	public:
		InputText(HANDLE outputHandle, std::wstring placeholder, size_t abX, size_t abY, size_t width = 10, size_t height = 3
			, Color defaultForeColor = Color::WHITE, Color defaultBackColor = Color::BLACK)
			: Text(outputHandle, placeholder, abX, abY, width, height, defaultForeColor, defaultBackColor), placeholder(placeholder), isPlaceholderVisible(true) {}

		InputText(HANDLE outputHandle, std::wstring placeholder, Point point, size_t width = 10, size_t height = 3
			, Color defaultForeColor = Color::WHITE, Color defaultBackColor = Color::BLACK)
			: InputText(outputHandle, placeholder, point.x, point.y, width, height, defaultForeColor, defaultBackColor) {}

		ComponentBase* onMouseEvent(MouseEvent&);
		std::function<int(InputText*)> textChanged;

	private:
		std::wstring placeholder;
		bool isPlaceholderVisible;
		std::function<int(MouseEvent&, ComponentBase*)> focusFun;
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
		LayoutBase(HANDLE outputHandle, size_t abX, size_t abY, size_t width, size_t height
			, Color defaultForeColor = Color::WHITE, Color defaultBackColor = Color::BLACK)
			: ComponentBase(outputHandle, Point(abX, abY), width, height, defaultForeColor, defaultBackColor) {}

		LayoutBase(HANDLE outputHandle, Point point, size_t width, size_t height
			, Color defaultForeColor = Color::WHITE, Color defaultBackColor = Color::BLACK)
			: LayoutBase(outputHandle, point.x, point.y, width, height, defaultForeColor, defaultBackColor) {}
		// virtual void registerComponents(std::vector<ComponentBase*>&, std::vector<ComponentBase*>&, std::vector<ComponentBase*>&);
		virtual bool addComponent(std::shared_ptr<ComponentBase>);
		virtual bool removeComponent(ComponentBase*);
		virtual void clear();
		void setShow(bool);
		~LayoutBase();
		void update();
		ComponentBase* onMouseEvent(MouseEvent&);
		ComponentBase* onKeyEvent(KeyEvent&);
		void setPoint(Point);

	protected:
		void beginDraw();
		std::vector<std::shared_ptr<ComponentBase>> allComponents;
		// std::vector<ComponentBase*> mouseComponents;
		// std::vector<ComponentBase*> keyComponents;
	};

	class Span :public LayoutBase
	{
	public:
		Span(HANDLE outputHandle, size_t abX, size_t abY, size_t width, size_t height, size_t space = 0
			, Color defaultForeColor = Color::WHITE, Color defaultBackColor = Color::BLACK)
			: LayoutBase(outputHandle, Point(abX, abY), width, height, defaultForeColor, defaultBackColor),
			space(space) {}
		Span(HANDLE outputHandle, Point point, size_t width, size_t height, size_t space = 0
			, Color defaultForeColor = Color::WHITE, Color defaultBackColor = Color::BLACK)
			: Span(outputHandle, point.x, point.y, width, height, space, defaultForeColor, defaultBackColor) {}
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
		Div(HANDLE outputHandle, size_t abX, size_t abY, size_t width, size_t height, size_t space = 0
			, Color defaultForeColor = Color::WHITE, Color defaultBackColor = Color::BLACK)
			: LayoutBase(outputHandle, Point(abX, abY), width, height, defaultForeColor, defaultBackColor),
			space(space) {}

		Div(HANDLE outputHandle, Point point, size_t width, size_t height, size_t space = 0
			, Color defaultForeColor = Color::WHITE, Color defaultBackColor = Color::BLACK)
			: Div(outputHandle, point.x, point.y, width, height, space, defaultForeColor, defaultBackColor) {}
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
